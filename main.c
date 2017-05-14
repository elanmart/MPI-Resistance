#include <stdio.h>
#include <mpi/mpi.h>
#include "src/node.h"
#include "src/config.h"


int main (int argc, char* argv[])
{
   config* cfg = parse(argc, argv);
   return 0;
}
