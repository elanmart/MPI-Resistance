#ifndef PR_NODE_H
#define PR_NODE_H

#include <mutex>
#include <map>
#include "common.h"
#include "comm.h"
#include "ack_queue.h"

class Manager;


enum class ResourceState {
    IDLE,                                                             // Resource not used
    WAITING,                                                          // Resource waiting for response
    LOCKED,                                                           // Resource being used
    NEEDED,                                                           // Resource available, but can't be used yet.
};

enum class MeetingState {
    IDLE,                                                             // Not participating, can join
    LOCKED,                                                           // Meeting in progress
    MASTER_ORG,                                                       // Meeting organizer
};

enum class AcceporState {
    None,
    Waiting,
    TakingOver,
    Active,
    Retired
};


class Node {
public:
    void debug();

    // ctors
    Node();
    explicit Node(int ID);
    explicit Node(int *buffer);
    void set_manager(Manager *m);                                       // Sets up a manager

    // Identity
    int32_t ID_;                                                      // ID
    int32_t level_;                                                   // Depth in tree
    int32_t is_acceptor_;

    // Configuration variables
    int resource_count_;
    double percentage_threshold_;

    // Topology
    int32_t parent_;
    set<int32_t> neighbours_;
    set<int32_t> children_;

    // Communication interface
    Manager *manager_ = nullptr;                                        // MPI Interface. Contains send and receive queues

    // Node States
    MeetingState  meeting_state_;
    ResourceState resource_state_;
    set<int> invitees_;
    set<int> participants_;
    queue<int> resource_answer_queue_;
    int awaiting_response_;
    int awaiting_confirmations_;
    uint64_t time_penalty_;

    // message bookkeeping
    uint64_t T_;
    set<int64_t> msg_cache_;                                             // Set of "seen messages"
    int msg_number_;                                                     // Sequential number assigned to every new message
    int operation_number_;

    // Message handlers map
    typedef void (Node::*comm_method)(Message);
    std::map<Words, comm_method> comm_func_map_t;

    // Acceptor queues
    AcceporState acceptor_state;
    AcceptorQueue acceptance_queue_;
    map<int64_t, vector<Message>> unanswered_requests_;
    map<int, uint64_t> timestamps_log_;
    uint64_t timestamp_limit;
    int64_t trigger_;
    int sucessor_id_;
    int retry_await_;
    int last_retry_;
    int now_;
    void initialize_role_transfer();
    void set_trigger(Message& msg);

    // synchronization
    bool STOP_;

    // main event loop
    void start_event_loop();                                             // Initializes main event loop

    // message passing
    bool get(Message *msg);
    void send_new_message(int destination, Words w, int *payload = nullptr);
    void send_new_acceptor_message(int destination, Words w, int *payload = nullptr);
    void broadcast(Message msg);
    void send_to(Message msg, set<int> recipients);                      // Sends message to a set of receipents
    void send_to(Message msg, int dest);                                 // Sends message to destination

    // message handling
    bool accept(Message &msg);                                           // Checks if we haven't received that msg already
    void consume(Message &msg);                                          // Checks if message should be consumed by us or forwarded
    void handle(Message msg);

    // Serialization
    pair<int, int *> serialize();
    void deserialize(int *buffer);

    // meetings
    void initialize_meeting_procedure();
    void check_invite_responses();                                              // After receiving a response from Invitee, check if everyone already responded. If true, start meeting
    void invite_participants();                                            // After getting permission, invites participants
    void meeting_cancel();

    // resource / ack acquisition
    void ask_for_resource();                                               // Asks anyone for resource
    void ask_for_acceptance();                                             // Asks someone higher in the hierarchy for permission to organize meeting
    void on_resource_available();
    void resource_answer(int id);                                          // Answer a process requesting a resource. This may happen in several places
    void perhaps_next_answer();
    void perhaps_meeting_answer(int process_id);

    // Message handlers
    void initialize_mapping();

    void HandleNoneMessage(Message msg);


    void HandleMeetingInvitiation(Message msg);

    void HandleMeetingInvitationAccept(Message msg);

    void HandleMeetingInvitationDecline(Message msg);

    void HandleMeetingStart(Message msg);

    void HandleMeetingEnd(Message msg);

    void HandleMeetingDone(Message msg);

    void HandleMeetingCancel(Message msg);


    void HandleResourceRequest(Message msg);

    void HandleResourceAnswer(Message msg);

    void HandleResourceAck(Message msg);

    void HandleResourceDenial(Message msg);

    void HandleResourceDelivery(Message msg);


    void HandleMeetingAcceptanceRequest(Message msg);

    void HandleMeetingAcceptanceReport(Message msg);

    void HandleMeetingAcceptanceGranted(Message msg);

    void HandleMeetingAcceptanceDenied(Message msg);

    void HandleMeetingAcceptanceFullfilled(Message msg);


    void HandleAcceptorPassOffer(Message msg);

    void HandleAcceptorPassAck(Message msg);

    void HandleAcceptorPassCancel(Message msg);

    void HandleAcceptorPassDeny(Message msg);

    void HandleAcceptorPassConfirm(Message msg);

    void HandleAcceptorPassTest(Message msg);

    void HandleAcceptorPassRelease(Message msg);

    void pass(int sender_id);

    void perhaps_release();
};

// Logging Helpers
#define DASH "=====================================\n"
#define NODE_LOG(msg, ...) printf(DASH                 \
                             "Node :: %d     \n"  \
                             "Operation :: %d     \n"  \
                             "msg  :: " msg "\n",  \
                             this->ID_, this->operation_number_, ##__VA_ARGS__)


#endif //PR_NODE_H
