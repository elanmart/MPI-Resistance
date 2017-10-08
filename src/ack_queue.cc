//
// Created by elan on 19.09.17.
//

#include "ack_queue.h"
#include "utils.h"

void AcceptorQueue::perhaps_insert_id(uint64_t T_request_sent, int process_id, int process_level, int n_requested) {

    if (contains(seen_ids_, process_id))
        return;

    auto entry        = MainQueueEntry(T_request_sent, process_id, process_level, n_requested);

    seen_ids_.insert(process_id);
    response_counter_[process_id] = 0;

    storage_.push_back(entry);
    sort(storage_.begin(), storage_.end());
}

void AcceptorQueue::add_response_entry(uint64_t T_request_sent, int process_id, int process_level, int n_requested,
                                       uint64_t T_request_recieved, int acceptor_id, int acceptor_level) {

    perhaps_insert_id(T_request_sent, process_id, process_level, n_requested);

    for (auto& item : storage_) {
        if (item.process_id_ == process_id) {

            auto& nested_queue = item.reports_;
            nested_queue.push_back(NestedQueueEntry(T_request_recieved, acceptor_id, acceptor_level));
            sort(nested_queue.begin(), nested_queue.end());

            response_counter_[process_id] += 1;
        }
    }
}

int AcceptorQueue::num_responses(int process_id) {
    return response_counter_[process_id];
}

MainQueueEntry AcceptorQueue::get(int process_id) {
    for (auto& item : storage_)
        if (item.process_id_ == process_id)
            return item;

    throw ("KeyError: " + to_string(process_id) + " was not found in queue");
}

bool AcceptorQueue::check_ready_and_limits(MainQueueEntry &item, int acceptor_id) {
    int counter = 0;

    for (auto& other : storage_) {
        counter += other.n_requested_;

        if (other.reports_.size() < n_expected_reports_)
            return false;

        if (counter > process_limit_)
            return false;

        if (other.process_id_ == item.process_id_)
            return true;
    }
    return false;
}

bool AcceptorQueue::check_is_first_higher(MainQueueEntry &item, int acceptor_id) {

    for (auto& report : item.reports_) {
        if (report.acceptor_level_ > item.process_level_)
            return report.acceptor_id_ == acceptor_id;
    }

    return false;
}

bool AcceptorQueue::check_all_invalid(MainQueueEntry &item, int acceptor_id) {
    for (auto& report : item.reports_){
        if (report.acceptor_level_ > item.process_level_)
            return false;
    }

    return true;
}

int AcceptorQueue::get_answer(int acceptor_id, int process_id) {

    // process was not yet seen
    if (not contains(seen_ids_, process_id))
        return 0;

    // the request was not yet received by all acceptors
    if (num_responses(process_id) < n_expected_reports_)
        return 0;

    // for performance reasons, we want to iterate only once
    auto&& item = get(process_id);

    // all acceptors are lower than requesting process
    if (check_all_invalid(item, acceptor_id))
        return -1;

    // not all earlier requests are ready to be deployed or they exceed a total process limit
    if (not check_ready_and_limits(item, acceptor_id))
        return 0;

    // acceptor is the first acceptor that is higher than the process.
    if (check_is_first_higher(item, acceptor_id))
        return 1;

    // everything is ready but there are acceptors that recieved the request before us
    // and are also higher in the tree than the process
    return 0;
}
