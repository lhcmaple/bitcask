#include <cstring>
#include <string>
#include <iostream>
#include <cassert>

#include "env.h"

using std::string;
using std::cout;

int main(int argc, char *argv[]) {
    Env *env = Env::newEnv();
    cout<<"[test WriteFile and RandomReadFile]\n";
    WriteFile *wf = env->newWriteFile(".Test/env_file", true);
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
    rf->read(0, nullptr, 0);
    rf->read(strlen(data[0]), &buf);
    cout<<buf;
    rf->read(0, nullptr, strlen(data[0]));
    rf->read(strlen(data[1]), &buf);
    cout<<buf;
    rf->read(0, nullptr, strlen(data[0]) + strlen(data[1]));
    rf->read(strlen(data[2]), &buf);
    cout<<buf;
    cout<<"[test WriteFile and RandomReadFile]\n";
    delete rf;
    delete wf;
    delete env;
    return 0;
}