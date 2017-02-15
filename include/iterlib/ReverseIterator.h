#pragma once

#include "iterlib/WrappedIterator.h"

namespace iterlib {

// reverse the order of the WrappedIterator
template <typename T=Item>
class ReverseIterator : public WrappedIterator<T> {
public:
  explicit ReverseIterator(Iterator<T>* iter)
    : WrappedIterator<T>(iter)
    , firstTime_(true){
  }

  virtual const T& value() const override {
    return results_.back();
  }

protected:
 bool doNext() override;

private:
  // first time load all data into memory
  void load();

  bool firstTime_;

  std::vector<T> results_;
};

}
