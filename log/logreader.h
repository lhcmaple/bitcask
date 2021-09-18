#ifndef LOGREADER_H
#define LOGREADER_H

#include "iter.h"
#include "env.h"
#include "hashtable.h"

#include <cstdint>

class LogReader {
private:
    uint64_t fileid_;
    RandomReadFile *rf_;

    class Iterator;
public:
    static LogReader *newLogReader(uint64_t fileid);
    LogReader(uint64_t fileid) : fileid_(fileid) {
        rf_ = Env::globalEnv()->newRandomReadFile(std::to_string(fileid_) + ".log");
    }
    void seek(const Handle &handle, string *value);
    ~LogReader() {
        delete rf_;
    }
    Iter *newIter();
};

class HIndexReader {

};

#endif