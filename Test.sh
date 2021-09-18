#!/bin/bash

[ -d .Test ] || mkdir .Test

CPPFLAG="-I./util/ -I./include/ -std=c++17"

# arena_test
# echo "-----test arena-----"
# g++ $CPPFLAG -o .Test/arena_test util/arena_test.cc util/arena.cc
# valgrind ./.Test/arena_test
# echo "-----arena tested-----"

# echo

# hash_test
# echo "-----test hash-----"
# g++ $CPPFLAG -o .Test/hash_test util/hash_test.cc util/hash.cc util/hashtable.cc util/hash.h
# ./.Test/hash_test
# echo "-----hash tested-----"

# echo

# hashtable_test
echo "-----test hashtable-----"
g++ $CPPFLAG -o .Test/hashtable_test util/hashtable_test.cc util/hashtable.cc util/hash.cc util/arena.cc
./.Test/hashtable_test
echo "-----hashtable tested-----"

# env_test
# echo "-----test env-----"
# g++ $CPPFLAG -o .Test/env_test util/env_test.cc util/env_posix.cc
# ./.Test/env_test
# rm .Test/env_file
# echo "-----env tested-----"

# crc32c_test
# echo "-----test crc32c-----"
# g++ $CPPFLAG -o .Test/crc32c_test util/crc32c_test.cc util/crc32c.cc
# ./.Test/crc32c_test
# echo "-----crc32c tested-----"