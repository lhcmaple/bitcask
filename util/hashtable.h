#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <cstring>

#include "arena.h"
#include "hash.h"

using std::string_view;

struct Handle {

};

struct Node {
    Node *hash_next;
    uint32_t khash;
    uint64_t sequence;
    Handle handle;
    size_t key_length;
    char key[1];
};

class HashTable {
private:
    Node **table_;
    size_t table_size_;
    size_t count_;
    Arena *arena_;

    void clearTable();
    Node **find(const string_view &key, uint32_t khash);
    void resize();
public:
    HashTable(size_t buckets = 10);
    ~HashTable();

    const Node *find(const string_view &key);
    const Node *insert(uint64_t sequence, const Handle &handle, const string_view &key);
    void erase(const string_view &key);
};

#endif