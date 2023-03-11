#include <iostream>

#include "src/hash_set_sequential.h"

int main(int /*argc*/, char** /*argv*/) {
    HashSetSequential<int> hash_set = HashSetSequential<int>(10);
    hash_set.Add(5);
    hash_set.Add(1);
    hash_set.Add(4);
    hash_set.Add(2);
    hash_set.Add(7);
    hash_set.Add(11);
    hash_set.Add(21);
    hash_set.Add(23);
    hash_set.Add(13);
    hash_set.Add(15);
    hash_set.Add(14);
    hash_set.Add(33);
    hash_set.Add(55);

    hash_set.Remove(15);
    
    std::cout << hash_set.Contains(15) << std::endl << hash_set.Contains(1) << std::endl << hash_set.Contains(69) << std::endl << hash_set.Size() << std::endl;

}
