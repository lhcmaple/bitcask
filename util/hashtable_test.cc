#include <unordered_map>
#include <string>
#include <vector>

#include "hashtable.h"

using namespace std;

#define N 100000

int main(int argc, char *argv[]) {
    // performance and correctness
    vector<pair<string, uint64_t>> kv(N);
    for(int i = 0; i < N; i++) {
        kv[i] = {to_string(i + 1'000'000'000), i};
    }
    unordered_map<string, uint64_t> base;
    HashTable ht;
    clock_t t_base, t_ht, dt_base = 0, dt_ht = 0;

    t_base = clock();
    for(int i = 0; i < N; i++) {
        base[kv[i].first] = kv[i].second;
    }
    dt_base += clock() - t_base;
    t_ht = clock();
    Handle handle;
    for(int i = 0; i < N; i++) {
        handle.sequence = kv[i].second;
        ht.insert(kv[i].first, handle);
    }
    dt_ht += clock() - t_ht;

    t_base = clock();
    for(int i = 0; i < N; i += 2) {
        base.erase(kv[i].first);
    }
    dt_base += clock() - t_base;
    t_ht = clock();
    for(int i = 0; i < N; i += 2) {
        ht.erase(kv[i].first);
    }
    dt_ht += clock() - t_ht;

    vector<bool> owned1(N, true), owned2(N, true);
    vector<uint64_t> value1(N, 0), value2(N, 0);

    t_base = clock();
    for(int i = 0; i < N; i++) {
        if(base.count(kv[i].first) == 0) {
            owned1[i] = false;
        } else {
            value1[i] = base[kv[i].first];
        }
    }
    dt_base += clock() - t_base;
    t_ht = clock();
    for(int i = 0; i < N; i++) {
        const Node *cur = ht.find(kv[i].first);
        if(cur == nullptr) {
            owned2[i] = false;
        } else {
            value2[i] = cur->handle.sequence;
        }
    }
    dt_ht += clock() - t_ht;
    Iter *iter = ht.newIter();
    iter->seekToFirst();
    bool isequal = true;
    while(iter->isValid()) {
        Node *node = static_cast<Node *>(iter->get());
        if(base.count(string(node->key, node->key_length)) == 0) {
            isequal = false;
            break;
        }
        base.erase(string(node->key, node->key_length));
        iter->next();
    }
    const char *logic[2] = {"false", "true"};
    printf("operations correctness:       %s, %s, %s\n", logic[owned1 == owned2], logic[value1 == value2], logic[isequal]);
    printf("base structure time consumed: %f seconds\n", (double)dt_base / CLOCKS_PER_SEC);
    printf("hashtable time consumed:      %f seconds\n", (double)dt_ht / CLOCKS_PER_SEC);
    return 0;
}