#ifndef LOGREADER_H
#define LOGREADER_H

#include "iter.h"
#include "env.h"
#include "hashtable.h"
#include "cache.h"

#include <cstdint>
#include <string>

using std::string;

struct LogContent {
    uint32_t crc;
    uint64_t sequence;
    string key;
    string value;
};

struct HIndexContent {
    uint64_t sequence;
    uint32_t offset;
    uint32_t size;
    string key;
};

class LogReader {
private:
    string db_name_;
    uint64_t file_id_;
    fdNode *fdnode_;
    string data_;

    class Iterator;
public:
    static LogReader *newLogReader(uint64_t file_id, const string_view &db_name);
    LogReader(uint64_t file_id, const string_view &db_name) : file_id_(file_id), db_name_(db_name) {
        fdnode_ = LRUCache::globalLRUCache()->get(file_id, db_name_ + "/" + std::to_string(file_id_) + ".log");
    }
    LogContent *seek(const Handle &handle);
    ~LogReader() {
        LRUCache::globalLRUCache()->release(fdnode_);
    }
    Iter *newIter();
};

class HIndexReader {
private:
    string db_name_;
    uint64_t file_id_;
    RandomReadFile *rf_;
    string data;

    void examine();

    class Iterator;
public:
    static HIndexReader *newHIndexReader(uint64_t file_id, const string_view &db_name);
    HIndexReader(uint64_t file_id, const string_view &db_name) : file_id_(file_id), db_name_(db_name) {
        rf_ = Env::globalEnv()->newRandomReadFile(db_name_ + "/" + std::to_string(file_id_) + ".hindex");
        if(rf_ != nullptr) { //校验索引完整性
            examine();
        }
    }
    Iter *newIter();
    ~HIndexReader() {
        delete rf_;
    }
};

#endif