#include "logreader.h"

#include <cassert>
#include <string>
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    assert(argc >= 4);
    if(string(argv[3]) == ".log") {
        LogReader *lr = LogReader::newLogReader(stoull(argv[2]), argv[1]);
        assert(lr != nullptr);
        Iter *iter = lr->newIter();
        assert(iter != nullptr);
        iter->seekToFirst();
        for(; iter->isValid(); iter->next()) {
            LogContent *lc = static_cast<LogContent *>(iter->get());
            printf("[seq, key, value] : %20ld, %s, %s\n", 
                lc->sequence, lc->key.c_str(), lc->value.c_str());
        }
        delete iter;
        delete lr;
    } else if(string(argv[3]) == ".hindex") {
        HIndexReader *hir = HIndexReader::newHIndexReader(stoull(argv[2]), argv[1]);
        assert(hir != nullptr);
        Iter *iter = hir->newIter();
        assert(iter != nullptr);
        iter->seekToFirst();
        for(; iter->isValid(); iter->next()) {
            HIndexContent *hic = static_cast<HIndexContent *>(iter->get());
            printf("[seq, key, offset, size] : %20ld, %s, %15d, %15d\n", 
                hic->sequence, hic->key.c_str(), hic->offset, hic->size);
        }
        delete iter;
        delete hir;
    }
    return 0;
}