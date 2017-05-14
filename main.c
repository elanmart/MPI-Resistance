#include <stdio.h>
#include <mpi/mpi.h>
#include <bits/signum.h>
#include <signal.h>
#include <zconf.h>
#include "src/node.h"
#include "src/config.h"


int32_t* _init_mpi() {
   int32_t size, rank;

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   int* status = malloc(sizeof(int) * 2);
   status[0] = size;
   status[1] = rank;

   return status;
}

void _exit_mpi() {
   MPI_Finalize();
}



Node* _init_tree(config* cfg) {
   Node* nodes = malloc(cfg->max_nodes * sizeof(Node));

   return nodes;
}

int main (int argc, char* argv[])
{
   int32_t* status = _init_mpi();

   if (status[1] is 0) {
      config* cfg = parse(argc, argv);
      Node* tree  = _init_tree(cfg);
   }

   event_loop();

   free(status);
   _exit_mpi();
   
   return 0;
}
