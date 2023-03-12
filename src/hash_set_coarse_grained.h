#ifndef HASH_SET_COARSE_GRAINED_H
#define HASH_SET_COARSE_GRAINED_H

#include <cassert>
#include <vector>
#include <functional>
#include <iostream>
#include <atomic>

#include "src/hash_set_base.h"

template <typename T>
class HashSetCoarseGrained : public HashSetBase<T> {
 public:
  explicit HashSetCoarseGrained(size_t initial_capacity) {
    std::scoped_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < initial_capacity; i++) 
        table_.push_back(std::vector<T>());
    size_ = 0;
    len_ = initial_capacity;
  }

  bool Add(T elem) final {
    if (Contains(elem))
      return true;
    std::scoped_lock<std::mutex> lock(mutex_);
    table_[std::hash<T>()(elem) % len_].push_back(elem);
    size_ ++;
    if (Policy()) 
      Resize();
    return false;
  }

  bool Remove(T elem) final {
    std::scoped_lock<std::mutex> lock(mutex_);
    size_t index = std::hash<T>()(elem) % len_;
    for (size_t i = 0; i < table_[index].size(); i++) {
      if (table_[index][i] == elem) {
        table_[index].erase(table_[index].begin() + static_cast<long>(i));
        size_ --;
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    std::scoped_lock<std::mutex> lock(mutex_);
    auto bucket = table_[std::hash<T>()(elem) % len_];
    for (T e : bucket) {
      if (e == elem) 
        return true;
    }
    return false;
  }

  [[nodiscard]] size_t Size() final {
    std::scoped_lock<std::mutex> lock(mutex_);
    return size_;
  }

  void PrintAll() {
    for (auto bucket : table_) {
      for (T e : bucket) 
        std::cout << "  " << e;
    }
    std::cout << std::endl;
  }

private:
  std::mutex mutex_;
  std::vector<std::vector<T>> table_;
  size_t size_; /* Number of elements. */
  size_t len_; /* Number of buckets (length of table_). */

  bool Policy() {
    return size_ / len_ > 4;
  }

  void Resize() {
    size_t old_len = len_;
    if (old_len != len_) 
      return;
    len_ = len_ * 2;
    std::vector<std::vector<T>> old_table = table_;
    table_.clear();
    table_.resize(len_);
    for (auto bucket : old_table) {
      for (T elem : bucket) {
        table_[std::hash<T>()(elem) % len_].push_back(elem);
      }
    }
  }
};


#endif  // HASH_SET_COARSE_GRAINED_H
