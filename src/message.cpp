#include "message.h"

int64_t identifier(Message &m) {
   int64_t ID = 0;

   ID |= m.sender;
   ID |= (m.timestamp << 32);

   return ID;
}