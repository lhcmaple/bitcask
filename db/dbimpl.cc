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
    Handle handle;
    GUARD {
        Lock lock(mutex_);
        if(lb_->append(key, value, &handle) != 0) {
            error = true;
            return -1;
        }
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
    const Node *cur = ht_->find(key);
    if(cur == nullptr) {
        return -1;
    }
    //
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
    //
    return 0;
}

int DBImpl::compact(bool background = true) {
    if(error) {
        return -1;
    }
    if(iscompacting_.load() == true) {
        return 0;
    }
    iscompacting_.store(true);
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