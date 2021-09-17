#include "crc32c.h"

#include <cstring>
#include <cstdio>

int main(int argc, char *argv[]) {
    const char *data1 = "hello  world!";
    const char *data2 = "hello, world!";
    const char *data12 = "hello  world!hello, world!";
    uint32_t crc1 = Value(data1, strlen(data1));
    uint32_t crc2 = Value(data2, strlen(data2));
    uint32_t crc12 = Value(data12, strlen(data12));
    uint32_t crc1_2 = Extend(crc1, data2, strlen(data2));
    uint32_t crc1_mask = Mask(crc1);
    const char *logic[2] = {"false", "true"};
    printf("correctness: %s, %s, %s\n", 
        logic[crc1 != crc2], logic[crc12 == crc1_2], logic[Unmask(crc1_mask) == crc1]);
    return 0;
}