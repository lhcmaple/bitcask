#include <unordered_map>
#include <string>
#include <vector>

#include "db.h"

using namespace std;

#define N 10000

int main(int argc, char *argv[]) {
    // performance and correctness
    DB *db = DB::open(argv[1]);
    for(int k = 0; k < 10; k++) {
        vector<pair<string, string>> kv(N);
        for(int i = 0; i < N; i++) {
            kv[i] = {to_string(i + 1'000'000'000), to_string(i + 1'000'000'000)};
        }
        unordered_map<string, string> base;
        clock_t t_base, t_ht, dt_base = 0, dt_ht = 0;

        t_base = clock();
        for(int i = 0; i < N; i++) {
            base[kv[i].first] = kv[i].second;
        }
        dt_base += clock() - t_base;
        t_ht = clock();
        for(int i = 0; i < N; i++) {
            db->put(kv[i].first, kv[i].second);
        }
        dt_ht += clock() - t_ht;

        t_base = clock();
        for(int i = 0; i < N; i += 2) {
            base.erase(kv[i].first);
        }
        dt_base += clock() - t_base;
        t_ht = clock();
        for(int i = 0; i < N; i += 2) {
            db->del(kv[i].first);
        }
        dt_ht += clock() - t_ht;

        vector<bool> owned1(N, true), owned2(N, true);
        vector<string> value1(N), value2(N);

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
            if(db->get(kv[i].first, &value2[i]) < 0) {
                owned2[i] = false;
            }
        }
        dt_ht += clock() - t_ht;
        Iter *iter = db->newIter();
        iter->seekToFirst();
        bool isequal = true;
        while(iter->isValid()) {
            pair<string, string> *kv = static_cast<pair<string, string> *>(iter->get());
            if(base.count(kv->first) == 0 || base[kv->first] != kv->second) {
                isequal = false;
                break;
            }
            base.erase(kv->first);
            iter->next();
        }
        const char *logic[2] = {"false", "true"};
        printf("operations correctness:       %s, %s, %s\n", logic[owned1 == owned2], logic[value1 == value2], logic[isequal]);
        printf("base structure time consumed: %f seconds\n", (double)dt_base / CLOCKS_PER_SEC);
        printf("hashtable time consumed:      %f seconds\n", (double)dt_ht / CLOCKS_PER_SEC);
        delete iter;
        db->compact(true);
    }
    delete db;
    return 0;
}