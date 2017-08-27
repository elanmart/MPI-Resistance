#ifndef PR_UTILS_H
#define PR_UTILS_H

#include "common.h"


template <typename T>
bool inline contains(set<T> collection, T item) {
   return collection.find(item) != collection.end();
}

inline int randint(int low, int high) {
   assert(low <= high);

   random_device rd;
   mt19937 rng(rd());
   uniform_int_distribution<int> dist(low, high);

   return dist(rng);
}


#endif //PR_UTILS_H
