#include "node.h"
#include "utils.h"

// --- ctors ---

Node::Node() {
   level_ = 0;
   parent_ = -1;
   msg_number_ = 0;
   resource_count_ = 0;
   resource_state_ = ResourceState::IDLE;
   meeting_state_ = MeetingState::IDLE;
   STOP_ = false;
   initialize_mapping();
}

Node::Node(int ID) : Node() {
   ID_ = ID;
   initialize_mapping();
}

Node::Node(int *buffer) : Node() {
   deserialize(buffer);
   initialize_mapping();
}

Node::initialize_mapping() {
   mapping[Words::NONE] = &Node::HandleNoneMessage;

   mapping[Words::RESOURCE_REQUEST] = &Node::HandleResourceRequest;
   mapping[Words::RESOURCE_ANSWER] = &Node::HandleResourceAnswer;
   mapping[Words::RESOURCE_ACK] = &Node::HandleResourceAck;
   mapping[Words::RESOURCE_DENIED] = &Node::HandleResourceDenial;
   mapping[Words::RESOURCE_SENT] = &Node::HandleResourceDelivery;

   mapping[Words::MEETING_ACCEPTANCE_REQUEST] = &Node::HandleMeetingAcceptanceRequest; // TODO
   mapping[Words::MEETING_ACCEPTANCE_ANSWER] = &Node::HandleMeetingAcceptanceAnswer; // TODO
   mapping[Words::MEETING_ACCEPTANCE_ACK] = &Node::HandleMeetingAcceptanceAck; // TODO
   mapping[Words::MEETING_ACCEPTANCE_DENIED] = &Node::HandleMeetingAcceptanceDenial; // TODO
   mapping[Words::MEETING_ACCEPTANCE_SENT] = &Node::HandleMeetingAcceptanceDelivery; // TODO

   mapping[Words::MEETING_INVITE] = &Node::HandleMeetingInvitiation;
   mapping[Words::MEETING_ACCEPT] = &Node::HandleMeetingAccept;
   mapping[Words::MEETING_DECLINE] = &Node::HandleMeetingDecline;
   mapping[Words::MEETING_CANCEL] = &Node::HandleMeetingCancel;
   mapping[Words::MEETING_START] = &Node::HandleMeetingStart;


   // TODO - Do we need these?
   mapping[Words::MEETING_ORG_ACCEPT] = &Node::HandleNoneMessage;
   mapping[Words::MEETING_NEW_ORG_PROBE] = &Node::HandleNoneMessage;
   mapping[Words::MEETING_NEW_ORG] = &Node::HandleNoneMessage;
   mapping[Words::MEETING_PARTC_ACK] = &Node::HandleNoneMessage;
}

void Node::set_manager(Manager *m) {
   manager_ = m;
}

//
// --- Communication Helper Functions ---
//
void Node::new_message(int destination, Words w, int payload[]) {
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

//
// --- Core Logic ---
//
void Node::start_event_loop() {
   Message msg;
   manager_->start();

   while (not STOP_) {

      if (this->get(&msg))
         consume(msg);

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
   comm_func_map_t::iterator x = this->mapping.find(msg.word);

   if (x != mapping.end()) {
      (*(x->second))(msg);
   }
}

//
// --- Abilities ---
//
void Node::invite_participants() {
   if (this->meeting_state_ == MeetingState::IDLE) {
      LOG("Inviting participants...");

      this->meeting_state_ = MeetingState::MASTER_ORG;

      this->invitees_.insert(children_.begin(), children_.end());
      this->invitees_.insert(neighbours_.begin(), neighbours_.end());
      this->invitees_.insert(parent_);

      for (auto id : this->invitees_) {
         new_message(id, MEETING_INVITE);
      }
   }
}

void Node::ask_for_resource() {
   assert(resource_count_ == 0);
   LOG("I need resource. Please propagate this to everyone!");
   new_message(ALL, Words::RESOURCE_REQUEST);
}

void Node::try_start_meeting() {
   if (awaiting_response_ == 0) {
      LOG("Starting meeting...");

      if (participants_.size() >= floor(this->invitees_.size() * percentage_threshold_)) {
         LOG("Sending invitations...");
         // Send message about started meeting
         for (auto id : this->invitees_) {
            new_message(id, MEETING_START);
         }
      } else {
         LOG("Not enough participants to start meeting, canceling...");

         for (auto id : this->invitees_) {
            new_message(id, MEETING_CANCEL);
         }
      }
   }
}

void Node::meet() {
   LOG("Received resource, let's invite guests.");
   resource_state_ = ResourceState::IDLE;
   this->invite_participants();
}

//
// ----- Invitation Handlers -----
//
void Node::HandleNoneMessage(Message msg) {
   LOG("FATAL: UNHANDLED BEHAVIOR");
}

void Node::HandleMeetingAccept(Message msg) {
   this->participants_.insert(msg.sender);
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingDecline(Message msg) {
   awaiting_response_ -= 1;
   try_start_meeting();
}

void Node::HandleMeetingInvitiation(Message msg) {
   if (time_penalty_ > 0)
      new_message(msg.sender, Words::MEETING_DECLINE);

   if (meeting_state_ == MeetingState::IDLE) {
      new_message(msg.sender, Words::MEETING_ACCEPT);
      meeting_state_ = MeetingState::WAITING;
   }

   if (meeting_state_ == MeetingState::MASTER_ORG) {
      meeting_state_ = MeetingState::SLAVE_ORG;
   }
}

void Node::HandleMeetingCancel(Message msg) {
   meeting_state_ = MeetingState::IDLE;
   this->invitees_.clear();

   // TODO remove participants and other things?
}

void Node::HandleMeetingStart(Message msg) {

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
         new_message(msg.sender, Words::RESOURCE_ANSWER);

         LOG("Send a response");
      }
   } else {
      // TODO: What else?
   }
}

void Node::HandleResourceAnswer(Message msg) {
   LOG("Recieved a response");
   if (resource_state_ == ResourceState::IDLE) {
      resource_state_ = ResourceState::WAITING;
      new_message(msg.sender, Words::RESOURCE_ACK);
      LOG("Accepted a resource of %d", msg.sender);
   } else {
      new_message(msg.sender, Words::RESOURCE_DENIED);
      LOG("Denied a resource of %d", msg.sender);
   }
}

void Node::HandleResourceAck(Message msg) {
   resource_count_ -= 1;
   new_message(msg.sender, Words::RESOURCE_SENT);
   LOG("Someone wanted my resource. Sent");
}

void Node::HandleResourceDenial(Message msg) {
   resource_state_ = ResourceState::IDLE;
   LOG("Someone didn't want my resource.");
}

void Node::HandleResourceDelivery(Message msg) {
   resource_count_ += 1;
   meet();
}

//
// ----- Meeting acceptance handler
//
// TODO - whole behavior
void Node::HandleMeetingAcceptanceRequest(Message msg) {
   // if I'm acceptor then accept
   // else pass to nearest acceptor via broadcast
}

// TODO - whole behavior
void Node::HandleMeetingAcceptanceAnswer(Message msg) {

}

// TODO - whole behavior
void Node::HandleMeetingAcceptanceAck(Message msg) {

}

// TODO - whole behavior
void Node::HandleMeetingAcceptanceDenial(Message msg) {

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