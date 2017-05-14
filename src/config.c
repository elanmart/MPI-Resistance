#include "config.h"


#define ARG(_name, _type, _default) {                  \
   char* val  = get(argc, argv, #_name);               \
   if (val is NULL)                                    \
      cfg->_name = _default;                           \
   else                                                \
      cfg->_name = _type(val);                         \
}

char* get(int argc, char **argv, char *key) {
   for(int i=0; i<argc-1; i++)
      if(strcmp(argv[i]+2, key) == 0)
         return argv[i+1];

   return NULL;
}

config* parse(int argc, char **argv) {
   config* cfg = malloc(sizeof(config));

   ARG(max_nodes, INT,   2048);
   ARG(nb_proba,  FLOAT, 0.3);
   ARG(mode,      STR,   "DEBUG");

   return cfg;
}
