#include <stddef.h>
#include <stdlib.h>
#include "node.h"

void init() {
   uint32_t ID;

   uint32_t* parent;
   uint32_t* neighbours;
   uint32_t* children;

   uint32_t* awaiting_resource;
   uint32_t* participants;

   uint8_t  is_engaded;
   uint8_t  has_resource;
   uint8_t  has_acceptor;
   uint64_t time_penalty;

   char**        messages;
}


void finalize(Node* n) {

}

void start(Node *n) {

}

void add_neighbour(Node* n, uint32_t idx) {
   uint32_t i = 0;
   while (n->neighbours + i != NULL) {
      if (n->neighbours[i] == idx)
         return;

      i += 1;
   }

   n->neighbours[i] = idx;
}

void pass_acceptor(Node *n) {

}

void get_resource(Node *n) {

}

void get_acceptance(Node *n) {

}

void gather_nodes(Node *n) {

}
