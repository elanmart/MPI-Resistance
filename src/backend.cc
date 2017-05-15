#include "backend.h"


pair<int, int> init_mpi() {
   int size, rank;

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   return {size, rank};
}

void exit_mpi() {
   MPI_Finalize();
}

