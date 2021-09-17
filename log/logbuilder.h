#ifndef LOGBUILDER_H
#define LOGBUILDER_H

#include <vector>
#include <string>

#include "hashtable.h"
#include "env.h"

using std::string_view;
using std::string;
using std::vector;

struct fileNode {
    uint64_t file_id;
    fileNode *prev;
    fileNode *next;
};

class LogBuilder {
private:
    fileNode activelist_;
    fileNode compactlist_;
    WriteFile *wf_;
    size_t file_size_;
    string file_buf_;
    string hashindex_;
    uint64_t cur_fileid_;
    uint64_t cur_sequence_;

    int write(const string_view &data);
public:
    LogBuilder() {
        activelist_.prev = &activelist_;
        activelist_.next = &activelist_;
        compactlist_.prev = &compactlist_;
        compactlist_.next = &compactlist_;
    }
    LogBuilder *newLogBuilder(const string_view &db_name, HashTable *htable);
    int append(const string_view &key, const string_view &value, 
        uint64_t sequence);
    int compact(bool background = true);
    ~LogBuilder() {
        delete wf_;
    }
};

#endif