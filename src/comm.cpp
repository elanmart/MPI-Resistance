#include "comm.h"

Manager::Manager(Config cfg) {
   mpi_init();

   cfg_         = cfg;
   MSG_Dataype_ = mpi_message_dtype(cfg_);
}

Manager::~Manager() {
   mpi_exit();
}

void Manager::send_node(Node &n, int dest) {
   auto msg = n.serialize();

   MPI_Send(&msg.second, msg.first, MPI_INT, dest, NOTAG, MPI_COMM_WORLD);
}

Node Manager::recv_node(int src) {
   int* buffer = new int[BUFFER_SIZE];
   MPI_Recv(buffer, BUFFER_SIZE, MPI_INT, src, NOTAG, MPI_COMM_WORLD, NULL);

   Node n = Node(buffer);

   return n;
}

void Manager::mpi_init() {
   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size_);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
}

void Manager::mpi_exit() {
   MPI_Finalize();
}

void Manager::communicate() {
   send_msg();
   recv_msg();
}

void Manager::send_msg() {
   if (not outgoing.empty()){
      Message msg = outgoing.front();
      MPI_Send(&msg, 1, MSG_Dataype_, msg.to, NOTAG, MPI_COMM_WORLD);

      outgoing.pop();
   }
}

void Manager::recv_msg() {
   int flag;
   MPI_Status status;
   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

   if (flag) {
      Message msg;
      int src = status.MPI_SOURCE;
      MPI_Recv(&msg, 1, MSG_Dataype_, src, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);

      msg.from = src;
      incoming.push(msg);
      incoming.pop();
   }
}

bool Manager::is_root() {
   return (rank_ == ROOT);
}

int Manager::root() {
   return ROOT;
}

MPI_Datatype mpi_message_dtype(Config &cfg) {
   int k = 5;

   int          sizes[k]   = {1, 1, 1, 1, 8};
   MPI_Datatype types[k]   = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
   MPI_Aint     offsets[k] = {offsetof(Message, timestamp), offsetof(Message, sender), offsetof(Message, destination),
                              offsetof(Message, word), offsetof(Message, payload)};

   MPI_Datatype dtype;
   MPI_Type_create_struct(k, sizes, offsets, types, &dtype);
   MPI_Type_commit(&dtype);

   return dtype;
}

