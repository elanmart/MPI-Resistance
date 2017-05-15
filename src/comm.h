#ifndef PR_COMM_H
#define PR_COMM_H

#include "node.h"
#include "config.h"

#define BUFFER_SIZE 4096 //todo: dynamic buffer? Using MPI_Status?
#define NOTAG 0
#define ROOT  0



enum Word {
   // resource acquisition
   RESOURCE_REQUEST = 0,
   RESOURCE_ANSWER = 1,
   RESOURCE_ACK = 2,
   RESOURCE_SENT = 3,

   // acceptance acquisition
   MEETING_REQUEST = 4,
   MEETING_FINISHED = 5,
   REQUEST_ACK = 6,
   REQUEST_ALLOW = 7,

   // organizing a meeting
   JOIN = 8,
   CANCEL = 9,
   DECLINE = 10,
   ACCEPT = 11,
   NEW_ORG_PROBE = 12,
   NEW_ORG = 13
};

typedef struct _Message {
   int timestamp;
   int sender;
   int destination;

   Word word;

   int payload[8];
} Message;


int64_t identifier(Message &m) {
   int64_t ID = 0;

   ID |= m.sender;
   ID |= (m.timestamp << 32);

   return ID;
}

MPI_Datatype mpi_message_dtype(Config &cfg);

class Manager {
public:
   Manager(Config cfg);
   ~Manager();

   int size_;
   int rank_;

   bool is_root();
   int  root();

   Config       cfg_;
   MPI_Datatype MSG_Dataype_;

   void send_node(Node& n, int dest);
   Node recv_node(int src);

   void mpi_init();
   void mpi_exit();

   void send_msg(Message msg, int dest);
   Message recv_msg();
};

#endif //PR_COMM_H
