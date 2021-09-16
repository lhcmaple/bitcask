//arena 是一个内存分配器, 最大的功能是将许多小块内存的申请转成大块内存的申请, 尽可能地避免内存碎片的生成
//external synchronization is needed while being accessed by multiple threads

#ifndef ARENA_H
#define ARENA_H

#include <cstddef>
#include <cassert>
#include <memory>
#include <map>

#include "config.h"

using std::map;

class Arena {
private:
    map<void *, int> used_mem_;
    void *base_;
    void *mem_;

    inline void *advance(void *mem, size_t step) {
        return static_cast<void *>(static_cast<char *>(mem) + step);
    }
public:
    Arena() : base_(nullptr), mem_(nullptr) {}
    ~Arena() {
        release(base_);
    }
    void *alloc(size_t size);
    void release(void *mem);
};

#endif