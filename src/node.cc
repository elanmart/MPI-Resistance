#include "node.h"
#include "utils.h"

#define MEETING_FREQ 128

//TODO
void Node::debug() {
    NODE_LOG("TODO: FOR DEBUGGING PURPOSES WE'RE MESSING UP THE ACCEPTOR / RESOURCE STUFF");
    if (ID_ == 0) {
        resource_count_ = 1;
        is_acceptor_ = 1;
    } else {
        resource_count_ = 0;
        is_acceptor_ = 0;
    }
}

// ctors
Node::Node() {
    T_ = 0;
    level_ = 0;
    parent_ = -1;
    msg_number_ = 0;
    is_acceptor_ = 0;
    resource_count_ = 0;
    operation_number_ = 0;
    acceptance_queue_ = AcceptorQueue(3, 3);
    unanswered_requests_ = map<int64_t, vector<Message>>();
    awaiting_confirmations_ = 0;
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
    debug();

    Message msg;
    manager_->start();

    while (not STOP_) {

        if (this->get(&msg))
            consume(msg);
        else if ((rand() % MEETING_FREQ == 0) and (meeting_state_ == MeetingState::IDLE))
            initialize_meeting_procedure();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// message passing
bool Node::get(Message *msg) {
    auto msg_available = manager_->get(msg);

    if (msg_available)
        T_ = max(msg->timestamp, T_);

    return msg_available;
}

void Node::send_new_message(int destination, Words w, int *payload) {
    msg_number_ += 1;

    auto msg = create_message(msg_number_, ID_, destination, T_, w, payload);
    msg_cache_.insert(identifier(msg));

    broadcast(msg);
}

void Node::send_new_acceptor_message(int destination, Words w, int *payload) {
    if (acceptor_state == AcceporState::Active or acceptor_state == AcceporState::TakingOver) {
        send_new_message(destination, w, payload);

    } else if (acceptor_state == AcceporState::Retired) {
        auto ID = split_64(trigger_);
        payload[6] = ID.first;
        payload[7] = ID.second;

        send_new_message(sucessor_id_, Words::ACCEPTOR_PASS_TEST, payload);
        perhaps_release();

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
    NODE_LOG("handle triggered for: %s %d", EnumStrings[msg.word], msg.word);

    auto handler = comm_func_map_t[msg.word];
    return (this->*handler)(msg);
}

// Serialization
pair<int, int *> Node::serialize() {
    auto n_children = (int) children_.size();
    auto n_neighbours = (int) neighbours_.size();

    int n_items = 5 + 2 + n_children + n_neighbours;
    auto *buffer = new int[n_items];

    buffer[0] = ID_;
    buffer[1] = level_;
    buffer[2] = parent_;
    buffer[3] = resource_count_;
    buffer[4] = is_acceptor_;
    buffer[5] = n_children;
    buffer[6] = n_neighbours;
    int offset = 7;

    for (auto item : children_)
        buffer[offset++] = item;

    for (auto item : neighbours_)
        buffer[offset++] = item;

    return make_pair(n_items, buffer);
}

void Node::deserialize(int *buffer) {
    ID_ = buffer[0];
    level_ = buffer[1];
    parent_ = buffer[2];
    resource_count_ = buffer[3];
    is_acceptor_ = buffer[4];
    int n_children = buffer[5];
    int n_neighbours = buffer[6];
    int offset = 7;

    children_.clear();
    neighbours_.clear();

    for (int i = offset; i < offset + n_children; i++)
        children_.insert(buffer[i]);

    offset += n_children;
    for (int i = offset; i < offset + n_neighbours; i++)
        neighbours_.insert(buffer[i]);
}

// meetings
void Node::initialize_meeting_procedure() {
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

    this->invitees_.clear();
    this->invitees_.insert(children_.begin(), children_.end());
    this->invitees_.insert(neighbours_.begin(), neighbours_.end());
    if (parent_ != -1) {
        this->invitees_.insert(parent_);
    }

    awaiting_response_ = (int) (this->invitees_.size());

    for (auto id : this->invitees_) {
        send_new_message(id, MEETING_INVITE);
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

    bool enough_participants = (participants_.size() >= floor(this->invitees_.size() * percentage_threshold_));

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
            (int) participants_.size()
    ).data();

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
    auto retcode = acceptance_queue_.get_answer(ID_, process_id);

    if (retcode == 1)
        send_new_acceptor_message(process_id, MEETING_ACCEPTANCE_GRANTED);

    if (retcode == -1)
        send_new_acceptor_message(process_id, MEETING_ACCEPTANCE_DENIED);
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
void Node::HandleNoneMessage(__unsued Message msg) {
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
    if (time_penalty_ >= T_) {
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

void Node::HandleMeetingCancel(__unsued Message msg) {
    meeting_state_ = MeetingState::IDLE;
    perhaps_next_answer();
}

void Node::HandleMeetingStart(Message msg) {
    NODE_LOG("Handling meeting start");

    send_new_message(msg.sender, Words::MEETING_DONE);
    this_thread::sleep_for(chrono::seconds(10));
}

void Node::HandleMeetingEnd(__unsued Message msg) {
    NODE_LOG("Handling meeting end");

    meeting_state_ = MeetingState::IDLE;
}

void Node::HandleMeetingDone(Message msg) {
    NODE_LOG("Handling meeting done from %d", msg.sender);

    awaiting_confirmations_ -= 1;
    if (awaiting_confirmations_ == 0) {
        NODE_LOG("I have all METTING_DONE messages.");

        for (auto id : participants_) {
            send_new_message(id, MEETING_END);
        }

        meeting_state_ = MeetingState::IDLE;

        send_new_message(ALL, MEETING_ACCEPTANCE_FULFILLED);
    }
}

// ----- Resource handling -----
void Node::HandleResourceRequest(Message msg) {
    NODE_LOG("Handling resource request");

    if ((resource_count_ > 0) and (resource_state_ == ResourceState::IDLE)) {
        NODE_LOG("Yes, I have an idle resource, responding...");

        resource_answer(msg.sender);
    } else {
        NODE_LOG("Resource unavailable at the moment. I'll ping you when I get one. State: %d, Count: %d",
                 resource_state_, resource_count_);

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

void Node::HandleResourceDenial(__unsued Message msg) {
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

void Node::HandleResourceDelivery(__unsued Message msg) {
    NODE_LOG("I've got resource delivered");

    resource_count_ += 1;
    on_resource_available();
}

// ----- Meeting acceptance handler -----
// node-side
void Node::HandleMeetingAcceptanceGranted(__unsued Message msg) {
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

    NODE_LOG("handling acceptance request msg from %d ,process lvl: %d n_requested: %d, sending acceptance report",
             msg.sender, process_lvl, n_requested);

    acceptance_queue_.perhaps_insert_id(msg.timestamp, msg.sender, process_lvl, n_requested);

    auto payload = create_payload(
            (int) msg.timestamp,
            msg.sender,
            process_lvl,
            n_requested,
            level_
    ).data();

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

    NODE_LOG("Aceptance Report received.\n"
             "sender: %d, process_T: %d, process_id: %d process_lvl: %d n_requested: %d acceptor_level: %d",
             msg.sender, process_T, process_id, process_lvl, n_requested, acceptor_level);

    acceptance_queue_.add_response_entry((uint64_t) process_T, process_id, process_lvl, n_requested,
                                         msg.timestamp, acceptor_id, acceptor_level);

    perhaps_meeting_answer(process_id);

}

void Node::HandleMeetingAcceptanceDenied(__unsued Message msg) {
    NODE_LOG("I'm cancelling the meeting");

    meeting_cancel();
}

void Node::HandleMeetingAcceptanceFullfilled(Message msg) {
    NODE_LOG("I have a meeting-fulfilled confirmation from %d, deleting it from my queue", msg.sender);

    acceptance_queue_.remove_entry(msg.sender);
}

// ACCEPTOR-PASSING ----------------------------------------------------------------------------------------------------

void Node::initialize_role_transfer() {
    if (acceptor_state == AcceporState::Active and (last_retry_ - now_) < retry_await_) {
        NODE_LOG("Initialize role transfer!");

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
        ).data();

        send_new_message(ALL, Words::ACCEPTOR_PASS_OFFER, payload);
    }
}

void Node::set_trigger(Message &msg) {
    trigger_ = identifier(msg);

    NODE_LOG("Now I'm tracking a message from %l", trigger_);
}

void Node::pass(int sender_id) {
    NODE_LOG("Passing to %d!", sender_id);

    auto payload = create_payload(
            is_acceptor_
    );

    is_acceptor_ = -1;
    sucessor_id_ = sender_id;
    acceptor_state = AcceporState::Retired;

    send_new_message(sender_id, Words::ACCEPTOR_PASS_CONFIRM, payload.data());
}

Message Node::set_acceptor_id(Message msg) {

}

void Node::perhaps_release() {
    if (acceptor_state != AcceporState::Retired)
        return;

    for (auto &item : timestamps_log_) {
        if (item.second < timestamp_limit)
            return;
    }

    NODE_LOG("I'm going to release the sucessor: %d", sucessor_id_);

    acceptor_state = AcceporState::None;
    send_new_message(sucessor_id_, Words::ACCEPTOR_PASS_RELEASE);
}

void Node::HandleAcceptorPassOffer(Message msg) {
    NODE_LOG("I was offered an acceptor role by: %d", msg.sender);

    if (acceptor_state != AcceporState::None) {
        NODE_LOG("But my acceptor state is not None, decline");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_DENY);
        return;
    }

    auto min_level = msg.payload[0];
    auto max_level = msg.payload[1];
    auto ban_neigh = (bool) msg.payload[2];

    if ((min_level <= level_) or (level_ > max_level)) {
        NODE_LOG("But my level is not correct, decline");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_DENY);
        return;
    }

    if (ban_neigh and (neighbours_.find(msg.sender) != neighbours_.end())) {
        NODE_LOG("But I'm a neighbour, decline");

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

        pass(msg.sender);
    } else {
        NODE_LOG("I'm going to deny the acceptor ACK signal");

        send_new_message(msg.sender, Words::ACCEPTOR_PASS_CANCEL);
    }
}

void Node::HandleAcceptorPassConfirm(Message msg) {
    auto accepor_id = msg.payload[0];

    NODE_LOG("I got the acceptor role. My new ack id: %d", accepor_id);

    is_acceptor_ = accepor_id;
    acceptor_state = AcceporState::TakingOver;
}

void Node::HandleAcceptorPassRelease(__unsued Message msg) {
    NODE_LOG("I'm now a standard acceptor");

    acceptor_state = AcceporState::Active;
}

void Node::HandleAcceptorPassCancel(__unsued Message msg) {
    NODE_LOG("I'm no longer in any acceptor state");

    assert(acceptor_state == AcceporState::Waiting &&
           "We've recieved a cancelling response, but we're not in Waiting state!");

    acceptor_state = AcceporState::None;
}

void Node::HandleAcceptorPassTest(Message msg) {
    int64_t ID = merge_64(msg.payload[6], msg.payload[7]);

    NODE_LOG("I was asked to test a trigger: %lu", ID);

    if (unanswered_requests_.find(ID) != unanswered_requests_.end()) {
        for (auto &answer : unanswered_requests_[ID]) {
            send_new_message(answer.destination, answer.word, answer.payload);
            NODE_LOG("Sending a message that was not answered to %d", answer.destination);
        }
    }
}

void Node::HandleAcceptorPassDeny(__unsued Message msg) {
    NODE_LOG("I've received an acceptor pass denial");
}

//done add logs
//done make sure sucessor_id_ is set
//todo properly handle acceptor ID filling in HandleMeetingAcceptanceReport and HandleMeetingAcceptanceRequest
//todo change is_acceptor_ to an ID, pass it from the tree root.
//todo pass the size of the tree
//todo update the map of timestamps
//todo verify that release makes sense
//todo verify that ack queues make sense
//todo pass all payloads as vectors
//todo move acceptor queue params to `deserialize()` and read parameters from config.
//todo are we even sending the release signal??????
//todo make sure you never pass 64 bit ID_ in a single, 32-bit payload entry
//todo add dynamic timestamp filling
//todo add "last_retry_ now_ retry_await_" functionality
//todo add missing entries in mapping
//todo handle ack pass deny
//todo note that we're not losing generality
//todo napisać sparwko
//todo add perhaps_release to event loop
//todo add aceptor_pass to event loop
//todo add proper time penalty