//  Copyright (c) 2016, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
#pragma once

#include "iterlib/WrappedIterator.h"

namespace iterlib {

class LimitIterator : public WrappedIterator {
 public:
  // Collect count results starting at startOffset
  LimitIterator(Iterator* iter, int count, id_t startOffset)
      : WrappedIterator(iter), count_(count), startOffset_(startOffset),
        firstTime_(true) {}

  std::string cookie() const override { return innerIter_->cookie(); }

  virtual bool orderPreserving() const override { return true; }

 protected:
  bool doNext() override;

 private:
  int32_t count_;
  id_t startOffset_;

  bool firstTime_;
};
}
