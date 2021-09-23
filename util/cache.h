#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>

using std::unordered_map;
using std::string;

struct fdNode {
    uint64_t file_id; // key
    int fd; // value
    int ref;
    fdNode *prev;
    fdNode *next;
};

struct kvNode {
    string key;
    string value;
    int ref;
    kvNode *prev;
    kvNode *next;
};

class LRUCache {
private:
    size_t fd_count_;
    size_t kv_count_;
    size_t fd_cachesize_;
    size_t kv_cachesize_;

    fdNode fdList;
    kvNode kvList;

    unordered_map<uint64_t, fdNode *> fdmap;
    unordered_map<string, kvNode *> kvmap;
public:
    LRUCache(size_t fd_cachesize, size_t kv_cachesize) 
        : fd_cachesize_(fd_cachesize), kv_cachesize_(kv_cachesize) {
        fd_count_ = 0;
        kv_count_ = 0;
        fdList.prev = &fdList;
        fdList.next = &fdList;
        kvList.prev = &kvList;
        kvList.next = &kvList;
    }
    void insertkv(const string &key, const string &value) {
        if(kvmap.count(key) == 1) {
            kvNode *cur = kvmap[key];
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            kv_count_--;
            releasekv(cur);
        }
        while(kv_count_ >= kv_cachesize_) {
            kvNode *cur = kvList.prev;
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            kv_count_--;
            releasekv(cur);
        }
        kvNode *cur = new kvNode;
        cur->key = key;
        cur->value = value;
        cur->ref = 1; // ref by LRUCache
        cur->next = kvList.next;
        cur->prev = &kvList;
        kvList.next->prev = cur;
        kvList.next = cur;
        kvmap[key] = cur;
    }
    void *get(void *key) {

    }
    void releasekv(kvNode *node) {
        node->ref--;
        if(node->ref == 0) {
            delete node;
        }
    }
    void clearCache() {

    }
    ~LRUCache() {

    }
};
#endif