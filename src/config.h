#ifndef PR_CONFIG_H
#define PR_CONFIG_H

#include "common.h"

using namespace std;

class Config {
public:
   Config();
   Config(int argc, char** argv);

   int max_children;
   int max_neighbours;
   int avg_neighbours;

   float acceptor_proba; // todo: add this to parser
   float resource_proba; // todo: add this to parser

   void parse_args(int argc, char** argv);
   void help();
};

#endif