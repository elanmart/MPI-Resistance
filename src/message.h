#ifndef PR_MESSAGE_H
#define PR_MESSAGE_H

#include "common.h"

enum Words {
   // resource acquisition
   RESOURCE_REQUEST,
   RESOURCE_ANSWER,
   RESOURCE_ACK,
   RESOURCE_SENT,
   RESOURCE_DEN,

   // acceptance acquisition
   MEETING_REQUEST,
   MEETING_FINISHED,
   REQUEST_ACK,
   REQUEST_ALLOW,

   // organizing a meeting
   MEETING_JOIN,
   MEETING_ACCEPT,
   MEETING_DECLINE,
   MEETING_CANCEL,
   MEETING_ORG_ACCEPT,
   MEETING_NEW_ORG_PROBE,
   MEETING_NEW_ORG,
   MEETING_PARTC_ACK,
   MEETING_MERGE,

   NONE
};

typedef struct _Message {
   int __from__;
   int __to__;

   int  number;
   int  timestamp;
   int  sender;
   int  destination;
   Words word;

   int payload[8];
} Message;

int64_t identifier(Message &m);

Message create_message(int msg_number, int sender, int destination);
Message create_message(int msg_number, int sender, int destination, Words w);
Message create_message(int msg_number, int sender, int destination, Words w, int payload[]);

Message detach(Message other);


#endif //PR_MESSAGE_H
