#include "backend.h"


pair<int, int> mpi_init() {
   int size, rank;

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   return {size, rank};
}

void mpi_exit() {
   MPI_Finalize();
}

void mpi_send(void* data, int count, MPI_Datatype dtype, int dest) {
   MPI_Send(data, count, dtype, dest, NOTAG, MPI_COMM_WORLD);
}



