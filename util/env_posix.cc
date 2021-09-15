#include "env.h"

class posixWriteFile;
class posixRandomReadFile;

class posixEnv : public Env {
public:
    inline WriteFile *newWriteFile() override {
        return new posixWriteFile();
    }
    inline RandomReadFile *newRandomReadFile() override {
        return new posixRandomReadFile();
    }
};

class posixWriteFile : public WriteFile{
public:
    int open() override;
    int write(const string_view &data) override;
    int flush() override;
    int close() override;
    ~posixWriteFile() override;
};

class posixRandomReadFile : public RandomReadFile {
public:
    int open() override;
    int read(size_t size, string *data, int offset) override;
    int read(size_t size, string *data) override;
    int close() override;
    ~posixRandomReadFile() override;
};

Env *newEnv() {
    return new posixEnv();
}