#include "utils.h"


uint32_t unsafe_len(void *arr) {
   uint32_t  cnt = 0;
   for(void* item=arr; item!=NULL; item++)
      cnt += 1;

   return cnt;
}
