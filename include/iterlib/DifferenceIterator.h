#pragma once

#include "iterlib/Iterator.h"

namespace iterlib {

class DifferenceIterator : public CompositeIterator {
public:
  DifferenceIterator(IteratorVector& children)
    : CompositeIterator(children) {
  }

  virtual ~DifferenceIterator() {}

  virtual const Item& value() const override {
    return iterators_.front()->value();
  }

  const std::unique_ptr<Iterator>& getFirstIterator() const {
    return iterators_.front();
  }

  const std::unique_ptr<Iterator>& getSecondIterator() const {
    return iterators_.back();
  }

protected:
  bool advanceToNextDifference();
  bool doNext() override;
  bool doSkipTo(id_t id) override;
};

}
