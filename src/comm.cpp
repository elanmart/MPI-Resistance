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

void Manager::send_msg(Message msg, int dest) {
   MPI_Send(&msg, 1, MSG_Dataype_, dest, NOTAG, MPI_COMM_WORLD);
}

Message Manager::recv_msg() {
   return Message();
}

bool Manager::is_root() {
   return (rank_ == ROOT);
}

int Manager::root() {
   return ROOT;
}

MPI_Datatype mpi_message_dtype(Config &cfg) {
   int32_t k = 5;

//   int32_t      sizes[k]    = {1, cfg.max_children, cfg.max_neighbours, 1, 1};
//   MPI_Datatype types[k]    = {MPI_UINT32_T, MPI_UINT32_T, MPI_UINT32_T, MPI_UINT8_T, MPI_UINT8_T};
//   MPI_Aint     offsets[k]  = {offsetof(Node, parent), offsetof(Node, children), offsetof(Node, neighbours),
//                               offsetof(Node, has_resource), offsetof(Node, has_acceptor)};
//   MPI_Datatype node_dtype;
//   MPI_Type_create_struct(k, sizes, offsets, types, &node_dtype);
//   MPI_Type_commit(&node_dtype);
//
//   return node_dtype;

}
