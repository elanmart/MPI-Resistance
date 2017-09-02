#ifndef PR_COMM_H
#define PR_COMM_H

#include "common.h"
#include "config.h"
#include "message.h"
#include "node.h"

#define BUFFER_SIZE 4096 // Jakiś bufor dla MPI-aja
#define ROOT   0         // ID roota (MPI, nie w naszej strukturze)
#define NOTAG  0         // to jest używane w MPI-u. powinniśmy używać MPI_NO_TAG, ale się z czymś gryzło
#define ALL   (-1)       // jeśli chcę wysłać wiadomość do wszystkich, to daję destination == ALL

class Node;      // to się chyba nazywa forward declaration. Żeby kompilator nie się nie wypierdalał z rowerka.

class Manager {

// threading
unique_ptr<mutex> _incoming_queue_mutex = nullptr; // mutex na kolejkę wiadomości przychodzących
unique_ptr<mutex> _outgoing_queue_mutex = nullptr; // mutex na kolejkę wiadomości wychodzących

unique_ptr<thread> _sender_thread = nullptr;   // wątek obsługujący kolejkę wiadomości wychodzących
unique_ptr<thread> _reciever_thread = nullptr; // wątek obsługujący kolejkę wiadomości przychodzących

public:
   // ctors ----------------------------------------------------------------
   explicit Manager(Config cfg);
   ~Manager();
   void start();  // startuje wątki reader i writera

   // mpi-state ----------------------------------------------------------------
   int size_;     // MPI-size. Ile wątków ma MPI
   int rank_;     // MPI-rank. Którym wątkiem jestem

   // queues ----------------------------------------------------------------
   /*
    * Sprawdza czy w kolejce wejściowej jest wiadomość.
    * jeśli tak to ładuje ją do msg i zwraca True. Else zwraca False
   */
   bool get(Message *msg);

   /*
    * Wkłada wiadomość do kolejki wychodzącej
    */
   void put(Message msg, int dest);

   // communication ----------------------------------------------------------------
    /*
     * Służy do tworzenia drzewa. Nieistotne.
     */
   void send_node(Node& n, int dest);
    /*
     * Służy do tworzenia drzewa. Nieistotne
     */
   Node recv_node(int src);

   // utils
    /*
     * Używane w czasie tworzenia drzewa.
     */
   bool is_root();
    /*
     * Używane w czasie tworzenia drzewa.
     */
   int  root();

private:
   // mpi-ctors ----------------------------------------------------------------
   void mpi_init(); // nuda
   void mpi_exit(); // nuda

   void start_sender();   // startuje wątek wysyłający wiadomości po MPI
   void start_reciever(); // startuje wątek wkładający wiadomości z MPI do kolejki

   void _sender_loop();   // funkcja wykonywana przez sender thread
   void _reciever_loop(); // funkcja wykonywana przez reciever thread

   void _send_msg();      // pobiera msg z kolejki wychodzących i wysyła po MPI
   void _recv_msg();      // pobiera msg z MPI i wkłąda do kolejki przychodzących

   // queues
   queue<Message> incoming;  // kolejka przychodzących
   queue<Message> outgoing;  // kolejka wychodzących

   // configuration & datatypes
   Config       cfg_;         // jakiś config
   MPI_Datatype MSG_Dataype_; // Jeżeli chcemy wysyłać struktury po MPI, musimy stworzyć dla nich specjalny typ.

   // SYNC
   bool STOP_;  // flaga dla wątków.
};

/*
 * Na podstawie configu tworzy typ. Ten typ będzie przekazywany do MPI, żeby MPI
 * wiedział jak serializować struktury które będą wysyłane
 */
MPI_Datatype get_mpi_message_dtype(Config &cfg);

#endif //PR_COMM_H
