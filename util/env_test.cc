#include <cstring>
#include <string>
#include <iostream>
#include <cassert>

#include "env.h"

using std::string;
using std::cout;

static int count = 0;
static int total = 100000;
static int nexit = 0;
void *task(void *arg) {
    Mutex *mutex = static_cast<Mutex *>(arg);
    for(int i = 0; i < total / 2; i++) {
        GUARD_BEGIN(mutex)
            count++;
        GUARD_END
    }
    GUARD_BEGIN(mutex)
        nexit++;
    GUARD_END
    return 0;
}

const char *logic[2] = {
    "false",
    "true"
};

int main(int argc, char *argv[]) {
    Env *env = Env::globalEnv();
    cout<<"[test WriteFile and RandomReadFile]\n";
    WriteFile *wf = env->newWriteFile(".Test/env_file");
    assert(wf != nullptr);
    const char *data[] = {
        "HELLOWORLD\n",
        "ENV_TEST\n",
        "bitcask\n"
    };
    wf->write(data[0]);
    wf->write(data[1]);
    wf->write(data[2]);
    string buf;
    RandomReadFile *rf = env->newRandomReadFile(".Test/env_file");
    assert(rf != nullptr);
    rf->read(strlen(data[0]), &buf, 0);
    cout<<buf;
    rf->read(strlen(data[1]), &buf, strlen(data[0]));
    cout<<buf;
    rf->read(strlen(data[2]), &buf, strlen(data[0]) + strlen(data[1]));
    cout<<buf;
    delete rf;
    delete wf;
    cout<<"[test WriteFile and RandomReadFile]\n\n";

    cout<<"[test Thread, Mutex and Lock]\n";
    Mutex *mutex = Env::globalEnv()->newMutex();
    Env::globalEnv()->newThread(task, mutex);
    Env::globalEnv()->newThread(task, mutex);
    while(nexit < 2);
    printf("Correctness test: %s\n", logic[count == total]);
    delete mutex;
    cout<<"[test Thread, Mutex and Lock]\n\n";

    cout<<"[test ReadDir and rmFile]\n";
    vector<string> files;
    Env::globalEnv()->readDir(".Test", &files);
    for(auto &f : files) {
        cout<<f<<" ";
    }
    cout<<std::endl;
    Env::globalEnv()->rmFile(".Test/env_file");
    cout<<">>>>delete env_file\n";
    Env::globalEnv()->readDir(".Test", &files);
    for(auto &f : files) {
        cout<<f<<" ";
    }
    cout<<std::endl;
    cout<<"[test ReadDir and rmFile]\n";
    return 0;
}