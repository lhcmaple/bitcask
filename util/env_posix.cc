#include "env.h"

class posixWriteFile;
class posixRandomReadFile;

class posixEnv : public Env {
public:
    WriteFile *newWriteFile(const string_view &fname, bool iscreat = false) override;
    RandomReadFile *newRandomReadFile(const string_view &fname) override;
    int newThread(FunctionHandle func, void *arg) override;
    void sleep(int microseconds) override;
};

int posixEnv::newThread(FunctionHandle func, void *arg) {
    //
    return 0;
}

void posixEnv::sleep(int microseconds) {
    
}

class posixWriteFile : public WriteFile{
private:
    int fd;
public:
    posixWriteFile(const string_view &fname, bool iscreat);
    ssize_t write(const string_view &data) override;
    int flush() override;
    ~posixWriteFile() override;

    static posixWriteFile *newposixWriteFile(const string_view &fname, bool iscreat) {
        posixWriteFile *pwf = new posixWriteFile(fname, iscreat);
        if(pwf->fd < 0) {
            delete pwf;
            return nullptr;
        }
        return pwf;
    }
};

posixWriteFile::posixWriteFile(const string_view &fname, bool iscreat) {
    int O_FLAG = O_WRONLY | O_APPEND;
    if(iscreat) {
        O_FLAG |= O_CREAT;
    }
    fd = open(fname.data(), O_FLAG, S_IRWXU);
}

posixWriteFile::~posixWriteFile() {
    close(fd);
}

ssize_t posixWriteFile::write(const string_view &data) {
    return ::write(fd, data.data(), data.size());
}

int posixWriteFile::flush() {
    return fsync(fd);
}

class posixRandomReadFile : public RandomReadFile {
private:
    int fd;
public:
    posixRandomReadFile(const string_view &fname);
    ssize_t read(size_t size, string *data, int offset) override;
    ssize_t read(size_t size, string *data) override;
    ~posixRandomReadFile() override;

    static posixRandomReadFile *newposixRandomReadFile(const string_view &fname) {
        posixRandomReadFile *prf = new posixRandomReadFile(fname);
        if(prf->fd < 0) {
            delete prf;
            return nullptr;
        }
        return prf;
    }
};

posixRandomReadFile::posixRandomReadFile(const string_view &fname) {
    fd = open(fname.data(), O_RDONLY);
}

posixRandomReadFile::~posixRandomReadFile() {
    close(fd);
}

ssize_t posixRandomReadFile::read(size_t size, string *data, int offset) {
    lseek(fd, offset, SEEK_SET);
    if(size == 0) {
        return 0;
    }
    return read(size, data);
}

ssize_t posixRandomReadFile::read(size_t size, string *data) {
    assert(data != nullptr);
    data->clear();
    data->resize(size);
    ssize_t nread = ::read(fd, data->data(), size);
    if(nread < 0) {
        data->clear();
        return nread;
    }
    data->resize(nread);
    return nread;
}

inline WriteFile *posixEnv::newWriteFile(const string_view &fname, bool iscreat) {
    return posixWriteFile::newposixWriteFile(fname, iscreat);
}
inline RandomReadFile *posixEnv::newRandomReadFile(const string_view &fname) {
    return posixRandomReadFile::newposixRandomReadFile(fname);
}

inline Env *Env::globalEnv() {
    static posixEnv env;
    return &env;
}
