#ifndef PR_CONFIG_H
#define PR_CONFIG_H

#include "common.h"

using namespace std;

class Config {
public:
   Config();
   Config(int argc, char** argv);

   int32_t max_children;
   int32_t max_neighbours;
   int32_t avg_neighbours;

   void parse_args(int argc, char** argv);
   void help();
};
