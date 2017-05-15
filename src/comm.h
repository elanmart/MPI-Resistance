#ifndef PR_COMM_H
#define PR_COMM_H

#include "node.h"
#include "config.h"
#include "message.h"

#define BUFFER_SIZE 4096 //todo: dynamic buffer? Using MPI_Status?
#define NOTAG 0
#define ROOT  0


MPI_Datatype mpi_message_dtype(Config &cfg);

class Manager {
public:
   // ctors
   Manager();
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
   queue<Message> incoming;
   queue<Message> outgoing;

   // configuration & datatypes
   Config       cfg_;
   MPI_Datatype MSG_Dataype_;

   // utils
   bool is_root();
   int  root();
};

#endif //PR_COMM_H
