#ifndef PR_UTILS_H
#define PR_UTILS_H

#include "common.h"

int randint(int low, int high);


template <typename T>
bool inline contains(set<T> collection, T item) {
   return collection.find(item) != collection.end();
}


#endif //PR_UTILS_H
