#include <cstdio>
#include <cstring>

#include "hash.h"

int main(int argc, char *argv[]) {
    const char *data1 = "HELLO, WORLD";
    const char *data2 = "HELLO  WORLD";
    const char *data3 = "KJAOIUDSKFJOIUWEOIRJEWKLJFDSOIUWKJLAH";
    const char *data4 = "198776*&*(^^%&(dsaklue";
    printf("%x\n", Hash(data1, strlen(data1)));
    printf("%x\n", Hash(data2, strlen(data1)));
    printf("%x\n", Hash(data3, strlen(data1)));
    printf("%x\n", Hash(data4, strlen(data1)));
    return 0;
}
