#ifndef PR_NODE_H
#define PR_NODE_H

#include <mutex>
#include <map>
#include "common.h"
#include "comm.h"
#include "utils.h"

class Manager; // todo: handle this;


enum class ResourceState {
    IDLE,    // Resource not used
    WAITING, // Resource waiting for response
    LOCKED   // Resource being used
};

enum class MeetingState {
    IDLE,       // Not participating, can join
    WAITING,    // Waiting for response (meeting start or decline)
    LOCKED,     // Meeting in progress
    MASTER_ORG, // Meeting organizer
    SLAVE_ORG   // Meeting participant - TODO: Difference between Locked?
};


class Node {
public:
    // ctors
    Node();
    Node(int ID);
    Node(int *buffer);

    typedef void (Node::*comm_method)(Message);
    std::map<Words, comm_method> comm_func_map_t;

    // Identity
    int32_t ID_;    // ID
    int32_t level_; // Depth in tree
    bool is_acceptor_;

    // Topology
    int32_t parent_;
    set<int32_t> neighbours_;
    set<int32_t> children_;
    set<int> invitees_;

    // Serialization
    pair<int, int *> serialize();

    void deserialize(int *buffer);

    // Communication interface
    Manager *manager_ = nullptr;  // MPI Interface. Contains send and receive queues
    void set_manager(Manager *m); // Sets up a manager

    // Configuration variables
    int resource_count_;
    int awaiting_response_;
    int time_penalty_;
    double percentage_threshold_;

    set<int> participants_;
    queue<int> resource_answer_queue_;
    MeetingState meeting_state_;
    ResourceState resource_state_;

    void start_event_loop(); // Initializes main event loop

    void send_new_message(int destination, Words w, int *payload = nullptr);

    void _send(Message msg);

    bool get(Message *msg);

    void broadcast(Message msg); // Sends a message to all neighbours, children and parent
    bool accept(Message &msg);   // Checks if we haven't received that msg already
    void consume(Message &msg);  // Checks if message should be consumed by us or forwarded
    void forward(Message msg, int target);  // Sends message to it's neighbours

    void send_to(Message msg, set<int> recipients); // Sends message to a set of receipents
    void send_to(Message msg, int dest); // Sends message to destination

    // message bookkeeping
    set<int64_t> msg_cache_; // Set of "seen messages"
    int msg_number_;         // Sequential number assigned to every new message

    // Main loop exec function, processes message and generates response
    void handle(Message msg);

    // synchronization
    bool STOP_;

private:
    void initialize_meeting_procedure();
    void try_start_meeting();   // After receiving a response from Invitee, check if everyone already responded. If true, start meeting
    void invite_participants(); // After getting permission, invites participants
    void meet();
    void ask_for_resource();    // Asks anyone for resource
    void ask_for_acceptance();  // Asks someone higher in the hierarchy for permission to organize meeting
    void on_resource_available();
    void resource_answer(int id);       // Answer a process requesting a resource. This may happen in several places
    void perhaps_next_answer();

    void HandleMeetingInvitiation(Message msg);

    void HandleMeetingInvitationAccept(Message msg);

    void HandleNoneMessage(Message msg);

    void HandleMeetingCancel(Message msg);

    void HandleMeetingInvitationDecline(Message msg);

    void HandleMeetingStart(Message msg);

    void HandleResourceRequest(Message msg);

    void HandleResourceAnswer(Message msg);

    void HandleResourceAck(Message msg);

    void HandleResourceDenial(Message msg);

    void HandleResourceDelivery(Message msg);

    void HandleMeetingAcceptanceRequest(Message msg);

    void HandleMeetingAcceptanceAnswer(Message msg);

    void HandleMeetingAcceptanceAck(Message msg);

    void HandleMeetingAcceptanceDenial(Message msg);

    void HandleMeetingAcceptanceDelivery(Message msg);

    void initialize_mapping();

    void HandleMeetingEnd(Message msg);
};

// Logging Helpers
#define DASH "=====================================\n"
#define LOG(msg, ...) printf(DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n",  \
                             this->ID_, ##__VA_ARGS__)


#endif //PR_NODE_H
