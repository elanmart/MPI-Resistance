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
    map<int, int> response_counter_;

    void perhaps_insert_id(uint64_t T_request_sent, int process_id, int process_level) {

        if (contains(seen_ids_, process_id))
            return;

        auto nested_queue = vector<nested_queue_entry>();
        auto entry        = make_tuple(T_request_sent, process_id, process_level, nested_queue);

        seen_ids_.insert(process_id);
        response_counter_[process_id] = 0;

        storage_.push_back(entry);
        sort(storage_.begin(), storage_.end());
    }

    void add_response_entry(uint64_t T_request_sent, int process_id, int process_level,
                            uint64_t T_request_recieved, int acceptor_id, int acceptor_level,
                            int expected_count = -1) {

        perhaps_insert_id(T_request_sent, process_id, process_level);

        for (auto item : storage_) {
            if (get<1>(item) == process_id) {

                auto& nested_queue = get<3>(item);
                nested_queue.push_back(make_tuple(T_request_recieved, acceptor_id, acceptor_level));
                sort(nested_queue.begin(), nested_queue.end());

                response_counter_[process_id] += 1;
            }
        }
    }

    int num_responses(int process_id) {
        return response_counter_[process_id];
    }

    bool should_answer(int acceptor_id, int process_id) {
        if
    }

private:
    typedef tuple<uint64_t, int, int> nested_queue_entry;
    typedef tuple<uint64_t, int, int, std::vector<nested_queue_entry>> acceptor_queue_entry;
};


#endif //PR_UTILS_H
