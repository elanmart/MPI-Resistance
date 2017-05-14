#include <stdlib.h>
#include "utils.h"


uint32_t unsafe_len(void *arr) {
   uint32_t  cnt = 0;
   for(void* item=arr; item!=NULL; item++)
      cnt += 1;

   return cnt;
}

void* safe_malloc(int32_t count, size_t dtype_size) {
   void* var = malloc((count+1) * dtype_size);
   for range(i, count+1) {
      var[i] = NULL;
   }

   return var;
}
