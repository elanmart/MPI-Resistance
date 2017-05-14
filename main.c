#include <stdio.h>
#include <mpi/mpi.h>
#include <bits/signum.h>
#include <signal.h>
#include <zconf.h>
#include "src/node.h"
#include "src/config.h"


void _init_mpi(int32_t* size, int32_t* rank) {

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, size);
   MPI_Comm_rank(MPI_COMM_WORLD, rank);
}

void _exit_mpi() {
   MPI_Finalize();
}



Node* _init_tree(int32_t n, config* cfg) {
   Node* nodes = malloc(cfg->max_nodes * sizeof(Node));

   return nodes;
}

int main (int argc, char* argv[])
{
   int32_t size, rank;
   _init_mpi(&size, &rank);

   if (rank is 0) {
      config* cfg = parse(argc, argv);
      Node* tree  = _init_tree(size, cfg);
   }

   event_loop();

   _exit_mpi();

   return 0;
}
