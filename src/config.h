#ifndef PR_CONFIG_H
#define PR_CONFIG_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <printf.h>
#include "utils.h"


#define INT    atoi
#define FLOAT  atof
#define STR

typedef struct _config {
   uint32_t max_nodes;
   uint32_t E_neighs;
   uint32_t max_neighs;
   float nb_proba;
   char *mode;
} config;


char *get(int argc, char *argv[], char *key);
config *parse(int argc, char *argv[]);


#endif //PR_CONFIG_H
