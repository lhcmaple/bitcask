#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <cassert>

#include "env.h"
#include "config.h"

using std::unordered_map;
using std::string_view;

struct fdNode {
    uint64_t file_id; // key
    RandomReadFile *rf; // value
    int ref;
    fdNode *prev;
    fdNode *next;
};

// 获取一个读文件句柄时, 查询fdmap中是否已经存在, 存在则直接返回对应的fdNode指针, 否则清除最近最久未使用的fdNode, 并插入新的fdNode
class LRUCache {
private:
    size_t fd_count_;
    fdNode fdList;
    unordered_map<uint64_t, fdNode *> fdmap;

    LRUCache();
public:
    fdNode *get(uint64_t file_id, const string_view &fname);
    void release(fdNode *node);
    void clearCache();
    ~LRUCache();
    static LRUCache *globalLRUCache() {
        static LRUCache cache;
        return &cache;
    }
};

#endif