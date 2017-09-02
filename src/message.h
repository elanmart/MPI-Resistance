#ifndef PR_MESSAGE_H
#define PR_MESSAGE_H

#include "common.h"

#define PAYLOAD_SIZE 8

enum Words {
   // resource acquisition
   RESOURCE_REQUEST,  // wysyłany przez node który chce risors
   RESOURCE_ANSWER,   // jeśli mam risors, to tym odpowiadam na RESOURCE_REQUEST
   RESOURCE_ACK,      // tym odpowiadame na RESOURCE_ANSWER jeśli chcę pobrać risors
   RESOURCE_SENT,     // tym odpowiadam na RESOURCE_ACK, potwierzam wysyłkę
   RESOURCE_DEN,      // tym odpowadam na RESOURCE_ANSWER jeśli nie chcę pobrać risorsa


    // acceptance acquisition // TODO -- JESZCZE NIE WIADOMO JAK TA KOMUNIKACJA POWINNA WYGLĄDAĆ
   MEETING_REQUEST,
   MEETING_FINISHED,
   REQUEST_ACK,
   REQUEST_ALLOW,

   // organizing a meeting // TODO -- JESZCZE NIE WIADOMO JAK TA KOMUNIKACJA POWINNA WYGLĄDAĆ
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
    /*
     * Adresy Node'a który przesyła wiadomość i adres Node'a do którego ma dotrzeć.
     * To jest low-lvl, MPI-specific
     * Te rzeczy są używane w komunikacji po MPI-u, zmieniają się za każdym razem jak pakiet
     * jest przesyłany pomiędzy parą wezłów.
     */
   int __from__;
   int __to__;

   int  number;      // Numer porządkowy wiadomości. Unikalny dla danego węzła.
   int  timestamp;   // Czas nadania wiadomości
   int  sender;      // Oryginalny twórca wiadomośći
   int  destination; // Ostateczny cel wiadomości. To nie dotyczy MPI, tylko drzewa. Może być "ALL"
   Words word;       // Zobacz enum wyżej. Mówi o tym co wiadomość ma oznaczać

   int payload[PAYLOAD_SIZE]; // Póki co nie używamy, ale może się przydać dodatkowy payload.
} Message;

/*
 * Tworzy nową wiadomość.
 */
Message create_message(int msg_number, int sender, int destination,
                       Words word = Words::NONE, int payload[] = nullptr);

/*
 * Zwraca unikalny identyfikator wiadomości, poprzez zapakowanie
 * `sender` oraz `number` w jeden int-64 (te pola są int-32, więc się mieszczą)
 */
int64_t identifier(Message &m);

/*
 * Póki co nieużywane. Resetuje pola __to__ i __from__.
 */
Message detach(Message other);


#endif //PR_MESSAGE_H
