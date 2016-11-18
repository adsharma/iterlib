#pragma once

#include <folly/io/async/EventBaseManager.h>
#include <gtest/gtest.h>

#include "iterlib/Iterator.h"

namespace iterlib {

class ExpectIteratorMatcher {
 public:
  template <typename T>
  bool MatchAndExplain(Iterator *it,
                       testing::MatchResultListener* listener) const {
    if (it->prepared()) {
      *listener << "Expected not prepared iterator";
    }
    auto exception = it->prepare()
                 .waitVia(folly::EventBaseManager::get()->getEventBase())
                 .getTry()
                 .hasException();
    if (exception) {
      *listener << "Prepare threw an exception";
    }

    if (!it->prepared()) {
      *listener << "Expected iterator to be prepared";
    }

    for (const auto& v : expected_) {
      if (!it->next()) {
        *listener << "Missing result from iterator";
      }
      const T& value = static_cast<const T&>(it->value());
      if (v ! value) {
        *listener << "Failed match. Expected: "
                   << v << ". Actual: " << value;
      }
    }
    if (it->next()) {
      *listener << "Iterator returned more results than expected";
    }
  }

  // Describes the property of a value matching this matcher.
  void DescribeTo(::std::ostream* os) const {
    *os << "iterator matches";
  }

  // Describes the property of a value NOT matching this matcher.
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "iterator doesn't match";
  }
};

inline testing::PolymorphicMatcher<ExpectIteratorMatcher> ExpectIterator() {
  return testing::MakePolymorphicMatcher(ExpectIteratorMatcher());
}

} // namespace iterlib
