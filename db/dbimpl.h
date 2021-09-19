#ifndef DBIMPL_H
#define DBIMPL_H

#include "db.h"
#include "logbuilder.h"
#include "hashtable.h"

class DBImpl : public DB {
private:
    LogBuilder *lb_;
    HashTable *ht_;
    Mutex *mutex_;
    bool error;
    bool iscompacting_;

    static void *compactThread(void *arg);
public:
    class Iterator;

    DBImpl(const string_view &db_name) : error(false) {
        iscompacting_ = false;
        mutex_ = Env::globalEnv()->newMutex();
        ht_ = new HashTable();
        lb_ = LogBuilder::newLogBuilder(db_name, ht_);
    }
    ~DBImpl() {
        delete ht_;
        delete lb_;
        delete mutex_;
    }
    int put(const string_view &key, const string_view &value) override;
    int get(const string_view &key, string *value) override;
    int del(const string_view &key) override;
    int compact(bool background = true) override;
    Iter *newIter() override;
    bool isValid() override {
        return !error;
    }
    friend class DB;
};

DB *DB::open(const string_view &db_name) {
    DBImpl *impl = new DBImpl(db_name);
    if(impl->lb_ == nullptr) {
        delete impl;
        impl = nullptr;
    }
    return impl;
}

#endif