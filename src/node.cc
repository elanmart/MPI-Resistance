#include "node.h"

// --- ctors ---

Node::Node() {
   level_ = 0;
   parent_ = -1;
   msg_number_ = 0;
   resource_count_ = 0;
   is_acceptor_ = rand() % 10 == 0; //10% chance to become acceptor
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

void Node::initialize_mapping() {
   comm_func_map_t[Words::NONE] = &Node::HandleNoneMessage;

   comm_func_map_t[Words::RESOURCE_REQUEST] = &Node::HandleResourceRequest;
   comm_func_map_t[Words::RESOURCE_ANSWER] = &Node::HandleResourceAnswer;
   comm_func_map_t[Words::RESOURCE_ACK] = &Node::HandleResourceAck;
   comm_func_map_t[Words::RESOURCE_DENIED] = &Node::HandleResourceDenial;
   comm_func_map_t[Words::RESOURCE_SENT] = &Node::HandleResourceDelivery;

   comm_func_map_t[Words::MEETING_ACCEPTANCE_REQUEST] = &Node::HandleMeetingAcceptanceRequest; // TODO
   comm_func_map_t[Words::MEETING_ACCEPTANCE_ANSWER] = &Node::HandleMeetingAcceptanceAnswer; // TODO
   comm_func_map_t[Words::MEETING_ACCEPTANCE_ACK] = &Node::HandleMeetingAcceptanceAck; // TODO
   comm_func_map_t[Words::MEETING_ACCEPTANCE_DENIED] = &Node::HandleMeetingAcceptanceDenial; // TODO
   comm_func_map_t[Words::MEETING_ACCEPTANCE_SENT] = &Node::HandleMeetingAcceptanceDelivery; // TODO

   comm_func_map_t[Words::MEETING_INVITE] = &Node::HandleMeetingInvitiation;
   comm_func_map_t[Words::MEETING_ACCEPT] = &Node::HandleMeetingInvitationAccept;
   comm_func_map_t[Words::MEETING_DECLINE] = &Node::HandleMeetingInvitationDecline;
   comm_func_map_t[Words::MEETING_CANCEL] = &Node::HandleMeetingCancel;
   comm_func_map_t[Words::MEETING_START] = &Node::HandleMeetingStart;
   comm_func_map_t[Words::MEETING_END] = &Node::HandleMeetingEnd;

   return;
}

void Node::set_manager(Manager *m) {
   manager_ = m;
}

//
// --- Communication Helper Functions ---
//
void Node::send_new_message(int destination, Words w, int *payload) {
   msg_number_++;
   auto msg = create_message(msg_number_, ID_, destination, w, payload);

   _send(msg);
}


void Node::_send(Message msg) {
   msg_cache_.insert(identifier(msg));
   broadcast(msg);
}

bool Node::get(Message *msg) {
   auto msg_available = manager_->get(msg);
   
   if (msg_available)
      T_ = max(msg->timestamp, T_);

   return msg_available;
}


void Node::send_to(Message msg, int dest) {
   T_ += 1;
   msg.timestamp = T_;

   manager_->put(msg, dest);
}

void Node::send_to(Message msg, set<int> recipients) {
   for (auto id : recipients)
      send_to(msg, id);
}

void Node::broadcast(Message msg) {
   send_to(msg, parent_);
   send_to(msg, children_);
   send_to(msg, neighbours_);
}

void Node::forward(Message msg, int target) {
   auto new_msg = create_message(msg.number, msg.sender, target, msg.word, msg.payload);

   send_to(new_msg, target);
}

//
// --- Core Logic ---
//
void Node::start_event_loop() {
   NODE_LOG("TODO: FOR DEBUGGING PURPOSES WE'RE MESSING UP THE ACCEPTOR / RESOURCE STUFF");
   if (ID_ == 0) {
      resource_count_ = 1;
      is_acceptor_ = 1;
   } else {
      resource_count_ = 0;
      is_acceptor_ = 0;
   }

   Message msg;
   manager_->start();

   while (not STOP_) {

      if (this->get(&msg)) {
         consume(msg);
      } else if (meeting_state_ == MeetingState::IDLE) {
         if (rand() % 3 == 0) {
            initialize_meeting_procedure();
         }
      }
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

bool Node::accept(Message &msg) {
   auto key = identifier(msg);

   if (msg_cache_.find(key) != msg_cache_.end()) {
      return false;
   } else {
      msg_cache_.insert(key);
      return true;
   }
}

void Node::handle(Message msg) {
   comm_method fp = this->comm_func_map_t[msg.word];
   NODE_LOG("handle triggered for: %s", EnumStrings[msg.word]);
   return (this->*fp)(msg);
}

//
// --- Abilities ---
//
void Node::initialize_meeting_procedure() {
   NODE_LOG("Let's organize a meeting!");
   meeting_state_ = MeetingState::MASTER_ORG;

   ask_for_resource();
   sleep(5);
}

void Node::invite_participants() {
   NODE_LOG("Inviting participants");

   if (this->meeting_state_ == MeetingState::MASTER_ORG) {
      this->invitees_.insert(children_.begin(), children_.end());
      this->invitees_.insert(neighbours_.begin(), neighbours_.end());
      this->invitees_.insert(parent_);

      awaiting_response_ = (int) (this->invitees_.size());

      NODE_LOG("Inviting participants... Count: %d", awaiting_response_);

      for (auto id : this->invitees_) {
         send_new_message(id, MEETING_INVITE);
      }
   } else {
      NODE_LOG("Meeting state not idle!");
   }
}

void Node::ask_for_resource() {

   assert(resource_state_ != ResourceState::WAITING
          && "Resource requested in an invalid state. Make sure your in IDLE if you want to organize a meeting");

   if (resource_count_ == 0) {

      NODE_LOG("I need resource. Please propagate this to everyone!");
      resource_state_ = ResourceState::WAITING;
      send_new_message(ALL, Words::RESOURCE_REQUEST);

   } else if (resource_count_ > 0) {

      if (resource_state_ == ResourceState::IDLE) {
         NODE_LOG("I've got resource, no need to ask.");
         on_resource_available();

      } else {
         NODE_LOG("I've got resource, But it's locked. We'll handle this later.");
         resource_state_ = ResourceState::NEEDED;
      }
   }
}

void Node::ask_for_acceptance() {
   if (parent_) {
      NODE_LOG("Asking for acceptance");
      send_new_message(parent_, Words::MEETING_ACCEPTANCE_REQUEST);
   } else {
      NODE_LOG("I'm tree master, accepting!");
      meet();
   }
}

void Node::on_resource_available() {
   resource_state_ = ResourceState::LOCKED;  // make sure noone steals our resource while we're awaiting acceptance.

   ask_for_acceptance();
}

void Node::try_start_meeting() {
   if (awaiting_response_ == 0) {
      NODE_LOG("Starting meeting...");

      if (participants_.size() >= floor(this->invitees_.size() * percentage_threshold_)) {
         NODE_LOG("Sending invitations...");
         // Send message about started meeting
         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_START);
         }

         sleep(5);

         NODE_LOG("Meeting is over!");

         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_END);
         }
      } else {
         NODE_LOG("Not enough participants to start meeting, canceling...");

         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_CANCEL);
         }
      }
   } else {
      NODE_LOG("Awaiting response from %d invitees...", awaiting_response_);
   }
}

void Node::meet() {
   NODE_LOG("Meet!");
   this->invite_participants();
}

void Node::resource_answer(int id) {
   NODE_LOG("Sending resource to %d", id);

   resource_state_ = ResourceState::LOCKED;
   send_new_message(id, Words::RESOURCE_ANSWER);
}

void Node::perhaps_next_answer() {
   if ((resource_count_ > 0) and (resource_answer_queue_.size() > 0)) {
      NODE_LOG("I have more resource left and there are people waiting. Sending next");

      auto id = resource_answer_queue_.front();
      resource_answer_queue_.pop(); // lol c++
      resource_answer(id);
   }
}

//
// ----- Invitation Handlers -----
//
void Node::HandleNoneMessage(__unsued Message msg) {
   NODE_LOG("FATAL: UNHANDLED BEHAVIOR");
}

void Node::HandleMeetingInvitationAccept(Message msg) {
   this->participants_.insert(msg.sender);
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingInvitationDecline(__unsued Message msg) {
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingInvitiation(Message msg) {
   if (time_penalty_ > 0) {
      NODE_LOG("Have a time penalty, denying invitation...");
      send_new_message(msg.sender, Words::MEETING_DECLINE);
   }

   if (meeting_state_ == MeetingState::IDLE) {
      NODE_LOG("Accepting invitation!");
      send_new_message(msg.sender, Words::MEETING_ACCEPT);
      meeting_state_ = MeetingState::WAITING;
   } else {
      NODE_LOG("I'm not idle, sorry, declining...");
      send_new_message(msg.sender, Words::MEETING_DECLINE);
   }
}

void Node::HandleMeetingCancel(__unsued Message msg) {
   meeting_state_ = MeetingState::IDLE;
}

void Node::HandleMeetingStart(__unsued Message msg) {
   meeting_state_ = MeetingState::LOCKED;
}

void Node::HandleMeetingEnd(__unsued Message msg) {
   meeting_state_ = MeetingState::IDLE;
}

//
// ----- Resource handling
//
void Node::HandleResourceRequest(Message msg) {
   NODE_LOG("Handling resource request");

   if ((resource_count_ > 0) and (resource_state_ == ResourceState::IDLE)) {
      NODE_LOG("Yes, I have idle resource, responding...");
      resource_answer(msg.sender);
   } else {
      NODE_LOG("Resource unavailable at the moment. I'll ping you when I get one");

      if (resource_state_ == ResourceState::LOCKED)
         NODE_LOG("I've got a LOCKED resource");
      if (resource_state_ == ResourceState::NEEDED)
         NODE_LOG("I've got a NEEDED resource");
      if (resource_state_ == ResourceState::WAITING)
         NODE_LOG("I'm waiting for resource myself");
      if (resource_count_ == 0)
         NODE_LOG("I dont have any resource");

      resource_answer_queue_.push(msg.sender);
   }
}

void Node::HandleResourceAnswer(Message msg) {
   assert(msg.destination == this->ID_
          && "We are handling a resource answer that is not meant for us...");

   if (resource_state_ == ResourceState::WAITING) {
      NODE_LOG("Accepted a resource from %d", msg.sender);
      resource_state_ = ResourceState::LOCKED;
      send_new_message(msg.sender, Words::RESOURCE_ACK);

   } else {
      NODE_LOG("Denied a resource from %d", msg.sender);
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
   NODE_LOG("Someone didn't want my resource.");

   if (resource_state_ == ResourceState::NEEDED) {
      NODE_LOG("I need a resource, so i'll consume it myself");

      resource_state_ = ResourceState::IDLE;
      ask_for_resource();

   } else {
      NODE_LOG("I'll just try to asnwer to any other people waiting for the resource");

      resource_state_ = ResourceState::IDLE;
      perhaps_next_answer();
   }
}

void Node::HandleResourceDelivery(__unsued Message msg) {
   resource_count_ += 1;

   on_resource_available();
}

//
// ----- Meeting acceptance handler
//
void Node::HandleMeetingAcceptanceRequest(Message msg) {
   if (is_acceptor_) {
      NODE_LOG("Acceptor here, accepting!");
      send_new_message(msg.sender, MEETING_ACCEPTANCE_ANSWER);
   } else {
      set<int> fwds;
      fwds.insert(neighbours_.begin(), neighbours_.end());
      fwds.insert(parent_);

      NODE_LOG("Forwarding acceptance request...");

      for (auto id : fwds) {
         forward(msg, id);
      }
   }
}

void Node::HandleMeetingAcceptanceAnswer(__unsued Message msg) {
   NODE_LOG("Got acceptance!");
   meet();
}

void Node::HandleMeetingAcceptanceAck(__unsued Message msg) {

}

void Node::HandleMeetingAcceptanceDenial(__unsued Message msg) {
   NODE_LOG("Acceptor denied meeting, cancelling...");
   meeting_state_ = MeetingState::IDLE;
   resource_state_ = ResourceState::IDLE;
}

// TODO - whole behavior
void Node::HandleMeetingAcceptanceDelivery(__unsued Message msg) {

}

//
// --- Serialization Helpers
//
pair<int, int *> Node::serialize() {
   int n_children = (int) children_.size();
   int n_neighbours = (int) neighbours_.size();

   int n_items = 4 + 2 + n_children + n_neighbours;
   int *buffer = new int[n_items];

   buffer[0] = ID_;
   buffer[1] = level_;
   buffer[2] = parent_;
   buffer[3] = resource_count_;
   buffer[4] = n_children;
   buffer[5] = n_neighbours;
   int offset = 6;

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
   int n_children = buffer[4];
   int n_neighbours = buffer[5];
   int offset = 6;

   children_.clear();
   neighbours_.clear();

   for (int i = offset; i < offset + n_children; i++)
      children_.insert(buffer[i]);

   offset += n_children;
   for (int i = offset; i < offset + n_neighbours; i++)
      neighbours_.insert(buffer[i]);
}