#include "message.h"

int64_t identifier(Message &m) {
   int64_t ID = 0;
   int64_t t  = (int64_t) m.number;

   ID |= m.sender;
   ID |= (t << 32);

   return ID;
}

Message  create_message(int msg_number, int sender, int destination) {
   Message msg = {-1,
                  -1,
                  msg_number,
                  (int) time(NULL),
                  sender,
                  destination,
                  Words::NONE,
                  {0, 0, 0, 0, 0, 0, 0, 0}
   };

   return msg;
}

Message create_message(int msg_number, int sender, int destination, Words w) {
   auto msg = create_message(msg_number, sender, destination);
   msg.word = w;

   return msg;
}

Message create_message(int msg_number, int sender, int destination, Words w, int payload[]) {
   auto msg = create_message(msg_number, sender, destination, w);
   for (int i = 0; i < 8; ++i) {
      msg.payload[i] = payload[i];
   }

   return msg;
}

Message detach(Message other) {
   Message msg = other;
   msg.__from__ = -1;
   msg.__to__   = -1;

   return msg;
}
