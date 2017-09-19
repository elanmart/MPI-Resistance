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


class AcceptorQueue{
public:
    vector<acceptor_queue_entry> storage_;
    set<int> seen_ids_;

    void perhaps_insert_id(uint64_t T_request_sent, int process_id) {

        if (contains(seen_ids_, process_id))
            return;

        auto nested_queue = vector<time_int_pair>();
        auto entry        = make_tuple(T_request_sent, process_id, nested_queue);

        seen_ids_.insert(process_id);
        storage_.push_back(entry);
        sort(storage_.begin(), storage_.end(), tuple_compare_by_first);
    }

    void add_response_entry(uint64_t T_request_sent, int process_id,
                            uint64_t T_request_recieved, int acceptor_id) {

        perhaps_insert_id(T_request_sent, process_id);

        for (auto item : storage_) {
            if (get<1>(item) == process_id) {

                auto& nested_queue = get<2>(item);
                nested_queue.push_back(make_pair(T_request_recieved, acceptor_id));
                sort(nested_queue.begin(), nested_queue.end(), pair_compare_by_first);

            }
        }
    }

private:
    typedef pair<uint64_t, int> time_int_pair;
    typedef tuple<uint64_t, int, std::vector<time_int_pair>> acceptor_queue_entry;

    bool pair_compare_by_first(time_int_pair left, time_int_pair right) {
       return (left.first < right.first);
    };

    bool tuple_compare_by_first(acceptor_queue_entry left, acceptor_queue_entry right) {
        return (get<0>(left) < get<0>(right));
    };
};


#endif //PR_UTILS_H
