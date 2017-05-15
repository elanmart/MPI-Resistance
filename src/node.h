#ifndef PR_NODE_H
#define PR_NODE_H

#include "common.h"

class Node {
public:
   Node();
   Node(int ID);
   Node(int* buffer);

   int32_t ID_;
   int32_t level_;

   int32_t      parent_;
   set<int32_t> neighbours_;
   set<int32_t> children_;

   pair<int, int*> serialize();
   void deserialize(int* buffer);

   void start();
   void start_event_loop();
};


#endif //PR_NODE_H
