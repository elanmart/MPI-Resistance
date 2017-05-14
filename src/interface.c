#include "interface.h"


MPI_Datatype make_node_dtype(Node *n) {
   int32_t k = 5;

   int32_t      sizes[k]    = {1, unsafe_len(n->children), unsafe_len(n->neighbours), 1, 1};
   MPI_Datatype types[k]    = {MPI_UINT32_T, MPI_UINT32_T, MPI_UINT32_T, MPI_UINT8_T, MPI_UINT8_T};
   MPI_Aint     offsets[k]  = {offsetof(Node, parent), offsetof(Node, children), offsetof(Node, neighbours),
                               offsetof(Node, has_resource), offsetof(Node, has_acceptor)};
   MPI_Datatype node_dtype;
   MPI_Type_create_struct(k, sizes, offsets, types, &node_dtype);
   MPI_Type_commit(&node_dtype);

   return node_dtype;
}
