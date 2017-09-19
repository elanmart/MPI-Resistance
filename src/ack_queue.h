#ifndef PR_ACCEPTORQUEUE_H
#define PR_ACCEPTORQUEUE_H

#include "common.h"
#include "utils.h"


class AcceptorQueue{
public:
    AcceptorQueue() {};
    AcceptorQueue(int n_expected_reports, int process_limit) {
        n_expected_reports_ = n_expected_reports;
        process_limit_      = process_limit;

        storage_            = vector<MainQueueEntry>();
        seen_ids_           = set<int>();
        response_counter_   = map<int, int>();
    }

    int n_expected_reports_;
    int process_limit_;

    set<int> seen_ids_;
    map<int, int> response_counter_;
    vector<MainQueueEntry> storage_;

    void remove_entry(int process_id) {
        for (uint32_t i = 0; i<storage_.size(); i++) {

            auto& item = storage_.at(i);

            if (item.process_id_ == process_id) {
                storage_.erase(storage_.begin() + i);
                return;
            }
        }
    }

    void perhaps_insert_id(uint64_t T_request_sent, int process_id, int process_level, int n_requested);

    void add_response_entry(uint64_t T_request_sent, int process_id, int process_level, int n_requested,
                            uint64_t T_request_recieved, int acceptor_id, int acceptor_level);

    int num_responses(int process_id);

    MainQueueEntry get(int process_id);

    bool check_ready_and_limits(MainQueueEntry& item, int acceptor_id);

    bool check_is_first_higher(MainQueueEntry& item, int acceptor_id);

    bool check_all_invalid(MainQueueEntry& item, int acceptor_id);

    int get_answer(int acceptor_id, int process_id);

private:
    typedef tuple<uint64_t, int, int> nested_queue_entry;
    typedef tuple<uint64_t, int, int, std::vector<nested_queue_entry>> acceptor_queue_entry;
};


class MainQueueEntry {
public:
    MainQueueEntry() {};
    MainQueueEntry(uint64_t T_request_sent, int process_id, int process_level, int n_requested) {
        T_request_sent_ = T_request_sent;
        process_id_     = process_id;
        process_level_  = process_level;
        n_requested_    = n_requested;

        reports_ = vector<NestedQueueEntry>();
    };

    uint64_t T_request_sent_;
    int process_id_;
    int process_level_;
    int n_requested_;

    vector<NestedQueueEntry> reports_;

    bool operator<(const MainQueueEntry& other) {
        return T_request_sent_ < other.T_request_sent_;
    }
};

class NestedQueueEntry {
public:
    NestedQueueEntry(uint64_t T_request_recieved, int acceptor_id, int acceptor_level) {
        T_request_recieved_ = T_request_recieved;
        acceptor_id_        = acceptor_id;
        acceptor_level_     = acceptor_level;
    };

    uint64_t T_request_recieved_;
    int acceptor_id_;
    int acceptor_level_;

    bool operator<(const NestedQueueEntry& other) {
        return T_request_recieved_ < other.T_request_recieved_;
    }
};


#endif //PR_ACCEPTORQUEUE_H
