#include "logbuilder.h"
#include "crc32c.h"
#include "hashtable.h"
#include "logreader.h"

#include <algorithm>
#include <unordered_set>

using std::unordered_set;

bool isLog(const string_view &file) {
    return file.size() >= 5 && file.substr(file.size() - 4, 4) == ".log";
}

bool isHIndex(const string_view &file) {
    return file.size() >= 8 && file.substr(file.size() - 8, 7) == ".hindex";
}

LogBuilder *LogBuilder::newLogBuilder(const string_view &db_name, HashTable *htable) {
    LogBuilder *lb = new LogBuilder;
    vector<string> files;
    if(Env::globalEnv()->readDir(db_name, &files) < 0) {
        delete lb;
        return nullptr;
    }
    std::sort(files.begin(), files.end(), [](string &s1, string &s2) {
        if(s1 == "." || s1 == "..") {
            return true;
        }
        if(s2 == "." || s2 == "..") {
            return false;
        }
        uint64_t id1 = std::stoull(s1), id2 = std::stoull(s2);
        if(id1 != id2) {
            return id1 < id2;
        }
        return isLog(s1);
    });
    fileNode *cur = &lb->activelist_;
    unordered_set<uint64_t> log_set;
    for(auto &f : files) {
        if(isLog(f)) {
            fileNode *fnode = new fileNode;
            fnode->file_id = std::stoull(f);
            fnode->next = cur->next;
            fnode->prev = cur;
            cur->next->prev = fnode;
            cur->next = fnode;
            log_set.insert(fnode->file_id);
        } else if(isHIndex(f)) {
            uint64_t id = std::stoull(f);
            if(log_set.count(id) == 0) {
                Env::globalEnv()->rmFile(f);
            }
        }
    }
    cur = lb->activelist_.prev;
    while(cur != &lb->activelist_) {
        uint64_t file_id = cur->file_id;
        HIndexReader *hir = HIndexReader::newHIndexReader(file_id);
        if(hir != nullptr) {
            //.
        } else {
            LogReader *lr = LogReader::newLogReader(file_id);
            WriteFile *wf = Env::globalEnv()->newWriteFile(std::to_string(file_id) + ".hindex", true);
            //
        }
        cur = cur->prev;
    }
    return lb;
}

int LogBuilder::append(const string_view &key, const string_view &value, Handle *handle) {
    assert(wf_ != nullptr);

    ++cur_sequence_; // new sequence
    file_buf_.clear();
    file_buf_.resize(4 + 8 + 4 + 4 + key.size() + value.size());
    *reinterpret_cast<uint64_t *>(file_buf_.data() + 4) = cur_sequence_;
    *reinterpret_cast<uint32_t *>(file_buf_.data() + 12) = key.size();
    *reinterpret_cast<uint32_t *>(file_buf_.data() + 16) = value.size();
    key.copy(file_buf_.data() + 20, key.size());
    value.copy(file_buf_.data() + 20 + key.size(), value.size());
    *reinterpret_cast<uint32_t *>(file_buf_.data()) = Mask(Value(file_buf_.data() + 4, file_buf_.size() - 4));
    if(write(file_buf_) != 0) {
        return -1;
    }

    hashindex_.resize(hashindex_.size() + 8 + 4 + 4 + 4 + key.size());
    char *hdata = hashindex_.data() + hashindex_.size();
    *reinterpret_cast<uint64_t *>(hdata) = cur_sequence_;
    *reinterpret_cast<uint32_t *>(hdata + 8) = file_size_;
    *reinterpret_cast<uint32_t *>(hdata + 12) = file_buf_.size();
    *reinterpret_cast<uint32_t *>(hdata + 16) = key.size();
    key.copy(hdata + 20, key.size());

    handle->file_id = cur_fileid_;
    handle->offset = file_size_;
    handle->sequence = cur_sequence_;
    handle->size = file_buf_.size();
    file_size_ += file_buf_.size();
    if(file_size_ >= LOG_FILE_SIZE_THRESHOLD) {
        return dump();
    }
    return 0;
}

int LogBuilder::dump() {
    assert(activelist_.next != &activelist_);

    uint64_t fid = activelist_.next->file_id;
    WriteFile *hwf = Env::globalEnv()->newWriteFile(std::to_string(fid) + ".hindex", true);
    if(hwf == nullptr) {
        return -1;
    }
    assert(hashindex_.size() >= 8);
    *reinterpret_cast<uint32_t *>(hashindex_.data() + 4) = hashindex_.size() - 8;
    *reinterpret_cast<uint32_t *>(hashindex_.data() + 4) = Mask(Value(hashindex_.data() + 4, hashindex_.size() - 4));
    if(hwf->write(hashindex_) != 0) {
        delete hwf;
        return -1;
    }

    delete hwf;
    delete wf_;
    fileNode *fnode = new fileNode;
    fnode->file_id = ++cur_fileid_;
    fnode->next = activelist_.next;
    fnode->prev = &activelist_;
    activelist_.next->prev = fnode;
    activelist_.next = fnode;
    wf_ = Env::globalEnv()->newWriteFile(std::to_string(fid) + ".log", true);
    hashindex_.resize(8);
}

void LogBuilder::compaction() {
    assert(activelist_.next != &activelist_);
    fileNode *head = activelist_.next, *tail = activelist_.prev;
    compactlist_.next = head;
    head->prev = &compactlist_;
    compactlist_.prev = tail;
    tail->prev = &compactlist_;
    
    fileNode *cur = compactlist_.next;
    cur->next->prev = &compactlist_;
    compactlist_.next = cur->next;
    cur->next = &activelist_;
    cur->prev = &activelist_;
    activelist_.next = cur;
    activelist_.prev = cur;
}

fileNode *LogBuilder::compactFile() {
    if(compactlist_.prev == &compactlist_) {
        return nullptr;
    }
    fileNode *older = compactlist_.prev, *newer = older->prev;
    newer->next = &compactlist_;
    compactlist_.prev = newer;
    older->prev = nullptr;
    older->next = nullptr;
    return older;
}

LogBuilder::~LogBuilder() {
    delete wf_;
    fileNode *cur = &activelist_;
    while(cur->next != cur) {
        fileNode *tmp = cur->next;
        cur->next = tmp->next;
        delete tmp;
    }
    assert(&compactlist_ == compactlist_.next);
}