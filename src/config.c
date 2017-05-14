#include "config.h"


#define ARG(_name, _type, _default) {                  \
   char* val  = get(argc, argv, #_name);               \
   cfg->_name = (val != NULL) ? _type(val) : _default; \
}

char *get(int argc, char **argv, char *key) {
   for(int i=0; i<argc-1; i++)
      if(strcmp(argv[i]+2, key) == 0)
         return argv[i+1];

   return NULL;
}

struct _config *parse(int argc, char **argv) {
   config* cfg = malloc(sizeof(config));

   ARG(max_nodes, INT,  2048);
   ARG(nb_proba,  REAL, 0.3);
   ARG(mode,      STR,  "DEBUG");

   return cfg;
}
