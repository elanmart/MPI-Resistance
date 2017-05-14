#ifndef PR_NODE_H
#define PR_NODE_H


#include <stdint.h>
#include "utils.h"


typedef struct _Node {
   struct uint32_t* parent;
   struct uint32_t* neighbours;
   struct uint32_t* children;

   struct uint32_t* awaiting_resource;
   struct uint32_t* participants;

   uint8_t       is_engaded;
   uint8_t       has_resource;
   uint8_t       has_acceptor;
   uint64_t      time_penalty;

   char*         messages;
} Node;


void init(Node *n);
void event_loop(Node* n);
void pass_acceptor(Node* n);
void get_resource(Node* n);
void get_acceptance(Node* n);
void gather_nodes(Node* n);


#endif //PR_NODE_H
