#ifndef LOGREADER_H
#define LOGREADER_H

#include "iter.h"
#include "env.h"

#include <cstdint>

class logreader {
private:
    uint64_t fileid_;
    RandomReadFile *rf;
public:
    logreader(uint64_t fileid) : fileid_(fileid) {
        rf = Env::globalEnv()->newRandomReadFile(std::to_string(fileid_) + ".log");
    }
    Iter *newIter() {

    }
};

#endif