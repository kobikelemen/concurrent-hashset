#ifndef HASH_SET_REFINABLE_H
#define HASH_SET_REFINABLE_H

#include <cassert>

#include "src/hash_set_base.h"

struct ResizeUsage
{
  std::thread_id id;
  bool mark;
};



template <typename T>
class HashSetRefinable : public HashSetBase<T> {
 public:
  explicit HashSetRefinable(size_t /*initial_capacity*/) {
    struct ResizeUsage *resize = new ResizeUsage();
    resize->who = -1;
    resize->mark = false;
    owner_.store(resize);

    for (size_t i = 0; i < initial_capacity; i++) {
      table_.push_back(std::vector<T>());
      std::mutex *m = new std::mutex();
      mutexes_.emplace_back(m);
    }
    size_.store(0);
    len_.store(initial_capacity);
    num_locks_.store(initial_capacity);
  }


  bool Add(T elem) final {
    std::unique_lock<std::mutex> lock = Lock();
    if (ContainsPriv(elem))
      return true;
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    table_[table_index].push_back(elem);
    size_.fetch_add(1);
    lock.unlock();

    if (Policy()) 
      Resize();
    return false;
  }


  bool Remove(T elem) final {
    std::unique_lock<std::mutex> lock = Lock();
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    for (size_t i = 0; i < table_[table_index].size(); i++) {
      if (table_[table_index][i] == elem) {
        table_[table_index].erase(table_[table_index].begin() + static_cast<long>(i));
        size_.fetch_sub(1);
        lock.unlock();
        return true;
      }
    }
    lock.unlock();
    return false;
  }


  [[nodiscard]] bool Contains(T elem) final {
    std::unique_lock<std::mutex> lock = Lock();
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    auto bucket = table_[table_index];
    for (T e : bucket) {
      if (e == elem) 
        return true;
    }
    lock.unlock();
    return false;
  }

  [[nodiscard]] size_t Size() final {
    return size_.load();
  }


private:

  std::vector<std::vector<T>> table_;
  std::atomic<size_t> size_; /* Number of elements. */
  std::atomic<size_t> len_; /* Number of buckets (length of table_). */
  std::atomic<size_t> num_locks_;
  std::atomic<ResizeUsage*> owner_;
  std::vector<std::unique_ptr<std::mutex>> mutexes_;


  std::unique_lock<std::mutex> Lock() {

    std::thread_id me = std::this_thread::get_id();
    ResizeUsage *who;
    while (true) {
      do {
        who = owner_.load();
      } while (who->mark && who->id != me);

      // int has_occured = x.lo
      std::vector<std::unique_ptr<std::mutex>> old_mutexes = mutexes_;
      std::unique_ptr<std::mutex>> *old_mutex = old_mutexes[std::hash<T>()(elem) % old_mutexes.size()];
      std::unique_lock<std::mutex> old_lock(*old_mutex);
      who = owner_.load();
      if ((!who->mark || who->id == me) && mutexes_ == old_mutexes) {
        return std::move(old_lock);
      } else {
        old_lock.unlock();
      }
    }
  }

  // void Unlock(T elem) {

  // }

  bool Policy() {
    return (size_.load()*1.0) / (len_.load()*1.0) > 4;
  }

  // void Resize() {
  //   size_t old_len = len_.load();
  //   int new_len = 2 * size_.load();
  //   ResizeUsage resize = {std::this_thread::get_id(), false};
  //   if (owner_.compare_exchange_strong({-1, resize.me}, {false, true})) {
  //     if (len_.load() != old_len) {
  //       return;
  //     }
  //     for ()
  //   }
  // }

  [[nodiscard]] bool ContainsPriv(T elem) {
    size_t table_index = std::hash<T>()(elem) % len_.load(); 
    auto bucket = table_[table_index];
    for (T e : bucket) {
      if (e == elem) 
        return true;
    }
    return false;
  }
};

#endif  // HASH_SET_REFINABLE_H
