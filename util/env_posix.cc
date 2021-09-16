#include "env.h"

class posixWriteFile;
class posixRandomReadFile;

class posixEnv : public Env {
public:
    inline WriteFile *newWriteFile(const string_view &fname, bool iscreat = false) override {
        return posixWriteFile::newposixWriteFile(fname, iscreat);
    }
    inline RandomReadFile *newRandomReadFile(const string_view &fname) override {
        return posixRandomReadFile::newposixRandomReadFile(fname);
    }
    virtual int newThread(FunctionHandle func, void *arg) override;
};

int posixEnv::newThread(FunctionHandle func, void *arg) {
    //
    return 0;
}

class posixWriteFile : public WriteFile{
private:
    int fd;
public:
    posixWriteFile(const string_view &fname, bool iscreat);
    int write(const string_view &data) override;
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
    int O_FLAG = O_WRONLY;
    if(iscreat) {
        O_FLAG |= O_CREAT;
    }
    fd = open(fname.data(), O_FLAG, S_IRWXU);
}

posixWriteFile::~posixWriteFile() {
    close(fd);
}

int posixWriteFile::write(const string_view &data) {
    //
}

int posixWriteFile::flush() {
    //
}

class posixRandomReadFile : public RandomReadFile {
private:
    int fd;
public:
    posixRandomReadFile(const string_view &fname);
    int read(size_t size, string *data, int offset) override;
    int read(size_t size, string *data) override;
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

int posixRandomReadFile::read(size_t size, string *data, int offset) {
    //
}

int posixRandomReadFile::read(size_t size, string *data, int offset) {
    //
}

Env *Env::newEnv() {
    return new posixEnv;
}
