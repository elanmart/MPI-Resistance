#include "utils.h"

int randint(int low, int high) {
   assert(low <= high);

   random_device rd;
   mt19937 rng(rd());
   uniform_int_distribution<int> dist(low, high);

   return dist(rng);
}
