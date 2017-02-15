#include "iterlib/ReverseIterator.h"

namespace iterlib {

template <typename T>
void ReverseIterator<T>::load() {
  while (this->innerIter_->next()) {
    results_.emplace_back(this->innerIter_->value());
  }
}

template <typename T>
bool ReverseIterator<T>::doNext() {
  if (this->done()) {
    return false;
  }

  // only get samples in the first time
  if (firstTime_) {
    firstTime_ = false;
    load();
  } else {
    results_.pop_back();
  }

  if (results_.empty()) {
    this->setDone();
    return false;
  }

  return true;
}

}
