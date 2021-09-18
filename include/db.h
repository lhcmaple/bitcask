#ifndef _DB_H
#define _DB_H

#include <string>

#include "iter.h"

using std::string_view;
using std::string;

class DB {
public:
    // 打开db_name, 失败返回nullptr
    static DB *open(const string_view &db_name);
    // 插入key-value条目
    virtual int put(const string_view &key, const string_view &value) = 0;
    // 查询key, 如果存在, 返回0, 否则返回-1, 如果data == nullptr, 只返回值, 否则将查询到的数据填入data
    virtual int get(const string_view &key, string *value) = 0;
    // 删除key, 如果存在, 返回0, 否则返回-1
    virtual int del(const string_view &key) = 0;
    // 进行数据库压缩, background表明进行前台压缩还是后台压缩, 默认为后台压缩(会导致数据库性能下降, 或许可以选择后台压缩的速率?)
    virtual int compact(bool background = true) = 0;
    // 迭代器
    virtual Iter *newIter() = 0;
    virtual bool isValid() = 0;
    virtual ~DB() {}
};

#endif