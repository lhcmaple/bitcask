#include "logbuilder.h"

#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    assert(argc >= 2);
    HashTable *ht = new HashTable();
    LogBuilder *lb = LogBuilder::newLogBuilder(argv[1], ht);
    assert(lb != nullptr);
    Handle handle;
    for(int k = 0; k < 10; k++) {
        printf("[%c]", 'a' + k);
        string key, value;
        key.push_back('a' + k);
        value.push_back('a' + k);
        lb->append(key, value, &handle);
        printf("sequence, offset, size: %ld, %ld, %ld, \n", 
            handle.sequence, handle.offset, handle.size);
    }
    delete lb;
    delete ht;
    ht = new HashTable();
    lb = LogBuilder::newLogBuilder(argv[1], ht);
    Iter *it = ht->newIter();
    it->seekToFirst();
    for(; it->isValid(); it->next()) {
        Node *node = static_cast<Node *>(it->get());
        printf("%s, %ld\n", node->key, node->handle.sequence);
    }
    delete lb;
    delete ht;
    return 0;
}