// Copyright 2015 - present, Facebook

#pragma once

#include "iterlib/OrIterator.h"

namespace iterlib {

class MergeIterator : public UnionIterator {
 public:
  explicit MergeIterator(IteratorVector& children)
      : UnionIterator(children) {}

  virtual const Item& value() const override {
    return value_;
  }

 protected:
  bool doNext() override {
    auto ret = UnionIterator::doNext();
    storeData();
    return ret;
  }

  void storeData();

  ItemOptimized value_;
};

}
