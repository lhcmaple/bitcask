#include "logreader.h"
#include "iter.h"
#include "crc32c.h"

#include <string>
#include <cassert>

using std::string;

class LogReader::Iterator : public Iter {
private:
    RandomReadFile *rf_;
    ssize_t off_cur_;
    ssize_t off_next_;
    string header_cur_;
    string data_cur_;
    LogContent lc_;

public:
    Iterator(RandomReadFile *rf) : rf_(rf) {
        off_cur_ = -1;
    }

    bool isValid() override {
        return off_cur_ >= 0;
    }

    void seekToFirst() override {
        off_cur_ = 0;
        off_next_ = 0;
        next();
    }

    void next() override {
        assert(isValid());
        off_cur_ = off_next_;
        rf_->read(20, &header_cur_, off_cur_);
        if(header_cur_.size() < 20) {
            off_cur_ = -1;
            return;
        }
        uint32_t crc = *reinterpret_cast<uint32_t *>(header_cur_.data());
        uint64_t sequence = *reinterpret_cast<uint32_t *>(header_cur_.data() + 4);
        size_t key_length = *reinterpret_cast<uint32_t *>(header_cur_.data() + 12);
        size_t value_length = *reinterpret_cast<uint32_t *>(header_cur_.data() + 16);
        rf_->read(key_length + value_length + 20, &data_cur_, off_cur_);
        off_next_ = off_cur_ + key_length + value_length + 20;
        if(data_cur_.size() < key_length + value_length + 20) {
            off_cur_ = -1;
            return;
        }
        if(Unmask(crc) != Value(data_cur_.data() + 4, key_length + value_length + 16)) {
            off_cur_ = -1;
            return;
        }
        lc_.sequence = sequence;
        lc_.crc = crc;
        lc_.key.assign(data_cur_.data() + 20, key_length);
        lc_.value.assign(data_cur_.data() + 20 + key_length, value_length);
    }

    void *get() override {
        assert(isValid());
        return &lc_;
    }

    ~Iterator() override {

    }
};

Iter *LogReader::newIter() {
    return new Iterator(rf_);
}

LogReader *LogReader::newLogReader(uint64_t file_id, const string_view &db_name) {
    LogReader *lr = new LogReader(file_id, db_name);
    if(lr->rf_ == nullptr) {
        delete lr;
        return nullptr;
    }
    return lr;
}

LogContent *LogReader::seek(const Handle &handle) {
    assert(handle.file_id == file_id_);
    rf_->read(handle.size, &data_, handle.offset);
    if(data_.size() < handle.size) {
        data_.clear();
        return nullptr;
    }
    uint32_t crc = Value(data_.data() + 4, data_.size() - 4);
    if(Unmask(*reinterpret_cast<uint32_t *>(data_.data())) != crc) {
        data_.clear();
        return nullptr;
    }
    uint32_t key_length = *reinterpret_cast<uint32_t *>(data_.data() + 12);
    uint32_t value_length = *reinterpret_cast<uint32_t *>(data_.data() + 16);
    LogContent *lc = new LogContent;
    lc->sequence = *reinterpret_cast<uint64_t *>(data_.data() + 4);
    lc->crc = crc;
    lc->key.assign(data_.data() + 20, key_length);
    lc->value.assign(data_.data() + 20 + key_length, value_length);    
    return lc;
}

HIndexReader *HIndexReader::newHIndexReader(uint64_t file_id, const string_view &db_name) {
    HIndexReader *hir = new HIndexReader(file_id, db_name);
    if(hir->rf_ == nullptr) {
        delete hir;
        hir = nullptr;
    }
    return hir;
}

void HIndexReader::examine() {
    rf_->read(4, &data, 4);
    if(data.size() < 4) {
        delete rf_;
        rf_ = nullptr;
        Env::globalEnv()->rmFile(db_name_ + "/" + std::to_string(file_id_) + ".hindex");
        return;
    }
    size_t totalsize = *reinterpret_cast<uint32_t *>(data.data());
    rf_->read(totalsize, &data, 0);
    if(data.size() < totalsize) {
        delete rf_;
        rf_ = nullptr;
        Env::globalEnv()->rmFile(db_name_ + "/" + std::to_string(file_id_) + ".hindex");
        return;
    }
    if(Unmask(*reinterpret_cast<uint32_t *>(data.data())) != 
        Value(data.data() + 4, data.size() - 4)) {
        delete rf_;
        rf_ = nullptr;
        Env::globalEnv()->rmFile(db_name_ + "/" + std::to_string(file_id_) + ".hindex");
        return;
    }
}

class HIndexReader::Iterator : public Iter {
private:
    string_view data_;
    ssize_t cur_off_;
    ssize_t next_off_;
    HIndexContent hic_;

public:
    Iterator(const string_view &data) : data_(data) {
        cur_off_ = -1;
    }

    bool isValid() override {
        return cur_off_ >= 0;
    }

    void seekToFirst() override {
        cur_off_ = 8;
        next_off_ = 8;
        next();
    }

    void next() override {
        assert(isValid());
        cur_off_ = next_off_;
        if(cur_off_ >= data_.size()) {
            cur_off_ = -1;
            return;
        }
        size_t key_length = *reinterpret_cast<const uint32_t *>(data_.data() + cur_off_ + 16);
        hic_.key.assign(data_.data() + cur_off_ + 20, key_length);
        hic_.offset = *reinterpret_cast<const uint32_t *>(data_.data() + cur_off_ + 8);
        hic_.size = *reinterpret_cast<const uint32_t *>(data_.data() + cur_off_ + 12);
        hic_.sequence = *reinterpret_cast<const uint64_t *>(data_.data() + cur_off_);
        next_off_ = cur_off_ + 8 + 4 + 4 + 4 + key_length;
    }

    void *get() {
        assert(isValid());
        return &hic_;
    }

    ~Iterator() {
        
    }
};

Iter *HIndexReader::newIter() {
    return new HIndexReader::Iterator(data);
}