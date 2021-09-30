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
    string db_name_;
    bool error;
    bool iscompacting_;
    uint64_t pid;

    static void *compactThread(void *arg);
    static void autoCompaction();
    int putUnlock(const string_view &key, const string_view &value);
public:
    class Iterator;

    DBImpl(const string_view &db_name) : error(false), db_name_(db_name) {
        iscompacting_ = false;
        mutex_ = Env::globalEnv()->newMutex();
        ht_ = new HashTable();
        lb_ = LogBuilder::newLogBuilder(db_name_, ht_);
    }
    ~DBImpl() {
        if(iscompacting_) {
            iscompacting_ = false;
            Env::globalEnv()->joinThread(pid); // auto compaction 时会死锁, 应该加个判断之类的, iscompacting_改成枚举会好点, 并且在autocompact时监测该变量的状态
        }
        delete ht_;
        delete lb_;
        delete mutex_;
    }
    int put(const string_view &key, const string_view &value) override;
    int get(const string_view &key, string *value) override;
    int del(const string_view &key) override;
    int compact(COMPACTION type) override;
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
