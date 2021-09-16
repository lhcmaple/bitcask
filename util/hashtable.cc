#include "hashtable.h"

static bool byteEqual(const string_view &sv1, const string_view &sv2) {
    if(sv1.size() != sv2.size()) {
        return false;
    }
    for(int k = 0; k < sv1.size(); k++) {
        if(sv1[k] != sv2[k]) {
            return false;
        }
    }
    return true;
}

HashTable::HashTable(size_t buckets) : arena_(new Arena), table_size_(buckets), count_(0) {
    table_ = static_cast<Node **>(arena_->alloc(sizeof(Node *) * table_size_));
    memset(table_, 0, sizeof(Node *) * table_size_);
}

HashTable::~HashTable() {
    clearTable();
    delete arena_;
}

const Node *HashTable::find(const string_view &key) {
    uint32_t khash = Hash(key.data(), key.size());
    return *find(key, khash);
}

Node **HashTable::find(const string_view &key, uint32_t khash) {
    uint32_t kindex = khash % table_size_;
    Node **pcur = &table_[kindex];
    while(*pcur && 
        !byteEqual(string_view((*pcur)->key, (*pcur)->key_length), key)) {
        pcur = &((*pcur)->hash_next);
    }
    return pcur;
}

const Node *HashTable::insert(uint64_t sequence, const Handle &handle, const string_view &key) {
    uint32_t khash = Hash(key.data(), key.size());
    Node *cur = *find(key, khash);
    if(cur == nullptr) {
        cur = static_cast<Node *>(arena_->alloc(sizeof(Node) + key.size()));
        size_t kindex = khash % table_size_;
        cur->key_length = key.size();
        key.copy(cur->key, key.size());
        cur->khash = khash;
        cur->hash_next = table_[kindex];
        table_[kindex] = cur;
        count_++;
        if(count_ > table_size_ * 1.2) {
            resize();
        }
    }
    cur->sequence = sequence;
    cur->handle = handle;
    return cur;
}

void HashTable::clearTable() {
    for(int k = 0; k < table_size_; k++) {
        Node *cur = table_[k];
        while(cur) {
            Node *next = cur->hash_next;
            arena_->release(cur);
            cur = next;
        }
    }
    arena_->release(table_);
}

void HashTable::resize() {
    Node **oldtable = table_;
    size_t oldsize = table_size_;
    table_size_ *= 2;
    table_ = static_cast<Node **>(arena_->alloc(sizeof(Node *) * table_size_));
    memset(table_, 0, sizeof(Node *) * table_size_);
    for(int k = 0; k < oldsize; k++) {
        Node *cur = oldtable[k];
        while(cur) {
            Node *next = cur->hash_next;
            size_t kindex = cur->khash % table_size_;
            assert(kindex < table_size_);
            cur->hash_next = table_[kindex];
            table_[kindex] = cur;
            cur = next;
        }
    }
    arena_->release(oldtable);
}

void HashTable::erase(const string_view &key) {
    uint32_t khash = Hash(key.data(), key.size());
    Node **pcur = find(key, khash);
    assert(*pcur != nullptr);
    Node *cur = *pcur;
    *pcur = cur->hash_next;
    count_--;
    arena_->release(cur);
}