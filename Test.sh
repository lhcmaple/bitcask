#!/bin/bash

[ -d .Test ] || mkdir .Test
rm .Test/db/*

CPPFLAG="-g -I./util/ -I./include/ -I./log -std=c++17 -lpthread"

# arena_test
# echo "-----test arena-----"
# g++ $CPPFLAG -o .Test/arena_test util/arena_test.cc util/arena.cc
# valgrind ./.Test/arena_test
# echo "-----arena tested-----"

# hash_test
# echo "-----test hash-----"
# g++ $CPPFLAG -o .Test/hash_test util/hash_test.cc util/hash.cc util/hashtable.cc util/hash.h
# ./.Test/hash_test
# echo "-----hash tested-----"

# # hashtable_test
# echo "-----test hashtable-----"
# g++ $CPPFLAG -o .Test/hashtable_test util/hashtable_test.cc util/hashtable.cc util/hash.cc util/arena.cc
# ./.Test/hashtable_test
# echo "-----hashtable tested-----"

# env_test
# echo "-----test env-----"
# g++ -o .Test/env_test util/env_test.cc util/env_posix.cc $CPPFLAG
# ./.Test/env_test
# echo "-----env tested-----"

# crc32c_test
# echo "-----test crc32c-----"
# g++ $CPPFLAG -o .Test/crc32c_test util/crc32c_test.cc util/crc32c.cc
# ./.Test/crc32c_test
# echo "-----crc32c tested-----"

# # dbimpl_test
# echo "-----test dbimpl-----"
# allfiles="db/dbimpl_test.cc db/dbimpl.cc \
# log/logbuilder.cc log/logreader.cc util/arena.cc \
# util/crc32c.cc util/env_posix.cc util/hash.cc util/hashtable.cc util/cache.cc"
# g++ -o .Test/dbimpl_test $allfiles $CPPFLAG
# echo "-----dbimpl tested-----"

# # logbuilder_test
# echo "-----test logbuilder-----"
# allfiles="log/logbuilder.cc log/logbuilder_test.cc log/logreader.cc \
# util/env_posix.cc util/hashtable.cc util/crc32c.cc util/arena.cc util/hash.cc"
# g++ -o .Test/logbuilder_test $allfiles $CPPFLAG
# .Test/logbuilder_test .Test/db
# echo "-----logbuilder tested-----"

# # logreader_test
# echo "-----test logreader-----"
# allfiles="log/logreader_test.cc log/logreader.cc \
# util/env_posix.cc util/hashtable.cc util/crc32c.cc util/arena.cc util/hash.cc"
# g++ -o .Test/logreader_test $allfiles $CPPFLAG
# echo "-----logreader tested-----"

# db_bench
echo "-----benchmark-----"
allfiles="db/dbimpl.cc benchmark/db_bench.cc \
log/logbuilder.cc log/logreader.cc util/arena.cc \
util/crc32c.cc util/env_posix.cc util/hash.cc util/hashtable.cc util/cache.cc"
g++ -o .Test/db_bench $allfiles $CPPFLAG
echo "-----benchmark-----"