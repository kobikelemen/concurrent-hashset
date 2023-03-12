#ifndef HASH_SET_STRIPED_H
#define HASH_SET_STRIPED_H

#include <cassert>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>
#include <atomic>

#include "src/hash_set_base.h"

template <typename T>
class HashSetStriped : public HashSetBase<T> {
 public:
  explicit HashSetStriped(size_t initial_capacity) {
    for (size_t i = 0; i < initial_capacity; i++) {
      table_.push_back(std::vector<T>());
      // std::unique_ptr<std::mutex> mutex_ptr(new std::mutex());
      // mutexes_.emplace_back(std::move(mutex_ptr));
      std::mutex *m = new std::mutex();
      mutexes_.emplace_back(m);
    }
    size_.store(0);
    len_.store(initial_capacity);
    num_locks_.store(initial_capacity);
  }

  bool Add(T elem) final {
    if (Contains(elem))
      return true;
    size_t mutex_index = std::hash<T>()(elem) % num_locks_.load();
    std::unique_lock<std::mutex> lock(*mutexes_[mutex_index]);
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    table_[table_index].push_back(elem);
    size_.fetch_add(1);
    lock.unlock();

    if (Policy()) 
      Resize();
    return false;
  }

  bool Remove(T elem) final {
    size_t mutex_index = std::hash<T>()(elem) % num_locks_.load(); 
    std::scoped_lock<std::mutex> lock(*mutexes_[mutex_index]);
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    
    for (size_t i = 0; i < table_[table_index].size(); i++) {
      if (table_[table_index][i] == elem) {
        table_[table_index].erase(table_[table_index].begin() + static_cast<long>(i));
        size_.fetch_sub(1);
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    size_t mutex_index = std::hash<T>()(elem) % num_locks_.load(); 
    // std::cout << "mutex_index: " << mutex_index << " mutexes.size(): " << mutexes_.size() << std::endl;
    std::scoped_lock<std::mutex> lock(*mutexes_[mutex_index]);
    size_t table_index = std::hash<T>()(elem) % len_.load(); 

    auto bucket = table_[table_index];
    for (T e : bucket) {
      if (e == elem) 
        return true;
    }
    return false;
  }

  [[nodiscard]] size_t Size() final {
    return size_.load();
  }

  void PrintAll() {
    for (auto bucket : table_) {
      for (T e : bucket) {
        std::cout << "  " << e;
      }
    }
    std::cout << std::endl;
  } 

private:
  std::vector<std::vector<T>> table_;
  std::atomic<size_t> size_; /* Number of elements. */
  std::atomic<size_t> len_; /* Number of buckets (length of table_). */
  std::atomic<size_t> num_locks_;
  std::vector<std::unique_ptr<std::mutex>> mutexes_;

  bool Policy() {
    return size_.load() / len_.load() > 4;
  }

  void Resize() {
    size_t old_length = len_.load();
    std::vector<size_t> mutex_indexes;
    size_t i = 0;
    /* Need all buckets locked to resize. */
    for (std::unique_ptr<std::mutex> &mutex : mutexes_) {
      mutex->lock();
      mutex_indexes.push_back(i);
      i ++;
    }
    /* Another thread beat this one. */
    if (old_length != len_.load()) {
      for (size_t j : mutex_indexes) 
        mutexes_[j]->unlock();
      return;
    }
    
    /* Resize. */
    len_.store(len_.load() * 2);
    std::vector<std::vector<T>> old_table = table_;
    table_.clear();
    table_.resize(len_.load());
    for (auto bucket : old_table) {
      for (T elem : bucket) {
        table_[std::hash<T>()(elem) % len_.load()].push_back(elem);
      }
    }
    for (std::unique_ptr<std::mutex> &mutex : mutexes_)
      mutex->unlock();
  }
};

#endif  // HASH_SET_STRIPED_H
