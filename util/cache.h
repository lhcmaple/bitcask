#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>

#include "env.h"

using std::unordered_map;
using std::string;

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
    size_t fd_cachesize_;
    string db_name_;

    fdNode fdList;

    unordered_map<uint64_t, fdNode *> fdmap;
public:
    LRUCache(size_t fd_cachesize, const string &db_name) 
        : fd_cachesize_(fd_cachesize), db_name_(db_name) {
        assert(fd_cachesize_ > 0);
        fd_count_ = 0;
        fdList.prev = &fdList;
        fdList.next = &fdList;
    }
    fdNode *get(uint64_t file_id) {
        if(fdmap.count(file_id) == 1) {
            fdNode *ret = fdmap[file_id];
            ret->prev->next = ret->next;
            ret->next->prev = ret->prev;
            ret->next = fdList.next;
            ret->prev = &fdList;
            fdList.next->prev = ret;
            fdList.next = ret;
            ret->ref++;
            return ret;
        }
        while(fd_count_ >= fd_cachesize_) {
            fdNode *dnode = fdList.prev;
            dnode->prev->next = &fdList;
            dnode->next->prev = dnode->prev;
            fdmap.erase(dnode->file_id);
            release(dnode);
            fd_count_--;
        }
        fd_count_++;
        fdNode *cur = new fdNode;
        fdmap[file_id] = cur;
        cur->file_id = file_id;
        cur->next = fdList.next;
        cur->prev = &fdList;
        fdList.next->prev = cur;
        fdList.next = cur;
        cur->ref = 2;
        cur->rf = Env::globalEnv()->newRandomReadFile(db_name_ + "/" + std::to_string(file_id) + ".log");
        return cur;
    }
    void release(fdNode *node) {
        node->ref--;
        if(node->ref == 0) {
            delete node->rf;
            delete node;
        }
    }
    void clearCache() {
        fdNode *cur = fdList.next;
        while(cur != &fdList) {
            fdNode *next = cur->next;
            release(cur);
            cur = next;
        }
        fdmap.clear();
        fd_count_ = 0;
        fdList.next = &fdList;
        fdList.prev = &fdList;
    }
    ~LRUCache() {
        clearCache();
    }
};

#endif