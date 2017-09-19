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

// TIME PRIORITY QUEUE
typedef pair<uint64_t, int> time_int_pair;

bool pair_compare_by_first(time_int_pair left, time_int_pair right) {
    return (left.first < right.first);
};

typedef priority_queue<
        time_int_pair,
        std::vector<time_int_pair>,
        decltype(pair_compare_by_first)
> T_priority_queue;


// ACCEPTOR QUEUE
typedef tuple<uint64_t, int, T_priority_queue> acceptor_queue_entry;

bool tuple_compare_by_first(acceptor_queue_entry left, acceptor_queue_entry right) {
   return (get<0>(left) < get<0>(right));
};

typedef priority_queue<
        acceptor_queue_entry,
        std::vector<acceptor_queue_entry>,
        decltype(tuple_compare_by_first)
> acceptor_queue_t;


bool id_in_queue(int id, acceptor_queue_t queue) {
   for (auto item : queue) {
      if (get<1>(item) == id)
         return true;
   }
   return false;
}


#endif //PR_UTILS_H
