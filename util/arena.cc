#include "arena.h"

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