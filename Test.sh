#!/bin/bash

[ -d .Test ] || mkdir .Test

CPPFLAG="-I./util/ -std=c++17"

# arena_test
g++ $CPPFLAG -o .Test/arena_test util/arena_test.cc util/arena.cc
valgrind ./.Test/arena_test

#