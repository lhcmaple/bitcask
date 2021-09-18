#include "dbimpl.h"
#include "iter.h"
#include "env.h"

class DBImpl::Iterator : public Iter {
private:
    DBImpl *impl_;
public:
    Iterator(DBImpl *impl) : impl_(impl) {
        //
    }
    bool isValid() override {
        //
    }
    void seekToFirst() override {
        //
    }
    void next() override {
        //
    }
    void *get() override {
        //
    }
    ~Iterator() override {}
};

int DBImpl::put(const string_view &key, const string_view &value) {
    if(error) {
        return -1;
    }
    uint64_t sequence;
    {
        Lock lock(mutex_);
        sequence = lb_->newSequence();
        lb_->append(key, value, sequence);
        Handle handle;
        handle.file_id = lb_->;
        //
        ht_->insert(key, handle);
    }
    return 0;
}

int DBImpl::get(const string_view &key, string *value) {
    if(error) {
        return -1;
    }
    if(value != nullptr) {
        value->clear();
    }
    const Node * cur = ht_->find(key);
    if(cur == nullptr) {
        return -1;
    }
    //
}

int DBImpl::del(const string_view &key) {
    if(error) {
        return -1;
    }
    lb_->append(key, "");
    ht_->erase(key);
    //
    return 0;
}

struct Info {
    DBImpl *impl;
    bool background;
};

void *DBImpl::compactThread(void *arg) {
    Info *pinfo = static_cast<Info *>(arg);
    DBImpl *impl = pinfo->impl;
    bool background = pinfo->background;
    //
    return 0;
}

int DBImpl::compact(bool background = true) {
    if(error) {
        return -1;
    }
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