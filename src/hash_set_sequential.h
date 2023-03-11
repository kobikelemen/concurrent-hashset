#ifndef HASH_SET_SEQUENTIAL_H
#define HASH_SET_SEQUENTIAL_H

#include <cassert>
#include <vector>
#include <functional>

#include "src/hash_set_base.h"

template <typename T>
class HashSetSequential : public HashSetBase<T> {
 public:
  explicit HashSetSequential(size_t /*initial_capacity*/) {}

  bool Add(T /*elem*/) final {
    assert(false && "Not implemented yet");
    return false;
  }

  bool Remove(T /*elem*/) final {
    assert(false && "Not implemented yet");
    return false;
  }

  [[nodiscard]] bool Contains(T /*elem*/) final {
    assert(false && "Not implemented yet");
    return false;
  }

  [[nodiscard]] size_t Size() const final {
    assert(false && "Not implemented yet");
    return 0u;
  }

private:
  std::vector<std::vector<T>> table_;
  size_t size_; /* Number of elements. */
  size_t len_;
};

#endif  // HASH_SET_SEQUENTIAL_H
