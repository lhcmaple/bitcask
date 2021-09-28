#include "cache.h"

LRUCache::LRUCache() {
    assert(LRUCACHE_SIZE > 0);
    fd_count_ = 0;
    fdList.prev = &fdList;
    fdList.next = &fdList;
}

fdNode *LRUCache::get(uint64_t file_id, const string_view &fname) {
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
    while(fd_count_ >= LRUCACHE_SIZE) {
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
    cur->rf = Env::globalEnv()->newRandomReadFile(fname);
    return cur;
}

void LRUCache::release(fdNode *node) {
    node->ref--;
    if(node->ref == 0) {
        delete node->rf;
        delete node;
    }
}

void LRUCache::clearCache() {
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

LRUCache::~LRUCache() {
    clearCache();
}
