### 日志文件格式
    [file_id].log
### 哈希索引备份文件
    [file_id].hindex
### 字段大小
    uint64_t for sequence
    uint32_t for offset
    uint32_t for size
    uint32_t for key length
    uint32_t for value length
    uint32_t for crc
    uint32_t for total length(mean the whole size of the file)
### 