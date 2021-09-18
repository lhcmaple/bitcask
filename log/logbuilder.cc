#include "logbuilder.h"
#include "crc32c.h"

LogBuilder *LogBuilder::newLogBuilder(const string_view &db_name, HashTable *htable) {
    LogBuilder *lb = new LogBuilder;
    //
    return lb;
}

int LogBuilder::append(const string_view &key, const string_view &value, uint64_t sequence) {
    assert(wf_ != nullptr);
    
    file_buf_.clear();
    file_buf_.resize(4 + 8 + 4 + 4 + key.size() + value.size());
    *reinterpret_cast<uint64_t *>(file_buf_.data() + 4) = sequence;
    *reinterpret_cast<uint32_t *>(file_buf_.data() + 12) = key.size();
    *reinterpret_cast<uint32_t *>(file_buf_.data() + 16) = value.size();
    key.copy(file_buf_.data() + 20, key.size());
    value.copy(file_buf_.data() + 20 + key.size(), value.size());
    *reinterpret_cast<uint32_t *>(file_buf_.data()) = Mask(Value(file_buf_.data() + 4, file_buf_.size() - 4));
    write(file_buf_);

    hashindex_.resize(hashindex_.size() + 8 + 4 + 4 + 4 + key.size());
    char *hdata = hashindex_.data() + hashindex_.size();
    *reinterpret_cast<uint64_t *>(hdata) = sequence;
    *reinterpret_cast<uint32_t *>(hdata + 8) = file_size_;
    *reinterpret_cast<uint32_t *>(hdata + 12) = file_buf_.size();
    *reinterpret_cast<uint32_t *>(hdata + 16) = key.size();
    key.copy(hdata + 20, key.size());

    file_size_ += file_buf_.size();
    if(file_size_ >= LOG_FILE_SIZE_THRESHOLD) {
        dump();
    }
    return 0;
}

void LogBuilder::dump() {
    assert(activelist_.next != &activelist_);

    uint64_t fid = activelist_.next->file_id;
    WriteFile *hwf = Env::globalEnv()->newWriteFile(std::to_string(fid) + ".hindex", true);
    assert(hashindex_.size() >= 8);
    *reinterpret_cast<uint32_t *>(hashindex_.data() + 4) = hashindex_.size() - 8;
    *reinterpret_cast<uint32_t *>(hashindex_.data() + 4) = Mask(Value(hashindex_.data() + 4, hashindex_.size() - 4));
    hwf->write(hashindex_);

    delete wf_;
    fileNode *fnode = new fileNode;
    fnode->file_id = newFileID();
    fnode->next = activelist_.next;
    fnode->prev = &activelist_;
    activelist_.next->prev = fnode;
    activelist_.next = fnode;
    wf_ = Env::globalEnv()->newWriteFile(std::to_string(fid) + ".log", true);
    hashindex_.resize(8);
}