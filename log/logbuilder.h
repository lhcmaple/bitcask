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
    string db_name_;
    fileNode activelist_;
    fileNode compactlist_; // empty list unless being compacted
    WriteFile *wf_;
    size_t file_size_;
    string file_buf_;
    string hashindex_;
    uint64_t cur_fileid_;
    uint64_t cur_sequence_;

    int dump();
public:
    LogBuilder(const string_view &db_name) : db_name_(db_name) {
        activelist_.prev = &activelist_;
        activelist_.next = &activelist_;
        compactlist_.prev = &compactlist_;
        compactlist_.next = &compactlist_;
        cur_fileid_ = 0;
        cur_sequence_ = 0;
    }
    static LogBuilder *newLogBuilder(const string_view &db_name, HashTable *htable);
    int append(const string_view &key, const string_view &value, 
        Handle *handle);
    void compaction();
    fileNode *compactFile();
    ~LogBuilder();
};

#endif