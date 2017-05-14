#include "backend.h"


MPI_Datatype make_node_dtype(config* cfg) {
   int32_t k = 5;

   int32_t      sizes[k]    = {1, cfg->max_nodes, cfg->max_neighs, 1, 1};
   MPI_Datatype types[k]    = {MPI_UINT32_T, MPI_UINT32_T, MPI_UINT32_T, MPI_UINT8_T, MPI_UINT8_T};
   MPI_Aint     offsets[k]  = {offsetof(Node, parent), offsetof(Node, children), offsetof(Node, neighbours),
                               offsetof(Node, has_resource), offsetof(Node, has_acceptor)};
   MPI_Datatype node_dtype;
   MPI_Type_create_struct(k, sizes, offsets, types, &node_dtype);
   MPI_Type_commit(&node_dtype);

   return node_dtype;
}

MPI_Status MPI_Recv_Default(void* buf, MPI_Datatype dtype, int32_t src) {
   MPI_Status status;
   MPI_Recv(buf, 1, dtype, src, 0, MPI_COMM_WORLD, &status);

   return status;
}
