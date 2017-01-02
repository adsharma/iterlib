//  Copyright (c) 2016, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Code to convert a Query object to iterator tree
#pragma once

#include <iterlib/Item.h>
#include <iterlib/Iterator.h>
#include <iterlib/FutureIterator.h>
#include <iterlib/AndIterator.h>

namespace iterlib {

typedef std::unique_ptr<Iterator> IteratorPtr;

enum class QueryType : int64_t {
  LIMIT,
  REVERSE,
  RANDOM,
  COUNT,
  AND,
  OR,
  DIFFERENCE,
  LET,
  MERGE,
  NEST,
  PROJECT,
  EXISTS,
  FILTER,
  GROUPBY,
  ORDERBY,
};

class Query : public dynamic {
 public:
  using dynamic::operator=;
  using dynamic::is_of;
  using dynamic::get;
  using dynamic::getRef;
  using dynamic::getNonConstRef;

  static const Query kEmptyQuery;

  Query() : dynamic() {}
  Query(const dynamic& other) : dynamic(other) {}
  virtual ~Query() {}

  Query(const Query& other) = default;
  Query(Query&& other) = default;
  Query& operator=(const Query& other) = default;
  Query& operator=(Query&& other) = default;
};

// vec is assumed to be in descending order
static inline std::unique_ptr<Iterator> getIterator(
  const std::vector<int64_t>& vec) {
  std::vector<ItemOptimized> res;
  res.reserve(vec.size());
  for (const auto& v : vec) {
    res.emplace_back(ItemOptimized{(id_t) v, 0, ""});
  }
  return folly::make_unique<FutureIterator<ItemOptimized>>(
    folly::makeFuture(res));
}

struct IteratorBuilder : boost::static_visitor<IteratorPtr> {

  IteratorPtr operator() (const boost::blank v) const {
    return folly::make_unique<FutureIterator<ItemOptimized>>(
      folly::makeFuture(std::vector<ItemOptimized>{}));
  }

  template <typename T>
  IteratorPtr operator() (const T v) const {
    return folly::make_unique<FutureIterator<ItemOptimized>>(
      folly::makeFuture(std::vector<ItemOptimized>{}));
  }

  IteratorPtr operator() (const variant::vector_dynamic_t& vec) const {
    return folly::make_unique<FutureIterator<Item>>(folly::makeFuture(
      *reinterpret_cast<const std::vector<Item> *>(&vec)));
  }

  IteratorPtr operator() (const std::vector<int64_t>& vec) const {
    return getIterator(vec);
  }

  IteratorPtr operator() (const variant::ordered_map_t& m) const {
    for (const auto& kvp : m) {
      if (kvp.first == int64_t(QueryType::AND)) {
        if (!kvp.second.is_of<variant::vector_dynamic_t>()) {
          throw std::logic_error("Expected a vector");
        }
        IteratorVector vec;
        for (const auto& c : kvp.second.getRef<variant::vector_dynamic_t>()) {
          const Query& cq = c;
          vec.emplace_back(std::move(boost::apply_visitor(*this, cq)));
        }
        return folly::make_unique<AndIterator>(vec);
      }
    }
    return folly::make_unique<FutureIterator<ItemOptimized>>(
      folly::makeFuture(std::vector<ItemOptimized>{}));
  }
};

inline IteratorPtr toIteratorTree(const Query& v) {
  return boost::apply_visitor(IteratorBuilder(), v);
}

}
