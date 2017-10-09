#include "message.h"

int64_t identifier(Message &m) {
    int64_t ID = 0;
    int64_t t = (int64_t) m.number;

    ID |= m.sender;
    ID |= (t << 32);

    return ID;
}


Message create_message(int msg_number, int sender, int destination, uint64_t T,
                       Words word,
                       const vector<int> &payload = DEFAULT_PAYLOAD) {

    Message msg = {
            -1,                         // __from__
            -1,                         // __to__
            T,                          // Lamport timestamp
            msg_number,                 // number
            sender,                     // sender
            destination,                // destination
            word,                       // word
    };

    for (int i = 0; i < PAYLOAD_SIZE; i++)
        msg.payload[i] = payload[i];

    return msg;
}
