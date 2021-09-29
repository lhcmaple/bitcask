#include "dbimpl.h"
#include "iter.h"
#include "env.h"
#include "logreader.h"

#include <utility>

using std::pair;

class DBImpl::Iterator : public Iter {
private:
    DBImpl *impl_;
    Iter *it_;
    string data_;
    pair<string, string> kv;
public:
    Iterator(DBImpl *impl) : impl_(impl), it_(impl->ht_->newIter()) {
        
    }
    bool isValid() override {
        return it_->isValid();
    }
    void seekToFirst() override {
        it_->seekToFirst();
    }
    void next() override {
        it_->next();
    }
    void *get() override {
        Node *cur = static_cast<Node *>(it_->get());
        LogReader *lr = LogReader::newLogReader(cur->handle.file_id, impl_->db_name_);
        assert(lr != nullptr);
        LogContent *lc = lr->seek(cur->handle);
        assert(lc != nullptr);
        kv.first.swap(lc->key);
        kv.second.swap(lc->value);
        delete lc;
        delete lr;
        return &kv;
    }
    ~Iterator() override {
        delete it_;
    }
};

int DBImpl::put(const string_view &key, const string_view &value) {
    if(error) {
        return -1;
    }
    Handle handle;
    GUARD_BEGIN(mutex_)
        if(lb_->append(key, value, &handle) != 0) {
            error = true;
            return -1;
        }
        ht_->insert(key, handle);
    GUARD_END
    return 0;
}

int DBImpl::putUnlock(const string_view &key, const string_view &value) {
    if(error) {
        return -1;
    }
    Handle handle;
    if(lb_->append(key, value, &handle) != 0) {
        error = true;
        return -1;
    }
    ht_->insert(key, handle);
    return 0;
}

int DBImpl::get(const string_view &key, string *value) {
    if(error) {
        return -1;
    }
    if(value != nullptr) {
        value->clear();
    }
    GUARD_BEGIN(mutex_)
        const Node *cur = ht_->find(key);
        if(cur == nullptr) {
            return -1;
        }
        LogReader *lr = LogReader::newLogReader(cur->handle.file_id, db_name_);
        assert(lr != nullptr);
        LogContent *lc = lr->seek(cur->handle);
        if(lc == nullptr) {
            error = true;
            return -1;
        }
        value->swap(lc->value);
        delete lr;
    delete lc;
    GUARD_END
    return 0;
}

int DBImpl::del(const string_view &key) {
    if(error) {
        return -1;
    }
    Handle handle;
    GUARD_BEGIN(mutex_)
        if(ht_->find(key) != nullptr) {
            if(lb_->append(key, "", &handle) != 0) {
                error = true;
                return -1;
            }
            ht_->erase(key);
        }
    GUARD_END
    return 0;
}

struct Info {
    DBImpl *impl;
    COMPACTION type;
};

void *DBImpl::compactThread(void *arg) {
    Info *pinfo = static_cast<Info *>(arg);
    DBImpl *impl = pinfo->impl;
    COMPACTION type = pinfo->type;
    if(type == AUTO) {
        autoCompaction(); //
        return nullptr;
    }
    delete pinfo;
    GUARD_BEGIN(impl->mutex_)
        impl->lb_->compaction();
    GUARD_END
    fileNode *fn = nullptr;
    while(fn = impl->lb_->compactFile()) {
        LogReader *lr = LogReader::newLogReader(fn->file_id, impl->db_name_);
        assert(lr != nullptr);
        Iter *it = lr->newIter();
        it->seekToFirst();
        for(; it->isValid(); it->next()) {
            LogContent *lc = static_cast<LogContent *>(it->get());
            GUARD_BEGIN(impl->mutex_)
            const Node *node = impl->ht_->find(lc->key);
            if(node == nullptr || node->handle.sequence > lc->sequence) {
                continue;
            }
            impl->putUnlock(lc->key, lc->value);
            GUARD_END
        }
        delete it;
        delete lr;
        Env::globalEnv()->rmFile(impl->db_name_ + "/" + std::to_string(fn->file_id) + ".log");
        Env::globalEnv()->rmFile(impl->db_name_ + "/" + std::to_string(fn->file_id) + ".hindex");
        delete fn;
    }
    impl->iscompacting_ = false;
    return nullptr;
}

int DBImpl::compact(COMPACTION type) {
    if(error) {
        return -1;
    }
    GUARD_BEGIN(mutex_)
        if(iscompacting_ == true) {
            return -1;
        }
        iscompacting_ = true;
    GUARD_END
    Info *info = new Info;
    info->impl = this;
    info->type = type;
    if(type != FOREGROUND) {
        return Env::globalEnv()->newThread(DBImpl::compactThread, info, &pid);
    } else {
        return reinterpret_cast<long long>(DBImpl::compactThread(info));
    }
}

Iter *DBImpl::newIter() {
    if(error) {
        return nullptr;
    }
    return new Iterator(this);
}