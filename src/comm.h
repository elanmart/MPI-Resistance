#ifndef PR_COMM_H
#define PR_COMM_H

#include "common.h"
#include "config.h"
#include "message.h"
#include "node.h"

//TODO: dynamic buffer?
//TODO: ROOT --> ?
#define BUFFER_SIZE 4096
#define ROOT   0
#define NOTAG  0
#define ALL   -1

class Node;

class Manager {
public:
   // ctors
   Manager(Config cfg);
   ~Manager();

   // mpi-ctors
   void mpi_init();
   void mpi_exit();

   // mpi-state
   int size_;
   int rank_;

   // communication
   void send_node(Node& n, int dest);
   Node recv_node(int src);
   void communicate();
   void send_msg();
   void recv_msg();

   // queues
   bool get(Message *msg);
   void push(Message msg, int dest);
   queue<Message> incoming;
   queue<Message> outgoing;

   // configuration & datatypes
   Config       cfg_;
   MPI_Datatype MSG_Dataype_;

   // utils
   bool is_root();
   int  root();
};

MPI_Datatype get_mpi_message_dtype(Config &cfg);

#endif //PR_COMM_H
