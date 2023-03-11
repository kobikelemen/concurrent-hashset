#include "src/hash_set_sequential.h"



HashSetSequantial::HashSetSequential(size_t init_cap) {
    for (auto x : init_cap) {
        table_.push_back(std::vector<T>());
    }
    size_ = 0;
    len_ = init_cap;
}

bool HashSetSequential::Add(T elem) {
    table_[std::hash<T>()(elem) % len].push_back(elem);
    if (Policy()) 
        Resize();
}

bool HashSetSequential::Remove(T elem) {

}

bool HashSetSequential::Contains(T elem) {

}

size_t HashSetSequential::Size() {
    return size_;
}