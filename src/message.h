#ifndef PR_MESSAGE_H
#define PR_MESSAGE_H

#include "common.h"

enum Word {
   // resource acquisition
   RESOURCE_REQUEST = 0,
   RESOURCE_ANSWER  = 1,
   RESOURCE_ACK     = 2,
   RESOURCE_SENT    = 3,

   // acceptance acquisition
   MEETING_REQUEST  = 4,
   MEETING_FINISHED = 5,
   REQUEST_ACK      = 6,
   REQUEST_ALLOW    = 7,

   // organizing a meeting
   JOIN          = 8,
   CANCEL        = 9,
   DECLINE       = 10,
   ACCEPT        = 11,
   NEW_ORG_PROBE = 12,
   NEW_ORG       = 13,

   NONE = 14
};

typedef struct _Message {
   int __from__;
   int __to__;

   int  number;
   int  timestamp;
   int  sender;
   int  destination;
   Word word;

   int payload[8];
} Message;

int64_t identifier(Message &m);

Message create_message(int &msg_number, int sender, int destination);

Message create_message(int &msg_number, int sender, int destination, Word w);

Message create_message(int &msg_number, int sender, int destination, Word w, int payload[]);

Message detach(Message other);


#endif //PR_MESSAGE_H
