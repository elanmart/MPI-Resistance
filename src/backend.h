#ifndef PR_INTERFACE_H
#define PR_INTERFACE_H

#include "common.h"
#include "node.h"

#define BUFFER_SIZE 4096 //todo: dynamic buffer? Using MPI_Status?
#define NOTAG 0

pair<int, int> mpi_init();
void mpi_exit();

void  mpi_send(void* data, int count, MPI_Datatype dtype, int dest);

template <typename T>
T* mpi_recv(int src, MPI_Datatype dtype) {
   T* buffer = new T[BUFFER_SIZE];
   for (int i = 0; i < BUFFER_SIZE; ++i) {
      buffer[i] = NULL;
   }

   MPI_Recv(buffer, BUFFER_SIZE, dtype, src, NOTAG, MPI_COMM_WORLD, NULL);

   return buffer;
}

#endif //PR_INTERFACE_H
