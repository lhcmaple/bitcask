#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>

#include "db.h"
#include "iter.h"

using namespace std;

#define N 10000

string rand_data;
vector<string> rand_key;

// key: uint64_t, value: N * 32K
pair<double, double> writeRandom32K(DB *db) {
    clock_t stime = clock();
    for(int i = 0; i < N; i++) {
        int pos = rand() % (rand_data.size() - 1024 * 32 + 1);
        db->put(rand_key[i], string_view(rand_data.data() + pos, 1024 * 32));
    }
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / dt / 1024, N / dt};
}

pair<double, double> readRandom(DB *db) {
    clock_t stime = clock();
    for(int i = 0; i < N; i++) {
        string value;
        db->get(rand_key[i], &value);
    }
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / dt / 1024, N / dt};
}

pair<double, double> del(DB *db) {
    clock_t stime = clock();
    for(int i = 0; i < N / 2; i++) {
        db->del(rand_key[i]);
    }
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / 2 / dt / 1024, N / 2 / dt};
}

pair<double, double> readLost(DB *db) {
    clock_t stime = clock();
    for(int i = 0; i < N / 2; i++) {
        string value;
        db->get(rand_key[i], &value);
    }
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / 2 / dt / 1024, N / 2 / dt};
}

pair<double, double> delLost(DB *db) {
    clock_t stime = clock();
    for(int i = 0; i < N / 2; i++) {
        db->del(rand_key[i]);
    }
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / 2 / dt / 1024, N / 2 / dt};
}

pair<double, double> compact(DB *db) {
    clock_t stime = clock();
    db->compact(false);
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / dt / 1024, N / dt};
}

pair<double, double> iterate(DB *db) {
    clock_t stime = clock();
    Iter *iter = db->newIter();
    iter->seekToFirst();
    for(; iter->isValid(); iter->next()) {
        iter->get();
    }
    delete iter;
    double dt = static_cast<double>(clock() - stime) / CLOCKS_PER_SEC;
    return {32 * N / 2 / dt / 1024, N / 2 / dt};
}

int main(int argc, char *argv[]) {
    for(int i = 0; i < 1024 * 1024; i++) {
        rand_data += to_string(rand());
    }
    for(int i = 0; i < N; i++) {
        rand_key.push_back(to_string(rand()));
    }
    DB *db = DB::open(argv[1]);
    // 写, (顺序写, 随机写, 无差别)
    auto rate = writeRandom32K(db);
    printf("write rate(32K) : %.3f Mb/s, %.1f Ops/s\n", rate.first, rate.second);
    // 读, (顺序读, 随机读, 无差别)
    rate = readRandom(db);
    printf("read rate(32K) : %.3f Mb/s, %.1f Ops/s\n", rate.first, rate.second);
    // 删除
    rate = del(db);
    printf("del rate(32K) : xxxx Mb/s, %.1f Ops/s\n", rate.second);
    // 读缺失数据
    rate = readLost(db);
    printf("read lost rate(32K) : xxxx Mb/s, %.1f Ops/s\n", rate.second);
    // 删除缺失数据
    rate = delLost(db);
    printf("del lost rate(32K) : xxxx Mb/s, %.1f Ops/s\n", rate.second);
    // 压缩
    rate = compact(db);
    printf("compact rate(32K) : %.3f Mb/s, %.1f Ops/s\n", rate.first, rate.second);
    // 迭代
    rate = iterate(db);
    printf("iterate rate(32K) : %.3f Mb/s, %.1f Ops/s\n", rate.first, rate.second);
    delete db;
    return 0;
}