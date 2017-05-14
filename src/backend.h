#ifndef PR_INTERFACE_H
#define PR_INTERFACE_H


#include <mpi.h>
#include "node.h"
#include "utils.h"
#include "config.h"
#include <stddef.h>


#define MPI_Send_Default(_buff, _dtype, _dest) MPI_Send((_buff), 1, (_dtype), (_dest), 0, MPI_COMM_WORLD)


MPI_Datatype make_node_dtype(config* cfg);
MPI_Status MPI_Recv_Default(void* buf, MPI_Datatype dtype, int32_t src);

#endif //PR_INTERFACE_H
