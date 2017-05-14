#ifndef PR_NODE_H
#define PR_NODE_H


#include <stdint.h>
#include "utils.h"


typedef struct _Node {
   uint32_t ID;
   uint32_t level;

   uint32_t  parent;
   uint32_t* neighbours;
   uint32_t* children;

   uint32_t* awaiting_resource;
   uint32_t* participants;

   uint8_t   is_engaded;
   uint8_t   has_resource;
   uint8_t   has_acceptor;
   uint64_t  time_penalty;

   char*     messages;
} Node;


void init(Node *n);
void finalize(Node *n);
void add_neighbour(Node* n, uint32_t idx);
void start(Node* n);
void pass_acceptor(Node* n);
void get_resource(Node* n);
void get_acceptance(Node* n);
void gather_nodes(Node* n);


#endif //PR_NODE_H
