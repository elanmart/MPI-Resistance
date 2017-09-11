#include "node.h"
#include "utils.h"

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
   return manager_->get(msg);
}


void Node::send_to(Message msg, int dest) {
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
   Message msg;
   manager_->start();

   while (not STOP_) {

      if (this->get(&msg))
         consume(msg);
      else if (meeting_state_ == MeetingState::IDLE) {
         if (rand() % 3 == 0) {
            LOG("Let's organize a meeting!");
            meeting_state_ = MeetingState::MASTER_ORG;

            ask_for_resource();
            sleep(1);
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
   LOG("%s", EnumStrings[msg.word]);
   return (this->*fp)(msg);
}

//
// --- Abilities ---
//
void Node::invite_participants() {
   if (this->meeting_state_ == MeetingState::MASTER_ORG) {
      this->invitees_.insert(children_.begin(), children_.end());
      this->invitees_.insert(neighbours_.begin(), neighbours_.end());
      this->invitees_.insert(parent_);

      awaiting_response_ = this->invitees_.size();

      LOG("Inviting participants... Count: %d", awaiting_response_);

      for (auto id : this->invitees_) {
         send_new_message(id, MEETING_INVITE);
      }
   } else {
      LOG("Meeting state not idle!");
   }
}

void Node::ask_for_resource() {
   if (resource_count_ == 0 && resource_state_ != ResourceState::WAITING) {
      LOG("I need resource. Please propagate this to everyone!");
      resource_state_ = ResourceState::WAITING;
      send_new_message(ALL, Words::RESOURCE_REQUEST);
   } else if (resource_count_ > 0 && resource_state_ != ResourceState::IDLE) {
      LOG("I've got resource, no need to ask.");
      ask_for_acceptance();
   }
}

void Node::ask_for_acceptance() {
   if (parent_) {
      LOG("Asking for acceptance");
      send_new_message(parent_, MEETING_ACCEPTANCE_REQUEST);
   } else {
      LOG("I'm tree master, accepting!");
      meet();
   }
}

void Node::try_start_meeting() {
   if (awaiting_response_ == 0) {
      LOG("Starting meeting...");

      if (participants_.size() >= floor(this->invitees_.size() * percentage_threshold_)) {
         LOG("Sending invitations...");
         // Send message about started meeting
         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_START);
         }

         sleep(1);

         LOG("Meeting is over!");

         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_END);
         }
      } else {
         LOG("Not enough participants to start meeting, canceling...");

         for (auto id : this->invitees_) {
            send_new_message(id, MEETING_CANCEL);
         }
      }
   } else {
      LOG("Awaiting response from %d invitees...", awaiting_response_);
   }
}

void Node::meet() {
   LOG("Meet!");
   resource_state_ = ResourceState::LOCKED;
   this->invite_participants();
}

//
// ----- Invitation Handlers -----
//
void Node::HandleNoneMessage(Message msg) {
   LOG("FATAL: UNHANDLED BEHAVIOR");
}

void Node::HandleMeetingInvitationAccept(Message msg) {
   this->participants_.insert(msg.sender);
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingInvitationDecline(Message msg) {
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingInvitiation(Message msg) {
   if (time_penalty_ > 0) {
      LOG("Have a time penalty, denying invitation...");
      send_new_message(msg.sender, Words::MEETING_DECLINE);
   }

   if (meeting_state_ == MeetingState::IDLE) {
      LOG("Accepting invitation!");
      send_new_message(msg.sender, Words::MEETING_ACCEPT);
      meeting_state_ = MeetingState::WAITING;
   } else {
      LOG("I'm not idle, sorry, declining...");
      send_new_message(msg.sender, Words::MEETING_DECLINE);
   }
}

void Node::HandleMeetingCancel(Message msg) {
   meeting_state_ = MeetingState::IDLE;
}

void Node::HandleMeetingStart(Message msg) {
   meeting_state_ = MeetingState::LOCKED;
}

void Node::HandleMeetingEnd(Message msg) {
   meeting_state_ = MeetingState::IDLE;
}

//
// ----- Resource handling
//
void Node::HandleResourceRequest(Message msg) {
   if (resource_count_ > 0) {
      LOG("Handling resource request");

      if (resource_state_ == ResourceState::LOCKED) {
         //todo: put to a queue of awaiting requests.
      } else if (resource_state_ == ResourceState::IDLE) {
         resource_state_ = ResourceState::LOCKED;
         send_new_message(msg.sender, Words::RESOURCE_ANSWER);

         LOG("Yes, I have idle resource, responding...");
      }
   } else {
      LOG("Don't have resource, sorry, forwarding");

      set<int> fwds;
      fwds.insert(neighbours_.begin(), neighbours_.end());
      fwds.insert(parent_);

      for (auto id : fwds) {
         forward(msg, id);
      }
   }
}

void Node::HandleResourceAnswer(Message msg) {
   if (resource_state_ == ResourceState::WAITING) {
//      resource_state_ = ResourceState::WAITING;
      send_new_message(msg.sender, Words::RESOURCE_ACK);
      LOG("Accepted a resource of %d", msg.sender);
   } else {
      send_new_message(msg.sender, Words::RESOURCE_DENIED);
      LOG("Denied a resource of %d", msg.sender);
   }
}

void Node::HandleResourceAck(Message msg) {
   resource_count_ -= 1;
   send_new_message(msg.sender, Words::RESOURCE_SENT);
   LOG("Someone wanted my resource. Sent");
}

void Node::HandleResourceDenial(Message msg) {
   resource_state_ = ResourceState::IDLE;
   LOG("Someone didn't want my resource.");
}

void Node::HandleResourceDelivery(Message msg) {
   resource_count_ += 1;
   ask_for_acceptance();
}

//
// ----- Meeting acceptance handler
//
void Node::HandleMeetingAcceptanceRequest(Message msg) {
   if (is_acceptor_) {
      LOG("Acceptor here, accepting!");
      send_new_message(msg.sender, MEETING_ACCEPTANCE_ANSWER);
   } else {
      set<int> fwds;
      fwds.insert(neighbours_.begin(), neighbours_.end());
      fwds.insert(parent_);

      LOG("Forwarding acceptance request...");

      for (auto id : fwds) {
         forward(msg, id);
      }
   }
}

void Node::HandleMeetingAcceptanceAnswer(Message msg) {
   LOG("Got acceptance!");
   meet();
}

void Node::HandleMeetingAcceptanceAck(Message msg) {

}

void Node::HandleMeetingAcceptanceDenial(Message msg) {
   LOG("Acceptor denied meeting, cancelling...");
   meeting_state_ = MeetingState::IDLE;
   resource_state_ = ResourceState::IDLE;
}

// TODO - whole behavior
void Node::HandleMeetingAcceptanceDelivery(Message msg) {

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