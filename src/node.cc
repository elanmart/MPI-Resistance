#include "node.h"
#include "utils.h"

// --- ctors ---

Node::Node() {
   level_     = 0;
   parent_    = -1;
   msg_number_ = 0;
   STOP_      = false;
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

// --- logic ---

void Node::start_event_loop() {
   LOG("I'm alive!");

   int xx = 0;

   Message msg;
   while (not STOP_) {
      manager_->communicate();

      if (manager_->get(&msg))
         consume(msg);

      if (((xx++) == 0) and (ID_ == 0)) {
         broadcast(create_message(msg_number_, ID_, ALL));
      }

      if (0) {
         initialzie_meeting();
      }

      if (0) {
         pass_acceptor();
      }

      if (0) {
         // clear_message_buffer(); (look at timestamps at throw away the oldest);
      }

      LOG("TICK");
      sleep(1);
   }
}

void Node::consume(Message &msg) {
   // if we've seen the message, don't do anything

   LOG("MSG arrived");

   auto key = identifier(msg);
   if (msg_cache_.find(key) != msg_cache_.end())
      return;
   msg_cache_.insert(key);

   LOG("MSG processed");

   // not to me. Forward everywhyere
   if (msg.destination != ID_ or msg.destination == ALL) {
      broadcast(msg);
   }
}

void Node::send_to(Message msg, int dest) {
   LOG("Sending msg number: %d to: %d", msg.number, dest);
   manager_->push(msg, dest);
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

// --- serialze ---

pair<int, int *> Node::serialize() {
   int n_children   = (int) children_.size();
   int n_neighbours = (int) neighbours_.size();

   int n_items = 3 + 2 + n_children + n_neighbours;
   int* buffer = new int[n_items];

   buffer[0]  = ID_;
   buffer[1]  = level_;
   buffer[2]  = parent_;
   buffer[3]  = n_children;
   buffer[4]  = n_neighbours;
   int offset = 5;

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
   int n_children    = buffer[3];
   int n_neighbours  = buffer[4];
   int offset        = 5;

   children_.clear();
   neighbours_.clear();

   for (int i=offset; i<offset + n_children; i++)
      children_.insert(buffer[i]);

   offset += n_children;
   for (int i=offset; i<offset+n_neighbours; i++)
      neighbours_.insert(buffer[i]);
}
