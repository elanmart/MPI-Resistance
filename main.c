#include <stdio.h>
#include <mpi/mpi.h>
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
   Node* nodes = safe_malloc(n, sizeof(Node));

   // root
   nodes[0].parent = NULL;
   nodes[0].level  = 0;

   uint32_t cnt = 0;
   uint32_t idx = 0;
   uint32_t max_children, n_children;

   // Topology
   while (cnt < n){
      max_children = min(cfg->max_nodes, n - cnt);
      n_children   = (uint32_t) (rand() % max_children) + 1;

      Node* node       = nodes + idx;
      node->children   = safe_malloc(n_children, sizeof(uint32_t));
      node->neighbours = safe_malloc(cfg->max_neighs, sizeof(uint32_t));

      for range(i, n_children) {
         nodes[cnt + i].parent = idx;
         nodes[cnt + i].level  = node->level + 1;
         node->children[i]     = cnt+i;
      }

      cnt += n_children;
      idx += 1;
   }

   // connections between neighbours
   int32_t start = 0;
   int32_t stop  = 0;
   while (start < n) {
      start = stop;
      // TODO: If neighbours have to have the same parent, just swap .level for .parent
      while ((&nodes[stop] != NULL) and (nodes[stop].level == nodes[start].level)) {
         stop++;
      }

      int32_t n1, n2;
      int32_t diff = stop - start;
      for range(i, diff * cfg->E_neighs) {
         n1 = (rand() % diff) + start;
         n2 = (rand() % diff) + start;
         if (n1 == n2)
            continue;

         add_neighbour(nodes + n1, (uint32_t) n2);  //todo: fix the messy casting that's floating fucking everywhere.
         add_neighbour(nodes + n2, (uint32_t) n1);  //todo: fix the messy casting that's floating fucking everywhere.
      }
   }

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
