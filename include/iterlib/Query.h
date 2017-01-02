//  Copyright (c) 2016, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Code to convert a Query object to iterator tree
#pragma once

#include <iterlib/Item.h>
#include <iterlib/Iterators.h>

namespace iterlib {

typedef std::unique_ptr<Iterator> IteratorPtr;

enum class QueryType : int64_t {
  LIMIT,
  REVERSE,
  RANDOM,
  COUNT,
  AND,
  OR,
  CONCAT,
  SORTED_MERGE,
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

struct IteratorBuilder;
template <QueryType queryType, class IteratorType>
IteratorPtr makeCompositeIt(const IteratorBuilder& b, const dynamic& args);

template <QueryType queryType, class IteratorType, typename... Args>
IteratorPtr makeWrappedIt(const IteratorBuilder& b, const dynamic& args);

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

  IteratorPtr operator() (const variant::ordered_map_t& m) const;
};

template <QueryType queryType, class IteratorType>
IteratorPtr makeCompositeIt(const IteratorBuilder& b, const dynamic& args) {
  if (!args.is_of<variant::vector_dynamic_t>()) {
    throw std::logic_error("Expected a vector");
  }
  IteratorVector vec;
  for (const auto& c : args.getRef<variant::vector_dynamic_t>()) {
    const Query& cq = c;
    vec.emplace_back(std::move(boost::apply_visitor(b, cq)));
  }
  return folly::make_unique<IteratorType>(vec);
}

template <QueryType queryType, class IteratorType, typename... Args>
IteratorPtr makeWrappedIt(const IteratorBuilder& b, const dynamic& d,
                          Args... args) {
  auto inner = std::move(boost::apply_visitor(b, d));
  return folly::make_unique<IteratorType>(inner.release(), args...);
}

// TODO: Retire makeWrappedItSingle in favor of makeWrappedIt
template <QueryType queryType, class IteratorType>
IteratorPtr makeWrappedItSingle(const IteratorBuilder& b, const dynamic& d) {
  auto inner = std::move(boost::apply_visitor(b, d));
  return folly::make_unique<IteratorType>(inner.release());
}

IteratorPtr IteratorBuilder::operator() (const variant::ordered_map_t& m) const {
  // We need to have exactly one key at the root of the tree. So this for
  // loop is mostly unnecesary. Keeping it around for completeness.
  for (const auto& kvp : m) {
    switch(QueryType(int64_t(kvp.first))) {
    // Composite Iterators
    case QueryType::AND:
      return makeCompositeIt<QueryType::AND, AndIterator>(*this, kvp.second);
    case QueryType::OR:
      return makeCompositeIt<QueryType::OR, UnionIterator>(*this, kvp.second);
    case QueryType::CONCAT:
      return makeCompositeIt<QueryType::CONCAT, ConcatIterator>(*this, kvp.second);
    case QueryType::SORTED_MERGE:
      return makeCompositeIt<QueryType::SORTED_MERGE, SortedMergeIterator>(*this, kvp.second);
    case QueryType::DIFFERENCE:
      return makeCompositeIt<QueryType::DIFFERENCE, DifferenceIterator>(*this, kvp.second);
    case QueryType::MERGE:
      return makeCompositeIt<QueryType::MERGE, MergeIterator>(*this, kvp.second);
    // Wrapped Iterators
    case QueryType::LIMIT:
      return makeWrappedIt<QueryType::LIMIT, LimitIterator>(*this, kvp.second, 0, 0);
    case QueryType::REVERSE:
      return makeWrappedItSingle<QueryType::REVERSE, ReverseIterator>(*this, kvp.second);
    case QueryType::RANDOM:
      return makeWrappedIt<QueryType::RANDOM, RandomIterator>(*this, kvp.second, 0);
    case QueryType::COUNT:
      return makeWrappedItSingle<QueryType::COUNT, CountIterator>(*this, kvp.second);
    case QueryType::LET:
      return makeWrappedIt<QueryType::LET, LetIterator>(*this, kvp.second, "", "");
    case QueryType::NEST:
      return makeWrappedIt<QueryType::NEST, NestIterator>(*this, kvp.second, "");
    case QueryType::PROJECT:
      return makeWrappedIt<QueryType::PROJECT, ProjectIterator>(*this, kvp.second, AttributeNameVec{});
    case QueryType::EXISTS:
      // Not implemented
      break;
    case QueryType::FILTER:
      // it->setFilter(xxx);
      return makeWrappedItSingle<QueryType::FILTER, FilterIterator>(*this, kvp.second);
    case QueryType::GROUPBY:
      return makeWrappedIt<QueryType::GROUPBY, GroupByIterator>(*this, kvp.second, AttributeNameVec{});
    case QueryType::ORDERBY:
      return makeWrappedIt<QueryType::ORDERBY, OrderByIterator>(*this, kvp.second, AttributeNameVec{});;
    }
  }
  return folly::make_unique<FutureIterator<ItemOptimized>>(
    folly::makeFuture(std::vector<ItemOptimized>{}));
}

inline IteratorPtr toIteratorTree(const Query& v) {
  return boost::apply_visitor(IteratorBuilder(), v);
}

}
