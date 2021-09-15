#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <cstddef>

using std::string_view;
using std::string;

class WriteFile;
class RandomReadFile;

class Env {
public:
    virtual WriteFile *newWriteFile() = 0;
    virtual RandomReadFile *newRandomReadFile() = 0;
};

class WriteFile {
public:
    virtual int open() = 0;
    virtual int write(const string_view &data) = 0;
    virtual int flush() = 0;
    virtual int close() = 0;
    virtual ~WriteFile() = 0;
};

class RandomReadFile {
public:
    virtual int open() = 0;
    virtual int read(size_t size, string *data, int offset) = 0;
    virtual int read(size_t size, string *data) = 0;
    virtual int close() = 0;
    virtual ~RandomReadFile() = 0;
};

Env *newEnv();

#endif