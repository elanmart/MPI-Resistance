#ifndef PR_COMM_H
#define PR_COMM_H

#include "node.h"
#include "config.h"

#define BUFFER_SIZE 4096 //todo: dynamic buffer? Using MPI_Status?
#define NOTAG 0

typedef struct _Message {

} Message;

MPI_Datatype mpi_message_dtype(Config &cfg);

class Manager {
public:
   Manager(Config cfg);
   ~Manager();

   int size_;
   int rank_;

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
