#ifndef PR_NODE_H
#define PR_NODE_H

#include <mutex>
#include "common.h"
#include "comm.h"

class Manager; // todo: handle this;

enum class ResourceState { IDLE, WAITING, LOCKED};
enum class MeetingState  { IDLE, WAITING, LOCKED, MASTER_ORG, SLAVE_ORG};

class Node {
public:
   // EXPERIMENTAL -----------------------------
   std::mutex message_queue_mutex;

   // END-EXPERIMENTAL -------------------------

   // ctors
   Node();
   Node(int ID);
   Node(int* buffer);

   // identity
   int32_t  ID_;
   int32_t  level_;

   // topology
   int32_t      parent_;
   set<int32_t> neighbours_;
   set<int32_t> children_;

   // serialization
   pair<int, int*> serialize();
   void            deserialize(int* buffer);

   // communication interface
   Manager* manager_ = nullptr;
   void set_manager(Manager *m);

   // tasks
   int           resource_count_;
   int           awaiting_response_;
   int           time_penalty_;
   set<int>      participants_;
   set<int>      perhaps_merge_orgs_;
   MeetingState  meeting_state_;
   ResourceState resource_state_;

   // generic communication
   void start_event_loop();

   void new_message(int destination, Words w, int payload[] = nullptr);
   void _send(Message msg);

   bool get(Message *msg);
   void put(Message msg, int dest);

   void broadcast(Message msg);
   bool accept(Message &msg);
   void consume(Message &msg);

   void send_to(Message msg, set<int> recipients);
   void send_to(Message msg, int dest);

   // message bookkeeping
   set<int64_t> msg_cache_; // todo: replace set with a map;
   int msg_number_;

   // special skills
   void pass_acceptor();
   void initialzie_meeting();
   void get_resource();
   void handle(Message msg);
   void get_accepatnce();
   void meet();

   // synchronization
   bool STOP_;
};

#define DASH "=====================================\n"
#define LOG(msg, ...) printf(DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n"  \
                             DASH,                \
                             this->ID_, ##__VA_ARGS__)

#endif //PR_NODE_H
