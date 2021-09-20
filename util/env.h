#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <cstddef>
#include <cassert>
#include <vector>
#include <utility>
#include <cstdio>

using std::string_view;
using std::string;
using std::vector;

class WriteFile;
class RandomReadFile;
class Mutex;

typedef void *(*FunctionHandle)(void *);

class Env {
public:
    virtual WriteFile *newWriteFile(const string_view &fname) = 0;
    virtual RandomReadFile *newRandomReadFile(const string_view &fname) = 0;
    virtual Mutex *newMutex() = 0;
    virtual int newThread(FunctionHandle func, void *arg, uint64_t *id) = 0;
    virtual int joinThread(uint64_t id) = 0;
    virtual int readDir(const string_view &dir_name, vector<string> *files) = 0;
    int rmFile(const string_view &fname) {
        return remove(fname.data());
    }

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
    virtual ssize_t read(size_t size, string *data, int offset) = 0;
    virtual ~RandomReadFile() {}
};

class Mutex {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual ~Mutex() {}
};

class Lock {
private:
    Mutex *mutex_;
public:
    Lock(Mutex *mutex) : mutex_(mutex) {
        mutex_->lock();
    }
    ~Lock() {
        mutex_->unlock();
    }
};

#define GUARD_BEGIN(mutex) { \
    Lock __lock(mutex);
#define GUARD_END }

#endif