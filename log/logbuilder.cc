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
    return write(file_buf_);
}