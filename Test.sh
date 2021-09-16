#!/bin/bash

[ -d .Test ] || mkdir .Test

CPPFLAG="-I./util/ -std=c++17"

# arena_test
echo "-----test arena-----"
g++ $CPPFLAG -o .Test/arena_test util/arena_test.cc util/arena.cc
valgrind ./.Test/arena_test
echo "-----arena tested-----"

echo

# hash_test
echo "-----test hash-----"
g++ $CPPFLAG -o .Test/hash_test util/hash_test.cc util/hash.cc util/hash.h
./.Test/hash_test
echo "-----hash tested-----"

echo

# hashtable_test
echo "-----test hashtable-----"

echo "-----hashtable tested-----"