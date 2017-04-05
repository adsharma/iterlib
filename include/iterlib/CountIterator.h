#pragma once

#include "iterlib/WrappedIterator.h"

namespace iterlib {
namespace detail {

// count the number of items in iter
template <typename T=Item>
class CountIterator : public WrappedIterator<T> {
public:
  explicit CountIterator(Iterator<T>* iter)
    : WrappedIterator<T>(iter)
    , countValue_(-1) {
  }

  virtual const T& key() const override {
    return kCountKey;
  }

  virtual const T& value() const override {
    return countValue_;
  }

  static const T kCountKey;

protected:
  bool doNext() override;

private:
  mutable T countValue_;
};

}

using CountIterator = detail::CountIterator<Item>;
extern template class detail::CountIterator<Item>;

}

#include "iterlib/CountIterator-inl.h"
