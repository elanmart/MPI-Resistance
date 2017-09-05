#include "message.h"

int64_t identifier(Message &m) {
   int64_t ID = 0;
   int64_t t  = (int64_t) m.number;

   ID |= m.sender;
   ID |= (t << 32);

   return ID;
}


Message  create_message(int msg_number, int sender, int destination,
                        Words word, int payload[]) {

   auto timestamp = (int) time(NULL);

   Message msg = {
           -1,                         // __from__
           -1,                         // __to__
           msg_number,                 // number
           timestamp,                  // timestamp
           sender,                     // sender
           destination,                // destination
           word,                       // word
   };

   if (payload != nullptr)
      for (int i=0; i<PAYLOAD_SIZE; i++)
         msg.payload[i] = payload[i];

   return msg;
}

Message create_message_to_acceptor(int msg_number, int sender) {
  auto timestamp = (int) time(NULL);

  Message msg = {
      -1,
      -1,
      msg_number,
      timestamp,
      sender,
      -1,
      Words::MEETING_ACCEPTANCE_ASK,
  };
}

Message detach(Message other) {
   Message msg = other;
   msg.__from__ = -1;
   msg.__to__   = -1;

   return msg;
}
