//arena 是一个内存分配器, 最大的功能是将许多小块内存的申请转成大块内存的申请, 尽可能地避免内存碎片的生成
//external synchronization is needed while being accessed by multiple threads

#ifndef _ARENA_H
#define _ARENA_H

#include <cstddef>
#include <cassert>
#include <memory>
#include <map>

#include <config.h>

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

void *Arena::alloc(size_t size) {
    void *ret = nullptr;
    if(size >= ARENA_BLOCK_SIZE / 8) {
        ret = malloc(size);
        used_mem_[ret] = 1;
        return ret;
    }
    if(base_ == nullptr || advance(mem_, size) > advance(base_, ARENA_BLOCK_SIZE)) {
        release(base_);
        base_ = malloc(ARENA_BLOCK_SIZE);
        mem_ = base_;
        used_mem_.emplace(base_, 1);
    }
    used_mem_[base_]++;
    ret = mem_;
    mem_ = advance(mem_, size);
    return ret;
}

void Arena::release(void *mem) {
    if(mem == nullptr) {
        return;
    }
    auto ibase = used_mem_.upper_bound(mem);
    assert(ibase != used_mem_.begin());
    ibase--;
    assert(static_cast<char *>(mem) - static_cast<char *>(ibase->first) < ARENA_BLOCK_SIZE);
    ibase->second--;
    if(ibase->second == 0) {
        free(ibase->first);
        used_mem_.erase(ibase);
    }
}

#endif