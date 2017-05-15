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
   NEW_ORG       = 13
};

typedef struct _Message {
   int from;
   int to;

   int timestamp;
   int sender;
   int destination;

   Word word;

   int payload[8];
} Message;

int64_t identifier(Message &m);


#endif //PR_MESSAGE_H
