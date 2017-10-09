#include "node.h"
#include "utils.h"

#define MEETING_FREQ 1500
#define PASS_FREQ    1500
#define RETRY_AWAIT  10000
#define MIN_PARTICIP 0
#define TIME_PENALTY 0


// ctors
Node::Node() {
    T_ = 0;
    level_ = 0;
    parent_ = -1;
    msg_number_ = 0;
    last_retry_ = 0;
    acceptor_id_ = -1;
    time_penalty_ = 0;
    resource_count_ = 0;
    operation_number_ = 0;
    unanswered_requests_ = map<int64_t, vector<Message>>();
    awaiting_confirmations_ = 0;
    acceptor_state = AcceporState::None;
    resource_state_ = ResourceState::IDLE;
    meeting_state_ = MeetingState::IDLE;
    STOP_ = false;

    initialize_mapping();
}

Node::Node(int ID) : Node() {
    ID_ = ID;
}

Node::Node(int *buffer) : Node() {
    deserialize(buffer);
}

void Node::set_manager(Manager *m) {
    manager_ = m;
}

// main event loop
void Node::start_event_loop() {
    Message msg;
    manager_->start();

    if (acceptor_id_ >= 0) {
        NODE_LOG("Acceptor %d reporting for duty!", acceptor_id_);
    }

    assert(meeting_state_ == MeetingState::IDLE);

    while (not STOP_) {

        if (this->get(&msg))
            consume(msg);

        if (meeting_state_ == MeetingState::IDLE and ID_ == 7)
            initialize_meeting_procedure();
//
        if (acceptor_id_ == 0)
            initialize_role_transfer();

//        if ((rand() % MEETING_FREQ == 0) and (meeting_state_ == MeetingState::IDLE))
//            initialize_meeting_procedure();
//
//        if ((rand() % PASS_FREQ == 0) and (acceptor_state == AcceporState::Active))
//            initialize_role_transfer();

//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// message passing
bool Node::get(Message *msg) {
    auto msg_available = manager_->get(msg);

    if (msg_available) {
        T_ = max(msg->timestamp, T_);
        timestamps_log_[msg->sender] = msg->timestamp;

        perhaps_release();
    }

    return msg_available;
}

void Node::send_new_message(int destination, Words w, vector<int> &payload) {
    msg_number_ += 1;

    auto msg = create_message(msg_number_, ID_, destination, T_, w, payload);
    msg_cache_.insert(identifier(msg));

    broadcast(msg);
}

void Node::send_new_acceptor_message(int destination, Words w, vector<int> &payload) {
    if (acceptor_state == AcceporState::Active or acceptor_state == AcceporState::TakingOver) {
        NODE_LOG("Answering to %d", destination);
        send_new_message(destination, w, payload);


    } else if (acceptor_state == AcceporState::Retired) {
        auto ID = split_64(trigger_);
        payload[6] = ID.first;
        payload[7] = ID.second;

        send_new_message(sucessor_id_, Words::ACCEPTOR_PASS_TEST, payload);

    } else {
        auto answer = create_message(-1, -1, destination, T_, w, payload);
        unanswered_requests_[trigger_].push_back(answer);

    }
}

void Node::broadcast(Message msg) {
    send_to(msg, parent_);
    send_to(msg, children_);
    send_to(msg, neighbours_);
}

void Node::send_to(Message msg, set<int> recipients) {
    for (auto id : recipients)
        send_to(msg, id);
}

void Node::send_to(Message msg, int dest) {
    T_ += 1;
    manager_->put(msg, dest);
}

// message handling
bool Node::accept(Message &msg) {
    auto key = identifier(msg);

    if (msg_cache_.find(key) != msg_cache_.end()) {
        return false;
    } else {
        msg_cache_.insert(key);
        return true;
    }
}

void Node::consume(Message &msg) {
    if (not accept(msg))
        return;

    // not to me. Forward everywhyere
    if (msg.destination != ID_ or msg.destination == ALL) {
        broadcast(msg);
    }

    if (msg.destination == ID_ or msg.destination == ALL) {
        handle(msg);
    }
}

void Node::handle(Message msg) {
//    NODE_LOG("handle triggered for: %s %d", EnumStrings[msg.word], msg.word); //todo

    auto handler = comm_func_map_t[msg.word];
    return (this->*handler)(msg);
}

// Serialization
pair<int, vector<int>> Node::serialize(Config &cfg) {
    auto n_children = (int) children_.size();
    auto n_neighbours = (int) neighbours_.size();

    int n_items = 9 + n_children + n_neighbours + 1024;
    auto buffer = vector<int>(BUFFER_SIZE);

    buffer[0] = ID_;
    buffer[1] = level_;
    buffer[2] = parent_;
    buffer[3] = resource_count_;
    buffer[4] = acceptor_id_;
    buffer[5] = cfg.n_acceptors;
    buffer[6] = cfg.max_processes;
    buffer[7] = n_children;
    buffer[8] = n_neighbours;
    int offset = 9;

    for (auto item : children_)
        buffer[offset++] = item;

    for (auto item : neighbours_)
        buffer[offset++] = item;

    return make_pair(n_items, buffer);
}

void Node::deserialize(int *buffer) {
    ID_               = buffer[0];
    level_            = buffer[1];
    parent_           = buffer[2];
    resource_count_   = buffer[3];
    acceptor_id_      = buffer[4];
    n_acceptors_      = buffer[5];
    max_processes_    = buffer[6];
    int n_children    = buffer[7];
    int n_neighbours  = buffer[8];
    int offset = 9;

    children_.clear();
    neighbours_.clear();

    for (int i = offset; i < offset + n_children; i++)
        children_.insert(buffer[i]);

    offset += n_children;
    for (int i = offset; i < offset + n_neighbours; i++)
        neighbours_.insert(buffer[i]);

    if (acceptor_id_ >= 0)
        acceptor_state = AcceporState::Active;
    else
        acceptor_state = AcceporState::None;

    acceptance_queue_ = AcceptorQueue(n_acceptors_, max_processes_);
}

// meetings
void Node::initialize_meeting_procedure() {
    assert(meeting_state_ == MeetingState::IDLE &&
           "Can't initialize a meeting if I'm not IDLE.");

    if (time_penalty_ > T_)
        return;

    NODE_LOG("Let's organize a meeting!");

    meeting_state_ = MeetingState::MASTER_ORG;
    ask_for_resource();
}

void Node::invite_participants() {
    NODE_LOG("Inviting participants");

    NODE_LOG("num invitees_ before clear: %lu, num neighbours: %lu, num children: %lu",
             this->invitees_.size(), this->neighbours_.size(), this->children_.size());

    assert(this->meeting_state_ == MeetingState::MASTER_ORG &&
           "I've tried to invite participants but I'm not a MASTER_ORG!");

    set<int> invitees;
    this->invitees_.clear();

    invitees.insert(children_.begin(), children_.end());
    invitees.insert(neighbours_.begin(), neighbours_.end());
    if (parent_ != -1) {
        invitees.insert(parent_);
    }

    awaiting_response_ = 0;
    for (auto id : invitees) {

        if (awaiting_response_ >= (max_processes_ - 1))
            break;

        this->invitees_.insert(id);
        send_new_message(id, MEETING_INVITE);

        awaiting_response_ += 1;
    }

    NODE_LOG("I've sent invites to %d processes", awaiting_response_);
}

void Node::check_invite_responses() {

    assert(awaiting_response_ >= 0 &&
           "Awaiting negative amount of invitees.");

    if (awaiting_response_ > 0) {
        NODE_LOG("Still waiting for response from %d invitees...", awaiting_response_);
        return;
    }

    NODE_LOG("All participants available");

    bool enough_participants = (participants_.size() >= MIN_PARTICIP);

    if (enough_participants) {
        NODE_LOG("I have enough people, asking for meeting accept");

        ask_for_acceptance();

    } else {
        NODE_LOG("Not enough participants to start meeting, canceling...");

        meeting_cancel();
    }
}

void Node::meeting_cancel() {
    for (auto id : this->participants_) {
        send_new_message(id, MEETING_CANCEL);
    }

    participants_.clear();
    resource_state_ = ResourceState::IDLE;
    meeting_state_ = MeetingState::IDLE;

    perhaps_next_answer();
}

// resource / ack acquisition
void Node::ask_for_resource() {
    if (resource_count_ == 0) {
        NODE_LOG("I need resource. Please propagate this to everyone!");

        resource_state_ = ResourceState::WAITING;
        send_new_message(ALL, Words::RESOURCE_REQUEST);

    } else if (resource_count_ > 0) {

        if (resource_state_ == ResourceState::IDLE) {
            NODE_LOG("I've got resource, no need to ask.");

            on_resource_available();
        } else {
            NODE_LOG("I've got resource, But it's locked. Marking as NEEDED");

            resource_state_ = ResourceState::NEEDED;
        }
    }
}

void Node::ask_for_acceptance() {
    auto payload = create_payload(
            level_,
            (int) participants_.size() + 1
    );

    send_new_message(ALL, Words::MEETING_ACCEPTANCE_REQUEST, payload);
}

void Node::resource_answer(int id) {
    NODE_LOG("Sending resource to %d", id);

    resource_state_ = ResourceState::LOCKED;
    send_new_message(id, Words::RESOURCE_ANSWER);
}

void Node::on_resource_available() {
    NODE_LOG("Resource available, I'm going to invite some friends");

    resource_state_ = ResourceState::LOCKED;  // make sure noone steals our resource while we're awaiting acceptance.
    this->invite_participants();
}

void Node::perhaps_next_answer() {
    NODE_LOG("Perhaps next answer. Count: %d, Queue Size: %d, ResourceState: %d",
             resource_count_, resource_answer_queue_.size(), resource_state_);

    if ((resource_count_ > 0) and (not resource_answer_queue_.empty())) {
        NODE_LOG("I have more resource left and there are people waiting. Sending next");

        auto id = resource_answer_queue_.front();
        resource_answer_queue_.pop(); // lol c++
        resource_answer(id);

    } else {
        NODE_LOG("Resource count: %d, Queue size: %lu", resource_count_, resource_answer_queue_.size());

        resource_state_ = ResourceState::IDLE;
        resource_answer_queue_ = queue<int>();
    }
}

void Node::perhaps_meeting_answer(int process_id) {
    auto retcode = acceptance_queue_.get_answer(acceptor_id_, process_id);

    if (retcode == 1)
        send_new_acceptor_message(process_id, MEETING_ACCEPTANCE_GRANTED);

    if (retcode == -1)
        send_new_acceptor_message(process_id, MEETING_ACCEPTANCE_DENIED);

    if (acceptor_id_ >= 0) {
        NODE_LOG("Meeting answer from acceptor %d: retcode was %d, process ID was %d",
                 acceptor_id_, retcode, process_id);
    }
}

// message handlers
void Node::initialize_mapping() {
    comm_func_map_t[Words::NONE] = &Node::HandleNoneMessage;

    comm_func_map_t[Words::RESOURCE_REQUEST] = &Node::HandleResourceRequest;
    comm_func_map_t[Words::RESOURCE_ANSWER] = &Node::HandleResourceAnswer;
    comm_func_map_t[Words::RESOURCE_ACK] = &Node::HandleResourceAck;
    comm_func_map_t[Words::RESOURCE_DENIED] = &Node::HandleResourceDenial;
    comm_func_map_t[Words::RESOURCE_SENT] = &Node::HandleResourceDelivery;

    comm_func_map_t[Words::MEETING_ACCEPTANCE_REQUEST] = &Node::HandleMeetingAcceptanceRequest;
    comm_func_map_t[Words::MEETING_ACCEPTANCE_GRANTED] = &Node::HandleMeetingAcceptanceGranted;
    comm_func_map_t[Words::MEETING_ACCEPTANCE_DENIED] = &Node::HandleMeetingAcceptanceDenied;
    comm_func_map_t[Words::MEETING_ACCEPTANCE_FULFILLED] = &Node::HandleMeetingAcceptanceFullfilled;
    comm_func_map_t[Words::MEETING_ACCEPTANCE_REPORT] = &Node::HandleMeetingAcceptanceReport;

    comm_func_map_t[Words::ACCEPTOR_PASS_OFFER] = &Node::HandleAcceptorPassOffer;
    comm_func_map_t[Words::ACCEPTOR_PASS_ACK] = &Node::HandleAcceptorPassAck;
    comm_func_map_t[Words::ACCEPTOR_PASS_DENY] = &Node::HandleAcceptorPassDeny;
    comm_func_map_t[Words::ACCEPTOR_PASS_CONFIRM] = &Node::HandleAcceptorPassConfirm;
    comm_func_map_t[Words::ACCEPTOR_PASS_CANCEL] = &Node::HandleAcceptorPassCancel;
    comm_func_map_t[Words::ACCEPTOR_PASS_TEST] = &Node::HandleAcceptorPassTest;
    comm_func_map_t[Words::ACCEPTOR_PASS_RELEASE] = &Node::HandleAcceptorPassRelease;

    comm_func_map_t[Words::MEETING_INVITE] = &Node::HandleMeetingInvitiation;
    comm_func_map_t[Words::MEETING_ACCEPT] = &Node::HandleMeetingInvitationAccept;
    comm_func_map_t[Words::MEETING_DECLINE] = &Node::HandleMeetingInvitationDecline;
    comm_func_map_t[Words::MEETING_CANCEL] = &Node::HandleMeetingCancel;
    comm_func_map_t[Words::MEETING_START] = &Node::HandleMeetingStart;
    comm_func_map_t[Words::MEETING_END] = &Node::HandleMeetingEnd;
    comm_func_map_t[Words::MEETING_DONE] = &Node::HandleMeetingDone;
}

// ----- Invitation Handlers -----
void Node::HandleNoneMessage(__unused Message msg) {
    NODE_LOG("FATAL: UNHANDLED BEHAVIOR");
}

void Node::HandleMeetingInvitationAccept(Message msg) {
    NODE_LOG("Accept from %d", msg.sender);

    this->participants_.insert(msg.sender);
    awaiting_response_ -= 1;
    check_invite_responses();
}

void Node::HandleMeetingInvitationDecline(Message msg) {
    NODE_LOG("Decline from %d", msg.sender);

    awaiting_response_ -= 1;
    check_invite_responses();
}

void Node::HandleMeetingInvitiation(Message msg) {
    if (time_penalty_ > T_) {
        NODE_LOG("Have a time penalty, denying invitation...");

        send_new_message(msg.sender, Words::MEETING_DECLINE);
    }

    if (meeting_state_ == MeetingState::IDLE) {
        NODE_LOG("Accepting invitation!");

        send_new_message(msg.sender, Words::MEETING_ACCEPT);
        meeting_state_ = MeetingState::LOCKED;

    } else {
        NODE_LOG("I'm not idle, sorry, declining...");

        send_new_message(msg.sender, Words::MEETING_DECLINE);
    }
}

void Node::HandleMeetingCancel(__unused Message msg) {
    meeting_state_ = MeetingState::IDLE;
    perhaps_next_answer();
}

void Node::HandleMeetingStart(Message msg) {
    NODE_LOG("Handling meeting start");

    send_new_message(msg.sender, Words::MEETING_DONE);
    this_thread::sleep_for(chrono::seconds(10));
}

void Node::HandleMeetingEnd(__unused Message msg) {
    NODE_LOG("Handling meeting end");

    meeting_state_ = MeetingState::IDLE;
    time_penalty_  = T_ + TIME_PENALTY;
}

void Node::HandleMeetingDone(Message msg) {
    NODE_LOG("Handling meeting done from %d", msg.sender);

    awaiting_confirmations_ -= 1;
    if (awaiting_confirmations_ == 0) {
        NODE_LOG("I have all METTING_DONE messages.");

        for (auto id : participants_) {
            send_new_message(id, MEETING_END);
        }

        resource_state_ = ResourceState::IDLE;
        meeting_state_ = MeetingState::IDLE;
        time_penalty_  = T_ + TIME_PENALTY;

        send_new_message(ALL, MEETING_ACCEPTANCE_FULFILLED);

    }
}

// ----- Resource handling -----
void Node::HandleResourceRequest(Message msg) {
//    NODE_LOG("Handling resource request"); //todo

    if ((resource_count_ > 0) and (resource_state_ == ResourceState::IDLE)) {
        NODE_LOG("Yes, I have an idle resource, responding...");

        resource_answer(msg.sender);
    } else {
//        NODE_LOG("Resource unavailable at the moment. I'll pinHandling resource reques //todo

        resource_answer_queue_.push(msg.sender);
    }
}

void Node::HandleResourceAnswer(Message msg) {
    assert(msg.destination == this->ID_
           && "We are handling a resource answer that is not meant for us...");

    if (resource_state_ == ResourceState::WAITING) {
        NODE_LOG("Accepting a resource from %d", msg.sender);

        resource_state_ = ResourceState::LOCKED;
        send_new_message(msg.sender, Words::RESOURCE_ACK);

    } else {
        NODE_LOG("Denying a resource from %d", msg.sender);

        send_new_message(msg.sender, Words::RESOURCE_DENIED);
    }
}

void Node::HandleResourceAck(Message msg) {
    NODE_LOG("Someone wanted my resource. Sending to %d", msg.sender);

    assert(resource_count_ > 0
           && "Something bad happened: we've unexpectedly lost a resource");

    resource_count_ -= 1;
    resource_state_ = ResourceState::IDLE;
    send_new_message(msg.sender, Words::RESOURCE_SENT);

    perhaps_next_answer();
}

void Node::HandleResourceDenial(__unused Message msg) {
    if (resource_state_ == ResourceState::NEEDED) {
        NODE_LOG("Someone didn't want my resource. I need a resource, so i'll consume it myself");

        resource_state_ = ResourceState::IDLE;
        ask_for_resource();

    } else {
        NODE_LOG("Someone didn't want my resource. I'll just try to answer to any other people waiting for it.");

        resource_state_ = ResourceState::IDLE;
        perhaps_next_answer();
    }
}

void Node::HandleResourceDelivery(__unused Message msg) {
    NODE_LOG("I've got resource delivered");

    resource_count_ += 1;
    on_resource_available();
}

// ----- Meeting acceptance handler -----
// node-side
void Node::HandleMeetingAcceptanceGranted(__unused Message msg) {
    NODE_LOG("Got Acceptance. Sending start token");

    awaiting_confirmations_ = (int) participants_.size();
    for (auto id : participants_) {
        send_new_message(id, MEETING_START);
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
}

// acceptor-side
void Node::HandleMeetingAcceptanceRequest(Message msg) {
    set_trigger(msg);

    int process_lvl = msg.payload[0];
    int n_requested = msg.payload[1];

    if (acceptor_id_ >= 0) {

        NODE_LOG("Acceptor %d handling acceptance request msg from %d, \n"
                         "process lvl: %d   n_requested: %d   sending acceptance report to ALL",
                 acceptor_id_, msg.sender, process_lvl, n_requested);

    }

    acceptance_queue_.perhaps_insert_id(msg.timestamp, msg.sender, process_lvl, n_requested);
    acceptance_queue_.add_response_entry(msg.sender, T_, acceptor_id_, level_); // "response" from self.

    auto payload = create_payload(
            (int) msg.timestamp,
            msg.sender,
            process_lvl,
            n_requested,
            level_,
            acceptor_id_
    );

    send_new_acceptor_message(ALL, MEETING_ACCEPTANCE_REPORT, payload);

    perhaps_meeting_answer(msg.sender);
}

void Node::HandleMeetingAcceptanceReport(Message msg) {
    set_trigger(msg);

    int process_T = msg.payload[0];
    int process_id = msg.payload[1];
    int process_lvl = msg.payload[2];
    int n_requested = msg.payload[3];
    int acceptor_level = msg.payload[4];
    int acceptor_id = msg.payload[5];

    if (acceptor_id_ >= 0) {

        NODE_LOG("Acceptor %d handling Acceptance Report from Acceptor %d\n"
                 "msg.sender: %d, process_T: %d, process_id: %d process_lvl: %d n_requested: %d acceptor_level: %d",
                 acceptor_id_, acceptor_id, msg.sender, process_T, process_id, process_lvl, n_requested, acceptor_level);

    }

    acceptance_queue_.perhaps_insert_id((uint64_t) process_T, process_id, process_lvl, n_requested);
    acceptance_queue_.add_response_entry(process_id, msg.timestamp, acceptor_id, acceptor_level);

    perhaps_meeting_answer(process_id);
}

void Node::HandleMeetingAcceptanceDenied(__unused Message msg) {
    NODE_LOG("I'm cancelling the meeting");

    meeting_cancel();
}

void Node::HandleMeetingAcceptanceFullfilled(Message msg) {
    NODE_LOG("I have a meeting-fulfilled confirmation from %d, deleting it from my queue", msg.sender);

    acceptance_queue_.remove_entry(msg.sender);
}

// ACCEPTOR-PASSING ----------------------------------------------------------------------------------------------------

void Node::initialize_role_transfer() {
    bool time_condition = ((T_ - last_retry_) > RETRY_AWAIT) or (last_retry_ == 0);

    if (acceptor_state == AcceporState::Active and time_condition) {
        last_retry_ = T_;

        NODE_LOG("Initialize role transfer!");

        last_retry_ = T_;

        float p = rand_f();

        int min_level = -1;
        int max_level = 999999;
        int ban_neigh = 0;

        if (p < 0.1) {
            min_level = level_;
        } else if (p < 0.2) {
            max_level = level_;
        } else {
            min_level = level_;
            max_level = level_;
            ban_neigh = 1;
        }

        auto payload = create_payload(
                min_level,
                max_level,
                ban_neigh
        );

        send_new_message(ALL, Words::ACCEPTOR_PASS_OFFER, payload);
    }
}

void Node::set_trigger(Message &msg) {
    trigger_ = identifier(msg);

//    NODE_LOG("Now I'm tracking a message from %l", trigger_);
}

void Node::pass(int sender_id, uint64_t timestamp) {
    NODE_LOG("Passing to %d!", sender_id);

    auto payload = create_payload(
            acceptor_id_
    );

    acceptor_id_    = -1;
    sucessor_id_    = sender_id;
    timestamp_limit = timestamp;
    acceptor_state  = AcceporState::Retired;

    send_new_message(sender_id, Words::ACCEPTOR_PASS_CONFIRM, payload);
}

Message Node::set_acceptor_id(Message msg) {
    if (msg.word == Words::MEETING_ACCEPTANCE_REPORT) {
        msg.payload[5] = acceptor_id_;
    }

    return msg;
}

void Node::perhaps_release() {
    if (acceptor_state != AcceporState::Retired)
        return;

    for (auto &item : timestamps_log_) {
        if (item.second < timestamp_limit) {
            NODE_LOG("Not releasing the successor. Waiting for: %d (T: %lu)", item.first, item.second);

            return;
        }
    }

    NODE_LOG("I'm going to release the sucessor: %d", sucessor_id_);

    acceptor_state = AcceporState::None;
    send_new_message(sucessor_id_, Words::ACCEPTOR_PASS_RELEASE);
}

void Node::HandleAcceptorPassOffer(Message msg) {
//    NODE_LOG("I was offered an acceptor role by: %d", msg.sender);

    if (acceptor_state != AcceporState::None) {
//        NODE_LOG("But my acceptor state is not None, decline");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_DENY);
        return;
    }

    auto min_level = msg.payload[0];
    auto max_level = msg.payload[1];
    auto ban_neigh = (bool) msg.payload[2];

    if ((min_level <= level_) or (level_ > max_level)) {
//        NODE_LOG("But my level is not correct, decline");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_DENY);
        return;
    }

    if (ban_neigh and (neighbours_.find(msg.sender) != neighbours_.end())) {
//        NODE_LOG("But I'm a neighbour, decline");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_DENY);
        return;
    }

    NODE_LOG("I'm going to accept the role");

    acceptor_state = AcceporState::Waiting;
    send_new_message(msg.sender, Words::ACCEPTOR_PASS_ACK);
}

void Node::HandleAcceptorPassAck(Message msg) {
    if (acceptor_state == AcceporState::Active) {
        NODE_LOG("I'm going to pass the acceptor");

        pass(msg.sender, msg.timestamp);
    } else {
        NODE_LOG("I'm going to deny the acceptor ACK signal");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_CANCEL);
    }
}

void Node::HandleAcceptorPassConfirm(Message msg) {
    auto accepor_id = msg.payload[0];

    NODE_LOG("I got the acceptor role. My new ack id: %d", accepor_id);

    acceptance_queue_.replace_id(accepor_id);
    acceptor_id_ = accepor_id;
    acceptor_state = AcceporState::TakingOver;
}

void Node::HandleAcceptorPassRelease(__unused Message msg) {
    NODE_LOG("I'm now a standard acceptor");

    acceptor_state = AcceporState::Active;
}

void Node::HandleAcceptorPassCancel(__unused Message msg) {
//    NODE_LOG("I'm no longer in any acceptor state");

    assert(acceptor_state == AcceporState::Waiting &&
           "We've recieved a cancelling response, but we're not in Waiting state!");

    acceptor_state = AcceporState::None;
}

void Node::HandleAcceptorPassTest(Message msg) {
    int64_t msg_identifier = merge_64(msg.payload[6], msg.payload[7]);

    NODE_LOG("I was asked to test a trigger: %lu", msg_identifier);

    if (unanswered_requests_.find(msg_identifier) != unanswered_requests_.end()) { // todo change this to a call to handle
        for (auto &answer : unanswered_requests_[msg_identifier]) {
            NODE_LOG("Sending a message that was not answered to %d", answer.destination);

            answer = set_acceptor_id(answer);

            auto vec_payload = vector<int>(begin(answer.payload), end(answer.payload));
            send_new_message(answer.destination, answer.word, vec_payload);
        }
    }
}

void Node::HandleAcceptorPassDeny(__unused Message msg) {

}

// done        add logs
// done        make sure sucessor_id_ is set
// done        properly handle acceptor ID filling in HandleMeetingAcceptanceReport and HandleMeetingAcceptanceRequest
// done        change acceptor_id_ to an ID, pass it from the tree root.
// canceled    pass the size of the tree
// done        update the map of timestamps
// done        verify that release makes sense
// done        verify that ack queues make sense
// done        pass all payloads as vectors
// done        move acceptor queue params to `deserialize()` and read parameters from config.
// done        are we even sending the release signal??????
// done        make sure you never pass 64 bit ID_ in a single, 32-bit payload entry
// done        add dynamic timestamp filling
// done        add "last_retry_ now_ retry_await_" functionality
// done        add missing entries in mapping
// done        handle ack pass deny
// done        make sure we never request more than max number of processes to meet
// done        add perhaps_release to event loop
// done        add aceptor_pass to event loop
// done        add proper time penalty
// todo        napisać sparwko
// cancelled   note that we're not losing generality
