#ifndef PR_NODE_H
#define PR_NODE_H

#include "common.h"
#include "comm.h"

class Node {
public:
   Node();
   Node(int ID);
   Node(int* buffer);

   int32_t ID_;
   int32_t level_;

   bool STOP_;

   int32_t      parent_;
   set<int32_t> neighbours_;
   set<int32_t> children_;

   pair<int, int*> serialize();
   void deserialize(int* buffer);

   void start_event_loop();

   Message listen();
   void handle(Message msg);
   void tick();

   void pass_acceptor();
   void initialzie_meeting();
};

#define DASH "=====================================\n"
#define LOG(msg, ...) printf("\n"                 \
                             DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n"  \
                             DASH,                \
                             this->ID_, ##__VA_ARGS__)

#endif //PR_NODE_H
