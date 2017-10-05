#ifndef PR_UTILS_H
#define PR_UTILS_H

#include <assert.h>
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

vector<int> create_payload(int _0 = 0, int _1 = 0, int _2 = 0, int _3 = 0,
                     int _4 = 0, int _5 = 0, int _6 = 0, int _7 = 0) {
   auto payload = vector<int> {
           _0, _1, _2, _3, _4, _5, _6, _7
   };

   return payload;
}

vector<int> split_64(uint64_t) {

}

uint64_t merge_64(int _0, int _1) {
   uint64_t ret = 0;

   return ret;
}

#endif //PR_UTILS_H
