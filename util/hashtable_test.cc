#include <unordered_map>
#include <string>
#include <vector>

#include "hashtable.h"

using namespace std;

#define N 10000

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
    for(int i = 0; i < N; i++) {
        ht.insert(kv[i].second, Handle(), kv[i].first);
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
            value2[i] = cur->sequence;
        }
    }
    dt_ht += clock() - t_ht;

    const char *logic[2] = {"false", "true"};
    printf("operations correctness:       %s, %s\n", logic[owned1 == owned2], logic[value1 == value2]);
    printf("base structure time consumed: %f seconds\n", (double)dt_base / CLOCKS_PER_SEC);
    printf("hashtable time consumed:      %f seconds\n", (double)dt_ht / CLOCKS_PER_SEC);
    return 0;
}