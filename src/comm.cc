#include "comm.h"
#include "utils.h"

// --- ctors ---

Manager::Manager(Config cfg) {
   mpi_init();

   cfg_         = cfg;
   MSG_Dataype_ = get_mpi_message_dtype(cfg_);
}

Manager::~Manager() {
   mpi_exit();
}

void Manager::mpi_init() {
   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size_);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
}

void Manager::mpi_exit() {
   MPI_Finalize();
}

// --- setup ---

void Manager::send_node(Node &n, int dest) {
   auto msg = n.serialize();
   MPI_Send(msg.second, msg.first, MPI_INT, dest, NOTAG, MPI_COMM_WORLD);
}

Node Manager::recv_node(int src) {
   int* buffer = new int[BUFFER_SIZE];
   MPI_Recv(buffer, BUFFER_SIZE, MPI_INT, src, NOTAG, MPI_COMM_WORLD, NULL);

   Node n = Node(buffer);

   return n;
}

// --- queues ---

bool Manager::get(Message *msg) {
   if (not incoming.empty()) {
      (*msg) = incoming.front();
      incoming.pop();

      return true;
   }

   return false;
}

void Manager::push(Message msg, int dest) {
   if (msg.__from__ != dest and dest >= 0) {
      msg.__to__ = dest;
      outgoing.push(msg);
   }
}

// --- comms ---

void Manager::communicate() {
   send_msg();
   recv_msg();
}

void Manager::send_msg() {
   if (not outgoing.empty()){
      Message msg = outgoing.front();
      MPI_Send(&msg, 1, MSG_Dataype_, msg.__to__, NOTAG, MPI_COMM_WORLD);

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

      msg.__from__ = src;
      incoming.push(msg);
   }
}

// --- utils ---

bool Manager::is_root() {
   return (rank_ == ROOT);
}

int Manager::root() {
   return ROOT;
}

MPI_Datatype get_mpi_message_dtype(Config &cfg) {
   const int k = 6;

   int          sizes[k]   = {1, 1, 1, 1, 1, 8};
   MPI_Datatype types[k]   = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
   MPI_Aint     offsets[k] = {offsetof(Message, number), offsetof(Message, timestamp),
                              offsetof(Message, sender), offsetof(Message, destination),
                              offsetof(Message, word),   offsetof(Message, payload)};

   MPI_Datatype dtype;
   MPI_Type_create_struct(k, sizes, offsets, types, &dtype);
   MPI_Type_commit(&dtype);

   return dtype;
}

