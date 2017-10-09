#ifndef PR_MESSAGE_H
#define PR_MESSAGE_H

#include "common.h"

#define PAYLOAD_SIZE 8
static vector<int> DEFAULT_PAYLOAD {0, 0, 0, 0, 0, 0, 0, 0};


enum Words {
    // Resource acquisition
    RESOURCE_REQUEST,
    RESOURCE_ANSWER,
    RESOURCE_ACK,
    RESOURCE_SENT,
    RESOURCE_DENIED,

    // Logic similar to resource acquisition
    MEETING_ACCEPTANCE_REQUEST,
    MEETING_ACCEPTANCE_REPORT,
    MEETING_ACCEPTANCE_GRANTED,
    MEETING_ACCEPTANCE_DENIED,
    MEETING_ACCEPTANCE_FULFILLED,

    ACCEPTOR_PASS_OFFER,
    ACCEPTOR_PASS_ACK,
    ACCEPTOR_PASS_DENY,
    ACCEPTOR_PASS_CONFIRM,
    ACCEPTOR_PASS_CANCEL,
    ACCEPTOR_PASS_TEST,
    ACCEPTOR_PASS_RELEASE,

    // Organizing a meeting
    MEETING_INVITE,
    MEETING_ACCEPT,
    MEETING_DECLINE,
    MEETING_CANCEL,
    MEETING_START,
    MEETING_END,
    MEETING_DONE,

    NONE
};

static const char * EnumStrings[] = {
        "RESOURCE_REQUEST",
        "RESOURCE_ANSWER",
        "RESOURCE_ACK",
        "RESOURCE_SENT",
        "RESOURCE_DENIED",

        "MEETING_ACCEPTANCE_REQUEST",
        "MEETING_ACCEPTANCE_REPORT",
        "MEETING_ACCEPTANCE_GRANTED",
        "MEETING_ACCEPTANCE_DENIED",
        "MEETING_ACCEPTANCE_FULFILLED",

        "ACCEPTOR_PASS_OFFER",
        "ACCEPTOR_PASS_ACK",
        "ACCEPTOR_PASS_DENY",
        "ACCEPTOR_PASS_CONFIRM",
        "ACCEPTOR_PASS_CANCEL",
        "ACCEPTOR_PASS_TEST",
        "ACCEPTOR_PASS_RELEASE",

        "MEETING_INVITE",
        "MEETING_ACCEPT",
        "MEETING_DECLINE",
        "MEETING_CANCEL",
        "MEETING_START",
        "MEETING_END",
        "MEETING_DONE",

        "NONE"
};

typedef struct _Message {
    int __from__;
    int __to__;

    uint64_t timestamp;
    int number;
    int sender;
    int destination;
    Words word;

    int payload[PAYLOAD_SIZE];
} Message;

Message create_message(int msg_number, int sender, int destination, uint64_t T,
                       Words word,
                       const vector<int> &payload = DEFAULT_PAYLOAD);

int64_t identifier(Message &m);


#endif //PR_MESSAGE_H
