#ifndef PR_CONFIG_H
#define PR_CONFIG_H

#include "common.h"

using namespace std;

class Config {
public:
   Config();
   Config(int argc, char** argv); 

   int tree_size;

   int max_children;
   int max_neighbours;
   int avg_neighbours;

   int n_resources;

   int n_acceptors;
   int max_processes;

   void parse_args(int argc, char** argv);
   void help();
};

#endif