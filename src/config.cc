#include "config.h"


Config::Config() {
   max_children   = 3;
   max_neighbours = 3;
   avg_neighbours = 2;
   n_resources    = 2;
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

      } else if (arg_match(idx, "--max-children")) {
         max_children = atoi(argv[idx + 1]);

      } else if (arg_match(idx, "--max-neighbours")) {
         max_neighbours = atoi(argv[idx + 1]);

      } else if (arg_match(idx, "--avg-neighbours")) {
         avg_neighbours = atoi(argv[idx + 1]);

      } else if (arg_match(idx, "--n-resources")) {
         n_resources = atoi(argv[idx + 1]);
      }
   }
}

void Config::help() {
   cerr    << "\n Available options:\n"
           << "  --max-children     maximum number of children_ per node  [" << max_children   << "]\n"
           << "  --max-neighbours   maximum number of neihbours per node [" << max_neighbours << "]\n"
           << "  --avg-neighbours   average number of neihbours per node [" << avg_neighbours << "]\n"
           << endl;
}

Config::Config(int argc, char** argv) : Config() {
   this->parse_args(argc, argv);
}
