#ifndef PR_INTERFACE_H
#define PR_INTERFACE_H


#include <mpi.h>
#include "node.h"
#include "utils.h"
#include <stddef.h>


MPI_Datatype make_node_dtype(Node *n);


#endif //PR_INTERFACE_H
