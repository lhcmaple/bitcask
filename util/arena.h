//arena 是一个内存分配器, 最大的功能是将许多小块内存的申请转成大块内存的申请, 尽可能地避免内存碎片的生成

#ifndef _ARENA_H
#define _ARENA_H

#include <cstddef>

#include <config.h>

class Arena {
private:
    vector<void *> mem_;
public:
    void *alloc(size_t size);
};

#endif