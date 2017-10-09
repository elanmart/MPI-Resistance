#ifndef PR_ACCEPTORQUEUE_H
#define PR_ACCEPTORQUEUE_H

#include "common.h"

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

    bool operator<(const NestedQueueEntry& other) const {
        return T_request_recieved_ < other.T_request_recieved_;
    }
};


class MainQueueEntry {
public:
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

    bool operator<(const MainQueueEntry& other) const {
        return T_request_sent_ < other.T_request_sent_;
    }

private:
    MainQueueEntry() {};

};


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

    void remove_entry(int process_id);

    void perhaps_insert_id(uint64_t T_request_sent, int process_id, int process_level, int n_requested);

    void add_response_entry(int process_id, uint64_t T_request_recieved, int acceptor_id, int acceptor_level);

    int num_responses(int process_id);

    MainQueueEntry get(int process_id);

    bool check_ready_and_limits(MainQueueEntry& item);

    bool check_is_first_higher(MainQueueEntry& item, int acceptor_id);

    bool check_all_invalid(MainQueueEntry& item);

    int get_answer(int acceptor_id, int process_id);

    void replace_id(int);

};


#define DASH "=====================================\n"
#define QUEUE_LOG(msg, ...) printf(DASH                 \
                             "msg  :: " msg "\n",  \
                             ##__VA_ARGS__)

#endif //PR_ACCEPTORQUEUE_H
