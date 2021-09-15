#include "arena.h"

#include <vector>
#include <utility>
#include <cstdio>

using std::vector;
using std::pair;

#define N 1000

int main(int argc, char *argv[]) {
    Arena arena;
    vector<pair<void *, bool>> buf(2 * N);
    for(int i = 0; i < N; i++) {
        buf[2 * i] = {arena.alloc(rand() % (ARENA_BLOCK_SIZE)), true};
        buf[2 * i + 1] = {arena.alloc(rand() % ARENA_BLOCK_SIZE / 8), true};
        int sindex = rand() % (2 * i + 2);
        if(buf[sindex].second) {
            arena.release(buf[sindex].first);
            buf[sindex].second = false;
        }
    }
    for(int i = 0; i < 2 * N; i++) {
        if(buf[i].second) {
            arena.release(buf[i].first);
        }
    }
    return 0;
}