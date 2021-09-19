#include "logreader.h"
#include "iter.h"
#include "crc32c.h"

#include <string>
#include <cassert>

using std::string;

class LogReader::Iterator : public Iter {
private:
    RandomReadFile *rf_;
    size_t off_cur_;
    size_t off_next_;
    string header_cur_;
    string data_cur_;
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
        off_cur_ = off_next_;
        size_t nread = rf_->read(20, &header_cur_, off_cur_);
        if(nread < 20) {
            off_cur_ = -1;
            return;
        }
        uint32_t crc = *reinterpret_cast<uint32_t *>(header_cur_.data());
        uint64_t sequence = *reinterpret_cast<uint32_t *>(header_cur_.data() + 4);
        size_t key_length = *reinterpret_cast<uint32_t *>(header_cur_.data() + 12);
        size_t value_length = *reinterpret_cast<uint32_t *>(header_cur_.data() + 16);
        nread = rf_->read(key_length + value_length + 20, &data_cur_, off_cur_);
        off_next_ = off_cur_ + key_length + value_length + 20;
        if(nread < key_length + value_length + 20) {
            off_cur_ = -1;
            return;
        }
        if(Unmask(crc) != Value(data_cur_.data() + 4, key_length + value_length + 16)) {
            off_cur_ = -1;
            return;
        }
    }
    void *get() override {
        return data_cur_.data();
    }
    ~Iterator() override {

    }
};

Iter *LogReader::newIter() {
    return new Iterator(rf_);
}

LogReader *LogReader::newLogReader(uint64_t file_id) {
    LogReader *lr = new LogReader(file_id);
    if(lr->rf_ == nullptr) {
        delete lr;
        return nullptr;
    }
    return lr;
}

int LogReader::seek(const Handle &handle, string *data) {
    assert(handle.file_id == file_id_);
    rf_->read(handle.size, data, handle.offset);
    if(data->size() < handle.size) {
        data->clear();
        return -1;
    }
    assert(handle.sequence == 
        *reinterpret_cast<uint64_t *>(data->data() + 4));
    if(Unmask(*reinterpret_cast<uint32_t *>(data->data()))
        != Value(data->data() + 4, data->size() - 4)) {
        data->clear();
        return -1;
    }
    return 0;
}

HIndexReader *HIndexReader::newHIndexReader(uint64_t file_id) {
    HIndexReader *hir = new HIndexReader(file_id);
    if(hir->rf_ == nullptr) {
        delete hir;
        hir = nullptr;
    }
    return hir;
}

RandomReadFile *HIndexReader::examine() {
    rf_->read(4, &data, 4);
    size_t totalsize = *reinterpret_cast<uint32_t *>(data.data());
    
}