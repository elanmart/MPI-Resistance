#ifndef PR_NODE_H
#define PR_NODE_H

#include "common.h"
#include "comm.h"

class Manager; // todo: handle this;

enum ResourceState { IDLE, WAITING, LOCKED};

class Node {
public:
   // ctors
   Node();
   Node(int ID);
   Node(int* buffer);
   void set_manager(Manager *m);

   // identity
   int32_t  ID_;
   int32_t  level_;
   Manager* manager_ = nullptr;

   // tasks
   int resource_count_;

   // topology
   int32_t      parent_;
   set<int32_t> neighbours_;
   set<int32_t> children_;

   // serialization
   pair<int, int*> serialize();
   void            deserialize(int* buffer);

   // generic communication
   void start_event_loop();
   void broadcast(Message msg);
   bool accept(Message &msg);
   void consume(Message &msg);
   void send_to(Message msg, set<int> recipients);
   void send_to(Message msg, int dest);
   Message new_message(int destination, Words w);
   Message new_message(int destination, Words w, int payload[]);

   // message bookkeeping
   set<int64_t> msg_cache_; // todo: replace set with a map;
   int msg_number_;

   // special skills
   void pass_acceptor();
   void initialzie_meeting();
   void get_resource();
   void handle(Message msg);
   void get_accepatnce();

   ResourceState resource_state_;

   // synchronization
   bool STOP_;

   void meet();
};

#define DASH "=====================================\n"
#define LOG(msg, ...) printf(DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n"  \
                             DASH,                \
                             this->ID_, ##__VA_ARGS__)

#endif //PR_NODE_H
