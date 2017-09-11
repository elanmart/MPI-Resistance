#ifndef PR_UTILS_H
#define PR_UTILS_H

#include "common.h"

#define __unsued __attribute__((unused))

/*
 * zwraca true jeżeli zbiór zawiera item.
 */
template <typename T>
bool inline contains(set<T> collection, T item) {
   return collection.find(item) != collection.end();
}

/*
 * Tak, w cpp potrzeba 5 LOC żeby wylosować inta.
 */
inline int randint(int low, int high) {
   assert(low <= high);

   random_device rd;
   mt19937 rng(rd());
   uniform_int_distribution<int> dist(low, high);

   return dist(rng);
}


#endif //PR_UTILS_H
