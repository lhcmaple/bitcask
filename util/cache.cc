#include "cache.h"

LRUCache::LRUCache() {
    assert(LRUCACHE_SIZE > 0);
    mutex_ = Env::globalEnv()->newMutex();
    fd_count_ = 0;
    fdList_.prev = &fdList_;
    fdList_.next = &fdList_;
}

fdNode *LRUCache::get(uint64_t file_id, const string_view &fname) {
    fdNode *cur = nullptr;
    GUARD_BEGIN(mutex_)
    if(fdmap_.count(file_id) == 1) {
        fdNode *ret = fdmap_[file_id];
        ret->prev->next = ret->next;
        ret->next->prev = ret->prev;
        ret->next = fdList_.next;
        ret->prev = &fdList_;
        fdList_.next->prev = ret;
        fdList_.next = ret;
        ret->ref++;
        return ret;
    }
    while(fd_count_ >= LRUCACHE_SIZE) {
        fdNode *dnode = fdList_.prev;
        dnode->prev->next = &fdList_;
        dnode->next->prev = dnode->prev;
        fdmap_.erase(dnode->file_id);
        release(dnode);
        fd_count_--;
    }
    fd_count_++;
    cur = new fdNode;
    fdmap_[file_id] = cur;
    cur->file_id = file_id;
    cur->ref = 2;
    cur->rf = Env::globalEnv()->newRandomReadFile(fname);
    cur->next = fdList_.next;
    cur->prev = &fdList_;
    fdList_.next->prev = cur;
    fdList_.next = cur;
    GUARD_END
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
    fdNode *cur = nullptr;
    GUARD_BEGIN(mutex_)
    cur = fdList_.next;
    while(cur != &fdList_) {
        fdNode *next = cur->next;
        release(cur);
        cur = next;
    }
    fdmap_.clear();
    fd_count_ = 0;
    fdList_.next = &fdList_;
    fdList_.prev = &fdList_;
    GUARD_END
}

LRUCache::~LRUCache() {
    clearCache();
    delete mutex_;
}
