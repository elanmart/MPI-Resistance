#include "node.h"
#include "utils.h"

// --- ctors ---

Node::Node() {
   level_          = 0;
   parent_         = -1;
   msg_number_     = 0;
   resource_count_ = 0;
   resource_state_ = ResourceState::IDLE;
   meeting_state_  = MeetingState::IDLE;
   STOP_           = false;
}

Node::Node(int ID) : Node() {
   ID_ = ID;
}

Node::Node(int *buffer) {
   deserialize(buffer);
}

void Node::set_manager(Manager *m) {
   manager_ = m;
}

// --- comms ---
void Node::new_message(int destination, Words w, int payload[] = nullptr) {
   msg_number_++;
   auto msg = create_message(msg_number_, ID_, destination, w, payload);

   _send(msg);
}


void Node::_send(Message msg) {
   msg_cache_.insert(identifier(msg));
   broadcast(msg);
}

// --- logic ---

// todo: focus on this
void Node::start_event_loop() {
   Message msg;

   if (resource_count_ > 0) {
      LOG("I have resources: %d", resource_count_);
   }

   int send = 1;
   while (not STOP_) {
      manager_->communicate();

      if (manager_->get(&msg))
         consume(msg);

      if (ID_ == 12 and send == 1) {
         send = 0;
         get_resource();
      }

      if (0) {
         pass_acceptor();
      }

      if (0) {
         // clear_message_buffer(); (look at timestamps at throw away the oldest);
      }
   }
}

// todo: focus on this
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

// todo: focus on this
void Node::initialzie_meeting() {
   LOG("Witam");


}

void Node::get_resource() {
   assert(resource_count_ == 0);
   LOG("I need resource. Please propagate this to everyone!");
   new_message(ALL, Words::RESOURCE_REQUEST);
}

// todo: WIP
void Node::handle(Message msg) {
   if (msg.word == Words::NONE) {

      //todo: working on participants acquisition here

   } else if (msg.word == Words::MEETING_ACCEPT) {
      participants_.insert(msg.sender);
      awaiting_response_ -= 1;


   } else if (msg.word == Words::MEETING_DECLINE) {
      awaiting_response_ -= 1;

   } else if (msg.word == Words::MEETING_JOIN) {
      if (time_penalty_ > 0)
         new_message(msg.sender, Words::MEETING_DECLINE);

      if (meeting_state_ == MeetingState::IDLE) {
         new_message(msg.sender, Words::MEETING_ACCEPT);
         meeting_state_ = MeetingState::WAITING;
      }

      if (meeting_state_ == MeetingState::MASTER_ORG){
         meeting_state_ = MeetingState::SLAVE_ORG;

      }

   } else if (msg.word == Words::MEETING_CANCEL) {
   } else if (msg.word == Words::MEETING_NEW_ORG) {
   } else if (msg.word == Words::MEETING_NEW_ORG_PROBE) {
   } else if (msg.word == Words::MEETING_PARTC_ACK) {

      // -------------------------------------------------------------

      // hey, u got some resource?
   } else if (msg.word == Words::RESOURCE_REQUEST and resource_count_ > 0){
      LOG("Handling resource request");

      if (resource_state_ == ResourceState::LOCKED) {
         //todo: push to a queue of awaiting requests.
      } else if (resource_state_ == ResourceState::IDLE){
         resource_state_ = ResourceState::LOCKED;
         new_message(msg.sender, Words::RESOURCE_ANSWER);

         LOG("Send a response");
      }

      // hey, i got some resource, u want?
   } else if (msg.word == Words::RESOURCE_ANSWER){
      LOG("Recieved a response");
      if (resource_state_ == ResourceState::IDLE) {
         resource_state_ = ResourceState::WAITING;
         new_message(msg.sender, Words::RESOURCE_ACK);
         LOG("Accepted a resource of %d", msg.sender);
      } else {
         new_message(msg.sender, Words::RESOURCE_DEN);
         LOG("Denied a resource of %d", msg.sender);
      }

      // hey, i want ur resource. pls send
   } else if (msg.word == Words::RESOURCE_ACK) {
      resource_count_ -= 1;
      new_message(msg.sender, Words::RESOURCE_SENT);
      LOG("Someone wanted my resource. Sent");

      // hey i dont want ur resource after all
   } else if (msg.word == Words::RESOURCE_DEN) {
      // todo: next in line
      resource_state_ = ResourceState::IDLE;
      LOG("Someone didn't want my resource.");

      // hey, here's the resource
   } else if (msg.word == Words::RESOURCE_SENT){
      resource_count_ += 1;
      meet();
   }
}

void Node::meet() {
   LOG("MEETING");
   resource_state_ = ResourceState::IDLE;
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

void Node::send_to(Message msg, int dest) {
   manager_->push(msg, dest);
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

// --- serialze ---

pair<int, int *> Node::serialize() {
   int n_children   = (int) children_.size();
   int n_neighbours = (int) neighbours_.size();

   int n_items = 4 + 2 + n_children + n_neighbours;
   int* buffer = new int[n_items];

   buffer[0]  = ID_;
   buffer[1]  = level_;
   buffer[2]  = parent_;
   buffer[3]  = resource_count_;
   buffer[4]  = n_children;
   buffer[5]  = n_neighbours;
   int offset = 6;

   for (auto item : children_)
      buffer[offset++] = item;

   for (auto item : neighbours_)
      buffer[offset++] = item;

   return make_pair(n_items, buffer);
}

void Node::deserialize(int* buffer) {
   ID_               = buffer[0];
   level_            = buffer[1];
   parent_           = buffer[2];
   resource_count_   = buffer[3];
   int n_children    = buffer[4];
   int n_neighbours  = buffer[5];
   int offset        = 6;

   children_.clear();
   neighbours_.clear();

   for (int i=offset; i<offset + n_children; i++)
      children_.insert(buffer[i]);

   offset += n_children;
   for (int i=offset; i<offset+n_neighbours; i++)
      neighbours_.insert(buffer[i]);
}
