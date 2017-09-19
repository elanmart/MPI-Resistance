#ifndef PR_CONFIG_H
#define PR_CONFIG_H

#include "common.h"

using namespace std;

class Config {
public:
   Config();
   Config(int argc, char** argv); // parsuje bezpośrednio z linii poleceń

   int tree_size;        // ile nodów w drzewie

   int max_children;     // max liczba dzieci per node
   int max_neighbours;   // max liczba sąsiadów per node
   int avg_neighbours;   // średnia liczba sąsiadóœ per node (losujemy sobie ile ich ma być)

   int n_resources;      // ile risorsów w całym drzewie

   int n_acceptors;      // ile akceptorów w drzewie

   void parse_args(int argc, char** argv); // ciekawe co to robi
   void help();
};

#endif