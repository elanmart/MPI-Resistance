#include "node.h"

// --- ctors ---

Node::Node() {
   level_  = 0;
   parent_ = -1;
   STOP_   = false;
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

   while (not STOP_) {
      manager_->communicate();

      if (not manager_->incoming.empty()) {
         Message msg = manager_->incoming.front();
         consume(msg);
         manager_->incoming.pop();
      }

      if (0) {
         initialzie_meeting();
      }

      if (0) {
         pass_acceptor();
      }
   }
}

void Node::consume(Message &msg) {

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
