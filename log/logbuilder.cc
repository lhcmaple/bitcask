#include "logbuilder.h"
#include "crc32c.h"
#include "hashtable.h"
#include "logreader.h"

#include <algorithm>
#include <unordered_set>

using std::unordered_set;

static bool isLog(const string_view &file) {
    return file.size() >= 5 && file.substr(file.size() - 4, 4) == ".log";
}

static bool isHIndex(const string_view &file) {
    return file.size() >= 8 && file.substr(file.size() - 7, 7) == ".hindex";
}

LogBuilder *LogBuilder::newLogBuilder(const string_view &db_name, HashTable *htable) {
    LogBuilder *lb = new LogBuilder(db_name);
    vector<string> files;
    if(Env::globalEnv()->readDir(db_name, &files) < 0) {
        delete lb;
        return nullptr;
    }
    std::sort(files.begin(), files.end(), [](string &s1, string &s2) {
        if((s1 == "." || s1 == "..") && (s2 == "." || s2 == "..")) {
            return s1 < s2;
        }
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
                Env::globalEnv()->rmFile(lb->db_name_ + "/" + f);
            }
        }
    }
    cur = lb->activelist_.prev;
    while(cur != &lb->activelist_) {
        uint64_t file_id = cur->file_id;
        lb->cur_fileid_ = std::max(lb->cur_fileid_, file_id);
        HIndexReader *hir = HIndexReader::newHIndexReader(file_id, lb->db_name_);
        if(hir != nullptr) {
            Iter *it = hir->newIter();
            it->seekToFirst();
            for(; it->isValid(); it->next()) {
                HIndexContent *hic = static_cast<HIndexContent *>(it->get());
                lb->cur_sequence_ = std::max(lb->cur_sequence_, hic->sequence);
                if(hic->key[0] != '#') {
                    Handle handle;
                    handle.file_id = file_id;
                    handle.offset = hic->offset;
                    handle.sequence = hic->sequence;
                    handle.size = hic->size;
                    htable->insert(string_view(hic->key.data() + 1, hic->key.size() - 1), handle);
                } else if(htable->find(string_view(hic->key.data() + 1, hic->key.size() - 1)) != nullptr) {
                    htable->erase(string_view(hic->key.data() + 1, hic->key.size() - 1));
                }
            }
            delete it;
        } else {
            LogReader *lr = LogReader::newLogReader(file_id, lb->db_name_);
            string hash_index;
            hash_index.resize(8);
            Iter *it = lr->newIter();
            it->seekToFirst();
            uint32_t offset = 0;
            for(; it->isValid(); it->next()) {
                LogContent *lc = static_cast<LogContent *>(it->get());
                lb->cur_sequence_ = std::max(lb->cur_sequence_, lc->sequence);
                char *cur = nullptr;
                size_t cur_off = hash_index.size();
                hash_index.resize(hash_index.size() + 20 + lc->key.size());
                cur = hash_index.data() + cur_off;
                *reinterpret_cast<uint64_t *>(cur) = lc->sequence;
                *reinterpret_cast<uint32_t *>(cur + 8) = offset;
                *reinterpret_cast<uint32_t *>(cur + 12) = lc->key.size() + lc->value.size() + 20;
                *reinterpret_cast<uint32_t *>(cur + 16) = lc->key.size() + 1;
                cur[20] = '#';
                lc->key.copy(cur + 21, lc->key.size());

                if(!lc->value.empty()) {
                    Handle handle;
                    handle.file_id = file_id;
                    handle.offset = offset;
                    handle.sequence = lc->sequence;
                    handle.size = lc->key.size() + lc->value.size() + 20;
                    htable->insert(lc->key, handle);
                } else if(htable->find(lc->key) != nullptr) {
                    htable->erase(lc->key);
                }
                offset += lc->key.size() + lc->value.size() + 20;
            }
            *reinterpret_cast<uint32_t *>(hash_index.data() + 4) = hash_index.size();
            *reinterpret_cast<uint32_t *>(hash_index.data()) = Mask(Value(hash_index.data() + 4, hash_index.size() - 4));
            if(hash_index.size() > 8) {
                WriteFile *hwf = Env::globalEnv()->newWriteFile(lb->db_name_ + "/" + std::to_string(file_id) + ".hindex");
                hwf->write(hash_index);
                delete hwf;
            }
            delete lr;
            delete it;
        }
        delete hir;
        cur = cur->prev;
    }
    lb->wf_ = Env::globalEnv()->newWriteFile(lb->db_name_ + "/" + std::to_string(++lb->cur_fileid_) + ".log");
    lb->file_size_ = 0;
    cur = new fileNode;
    cur->file_id = lb->cur_fileid_;
    cur->next = lb->activelist_.next;
    cur->prev = &lb->activelist_;
    lb->activelist_.next->prev = cur;
    lb->activelist_.next = cur;
    lb->hashindex_.resize(8);
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
    if(wf_->write(file_buf_) != file_buf_.size()) {
        return -1;
    }
    wf_->flush();

    char *hdata = nullptr;
    size_t hdata_off = hashindex_.size();
    hashindex_.resize(hashindex_.size() + 8 + 4 + 4 + 4 + key.size() + 1); // 补锅
    hdata = hashindex_.data() + hdata_off;
    *reinterpret_cast<uint64_t *>(hdata) = cur_sequence_;
    *reinterpret_cast<uint32_t *>(hdata + 8) = file_size_;
    *reinterpret_cast<uint32_t *>(hdata + 12) = file_buf_.size();
    *reinterpret_cast<uint32_t *>(hdata + 16) = key.size() + 1;
    hdata[20] = '#';
    key.copy(hdata + 21, key.size());

    handle->file_id = cur_fileid_;
    handle->offset = file_size_;
    handle->sequence = cur_sequence_;
    handle->size = file_buf_.size();
    file_size_ += file_buf_.size();
    if(file_size_ >= LOG_FILE_SIZE_THRESHOLD) {
        return dump(false);
    }
    return 0;
}

int LogBuilder::dump(bool isexit) {
    assert(activelist_.next != &activelist_);

    uint64_t fid = activelist_.next->file_id;
    WriteFile *hwf = Env::globalEnv()->newWriteFile(db_name_ + "/" + std::to_string(fid) + ".hindex");
    if(hwf == nullptr) {
        return -1;
    }
    assert(hashindex_.size() >= 8);
    *reinterpret_cast<uint32_t *>(hashindex_.data() + 4) = hashindex_.size();
    *reinterpret_cast<uint32_t *>(hashindex_.data()) = Mask(Value(hashindex_.data() + 4, hashindex_.size() - 4));
    if(hwf->write(hashindex_) != hashindex_.size()) {
        delete hwf;
        return -1;
    }
    hwf->flush();
    delete hwf;

    if(!isexit) {
        fileNode *fnode = new fileNode;
        fnode->file_id = ++cur_fileid_;
        fnode->next = activelist_.next;
        fnode->prev = &activelist_;
        activelist_.next->prev = fnode;
        activelist_.next = fnode;
        delete wf_;
        wf_ = Env::globalEnv()->newWriteFile(db_name_ + "/" + std::to_string(fnode->file_id) + ".log");
        hashindex_.resize(8);
        file_size_ = 0;
    }
    return 0;
}

void LogBuilder::compaction() {
    assert(activelist_.next != &activelist_);
    fileNode *head = activelist_.next, *tail = activelist_.prev;
    compactlist_.next = head;
    head->prev = &compactlist_;
    compactlist_.prev = tail;
    tail->next = &compactlist_;
    
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
    if(hashindex_.size() > 8) {
        dump(true);
    }
    delete wf_;
    fileNode *cur = &activelist_;
    while(cur->next != cur) {
        fileNode *tmp = cur->next;
        cur->next = tmp->next;
        delete tmp;
    }
    assert(&compactlist_ == compactlist_.next);
}