#ifndef PR_COMM_H
#define PR_COMM_H

#include "common.h"
#include "config.h"
#include "message.h"
#include "node.h"

#define BUFFER_SIZE 4096
#define ROOT   0
#define NOTAG  0
#define ALL   -1

class Node;

class Manager {

// threading
unique_ptr<mutex> _incoming_queue_mutex = nullptr;
unique_ptr<mutex> _outgoing_queue_mutex = nullptr;

unique_ptr<thread> _sender_thread = nullptr;
unique_ptr<thread> _reciever_thread = nullptr;

public:
   // ctors
   Manager(Config cfg);
   ~Manager();
   void start();

   // mpi-state
   int size_;
   int rank_;

   // queues
   bool get(Message *msg);
   void put(Message msg, int dest);

   // communication
   void send_node(Node& n, int dest);
   Node recv_node(int src);

   // utils
   bool is_root();
   int  root();

private:
   // mpi-ctors
   void mpi_init();
   void mpi_exit();

   void start_sender();
   void start_reciever();

   void _sender_loop();
   void _reciever_loop();

   void _send_msg();
   void _recv_msg();

   // queues
   queue<Message> incoming;
   queue<Message> outgoing;

   // configuration & datatypes
   Config       cfg_;
   MPI_Datatype MSG_Dataype_;

   // SYNC
   bool STOP_;
};

MPI_Datatype get_mpi_message_dtype(Config &cfg);

#endif //PR_COMM_H
