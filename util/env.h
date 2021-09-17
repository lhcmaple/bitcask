#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

using std::string_view;
using std::string;

class WriteFile;
class RandomReadFile;

typedef void (*FunctionHandle)(void *);

class Env {
public:
    virtual WriteFile *newWriteFile(const string_view &fname, bool iscreat = false) = 0;
    virtual RandomReadFile *newRandomReadFile(const string_view &fname) = 0;
    virtual int newThread(FunctionHandle func, void *arg) = 0;
    virtual void sleep(int microseconds) = 0;
    static Env *globalEnv();
};

class WriteFile {
public:
    virtual ssize_t write(const string_view &data) = 0;
    virtual int flush() = 0;
    virtual ~WriteFile() {}
};

class RandomReadFile {
public:
    // size = 0, data = *, offset = *, 
    // just seek to the offset of the file without doing anything
    virtual ssize_t read(size_t size, string *data, int offset) = 0;
    virtual ssize_t read(size_t size, string *data) = 0;
    virtual ~RandomReadFile() {}
};

#endif
