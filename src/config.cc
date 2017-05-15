#include "config.h"


Config::Config() {
   max_children   = 4;
   max_neighbours = 4;
   avg_neighbours = 2;
}

void Config::parse_args(int argc, char** argv) {
   auto arg_match = [argv](int32_t idx, string name) { return strcmp(argv[idx], name.c_str()) == 0; };

   for (int32_t idx=1; idx < argc; idx+=2) {

      if (argv[idx][0] != '-' or argv[idx][1] != '-') {
         cerr << "Invalid arguments list. (Arguments should start with double dash [--arg])" << endl;
         help();
         exit(EXIT_FAILURE);
      }

      if (arg_match(idx, "-h")) {
         cerr << "Printing help: " << endl;
         help();
         exit(EXIT_FAILURE);

      } else if (arg_match(idx, "--max-children_")) {
         max_children = atoi(argv[idx + 1]);

      } else if (arg_match(idx, "--max-neighbours_")) {
         max_neighbours = atoi(argv[idx + 1]);

      } else if (arg_match(idx, "--avg-neighbours_")) {
         avg_neighbours = atoi(argv[idx + 1]);
      }
   }
}

void Config::help() {
   cerr    << "\n Available options:\n"
           << "  --max-children_     maximum number of children_ per node  [" << max_children   << "]\n"
           << "  --max-neighbours_   maximum number of neihbours per node [" << max_neighbours << "]\n"
           << "  --avg-neighbours_   average number of neihbours per node [" << avg_neighbours << "]\n"
           << endl;
}

Config::Config(int argc, char** argv) : Config() {
   this->parse_args(argc, argv);
}
