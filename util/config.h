#ifndef CONFIG_H
#define CONFIG_H

// 小内存分配器块内存大小
#define ARENA_BLOCK_SIZE 32768

// 文件大小阈值
#define LOG_FILE_SIZE_THRESHOLD 33554432

// 冗余度阈值
#define REDUNDANCE_THRESHOLD 0.7

// 文件最小大小
#define MIN_FILE_SIZE (LOG_FILE_SIZE_THRESHOLD / 64)

#endif