// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef HASH_H
#define HASH_H

#include <cstddef>
#include <cstdint>

uint32_t Hash(const char* data, size_t n, uint32_t seed = 0);

#endif