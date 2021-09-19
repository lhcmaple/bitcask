#include "dbimpl.h"
#include "iter.h"
#include "env.h"
#include "logreader.h"

#include <utility>

using std::pair;

class DBImpl::Iterator : public Iter {
private:
    Iter *it_;
    string data_;
    pair<string, string> kv;
public:
    Iterator(DBImpl *impl) : it_(impl->ht_->newIter()) {
        
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
        LogReader *lr = LogReader::newLogReader(cur->handle.file_id);
        assert(lr != nullptr);
        LogContent *lc = lr->seek(cur->handle);
        assert(lc != nullptr);
        kv.first.swap(lc->key);
        kv.second.swap(lc->value);
        delete lc;
        return &kv;
    }
    ~Iterator() override {

    }
};

int DBImpl::put(const string_view &key, const string_view &value) {
    if(error) {
        return -1;
    }
    Handle handle;
    GUARD_BEGIN(mutex_)
        Lock lock(mutex_);
        if(lb_->append(key, value, &handle) != 0) {
            error = true;
            return -1;
        }
        ht_->insert(key, handle);
    GUARD_END
    return 0;
}

int DBImpl::get(const string_view &key, string *value) {
    if(error) {
        return -1;
    }
    if(value != nullptr) {
        value->clear();
    }
    const Node *cur = ht_->find(key);
    if(cur == nullptr) {
        return -1;
    }
    LogReader *lr = LogReader::newLogReader(cur->handle.file_id);
    assert(lr == nullptr);
    LogContent *lc = lr->seek(cur->handle);
    if(lc == nullptr) {
        error = true;
        return -1;
    }
    value->swap(lc->value);
    delete lc;
    return 0;
}

int DBImpl::del(const string_view &key) {
    return put(key, "");
}

struct Info {
    DBImpl *impl;
    bool background;
};

void *DBImpl::compactThread(void *arg) {
    Info *pinfo = static_cast<Info *>(arg);
    DBImpl *impl = pinfo->impl;
    bool background = pinfo->background;
    if(background) {
        Lock lock(impl->mutex_);
        impl->lb_->compaction();
    } else {
        impl->lb_->compaction();
    }
    fileNode *fn = nullptr;
    while(fn = impl->lb_->compactFile()) {
        LogReader *lr = LogReader::newLogReader(fn->file_id);
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
            impl->put(lc->key, lc->value);
            GUARD_END
        }
        delete lr;
        Env::globalEnv()->rmFile(std::to_string(fn->file_id) + ".log");
        Env::globalEnv()->rmFile(std::to_string(fn->file_id) + ".hindex");
        delete fn;
    }
    impl->iscompacting_ = false;
    return 0;
}

int DBImpl::compact(bool background = true) {
    if(error) {
        return -1;
    }
    GUARD_BEGIN(mutex_)
        Lock lock(mutex_);
        if(iscompacting_ == true) {
            return -1;
        }
        iscompacting_ = true;
    GUARD_END
    Info info;
    info.impl = this;
    info.background = background;
    if(background) {
        return Env::globalEnv()->newThread(DBImpl::compactThread, &info);
    } else {
        return reinterpret_cast<int>(DBImpl::compactThread(&info));
    }
}

Iter *DBImpl::newIter() {
    if(error) {
        return nullptr;
    }
    return new Iterator(this);
}