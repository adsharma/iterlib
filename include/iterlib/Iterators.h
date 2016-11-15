//  Copyright (c) 2016, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
#pragma once

/**
 * Exports all Iterators via one header.
 * Please try to include only iterators you need.
 */

#include "iterlib/Iterator.h"

// Leaf Iterators
#include "iterlib/RocksDBIterator.h"

// Most common Iterators
#include "iterlib/FilterIterator.h"
#include "iterlib/LimitIterator.h"
#include "iterlib/OrderbyIterator.h"
#include "iterlib/ProjectIterator.h"
#include "iterlib/RandomIterator.h"
#include "iterlib/ReverseIterator.h"

#include "iterlib/AdvancedIterators.h"
