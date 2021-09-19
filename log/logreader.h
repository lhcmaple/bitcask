#ifndef LOGREADER_H
#define LOGREADER_H

#include "iter.h"
#include "env.h"
#include "hashtable.h"

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
    uint64_t file_id_;
    RandomReadFile *rf_;
    string data_;

    class Iterator;
public:
    static LogReader *newLogReader(uint64_t file_id);
    LogReader(uint64_t file_id) : file_id_(file_id) {
        rf_ = Env::globalEnv()->newRandomReadFile(std::to_string(file_id_) + ".log");
    }
    LogContent *seek(const Handle &handle);
    ~LogReader() {
        delete rf_;
    }
    Iter *newIter();
};

class HIndexReader {
private:
    uint64_t file_id_;
    RandomReadFile *rf_;
    string data;

    void examine();

    class Iterator;
public:
    static HIndexReader *newHIndexReader(uint64_t file_id);
    HIndexReader(uint64_t file_id) : file_id_(file_id) {
        rf_ = Env::globalEnv()->newRandomReadFile(std::to_string(file_id_) + ".hindex");
        if(rf_ != nullptr) { //校验索引完整性
            examine();
        }
    }
    Iter *newIter();
};

#endif