#ifndef PR_NODE_H
#define PR_NODE_H

#include <mutex>
#include "common.h"
#include "comm.h"

class Manager; // todo: handle this;

enum class ResourceState { IDLE, WAITING, LOCKED};
enum class MeetingState  { IDLE, WAITING, LOCKED, MASTER_ORG, SLAVE_ORG};

class Node {
public:
   // ctors
   Node();            // tworzy pusty węzeł, bez risorsów, wiadomości, dzieci itd.
   Node(int ID);      // jw., ale dodaje ID
   Node(int* buffer); // [NIEISTOTNE] używany podczas tworzenia drzewa. Deserializuje Node z MPI

   // identity
   int32_t  ID_;    // ID węzła
   int32_t  level_; // Głębokość w drzewie

   // topology
   int32_t      parent_;     // rodzic w drzewie
   set<int32_t> neighbours_; // sąsiedzi w drzewie
   set<int32_t> children_;   // dzieci w drzewie

   // serialization
   pair<int, int*> serialize();              // [NIEISTOTNE] używane podczas tworzenia drzewa.
   void            deserialize(int* buffer); // [NIEISTOTNE] używane podczas tworzenia drzewa

   // communication interface
   Manager* manager_ = nullptr;  // Interfejs do MPI. Posiada kolejki z których czytamy i do których wkładamy msg
   void set_manager(Manager *m); // ustaia managera. Chyba nie da się tego inaczej zrobić

   // tasks //TODO: WIP. to są jakieś flagi i county które mogą być przydatne.
   int           resource_count_;
   int           awaiting_response_;
   int           time_penalty_;
   set<int>      participants_;
   set<int>      perhaps_merge_orgs_;
   MeetingState  meeting_state_;
   ResourceState resource_state_;

   // generic communication
   void start_event_loop(); // startuje Node'a. Czyli nasłuchiwanie, wysyłanie, organizacja spotkań itd.

    /*
     * Tworzy nową wiadomość o znaczeniu `w` skierowaną do `destination`
     */
   void new_message(int destination, Words w, int payload[] = nullptr);
    /*
     * Wysyła gotową wiadomość
     */
   void _send(Message msg);

    /*
     * Wywołuje Manager->get
     */
   bool get(Message *msg);

   void broadcast(Message msg); // Wysyła wiadomość do wszystkich sąsiadów i dzieci oraz do rodzica
   bool accept(Message &msg);   // Sprawdza czy nie dostaliśmy już wcześniej widzianej wiadomości
   void consume(Message &msg);  // Sprawdza czy wiadomość jest do nas / powinna zostać przekazana

   void send_to(Message msg, set<int> recipients); // helper. Wysyła msg do kilku innych na raz
   void send_to(Message msg, int dest); // wysyła wiadomość do "dest"

   // message bookkeeping
   set<int64_t> msg_cache_; // widziane wiadomośći
   int msg_number_;         // aktualny numer porządkowy, bęðzie przypisany kolejne wygenerowanej msg.

   // special skills
   void pass_acceptor();      // nie zimplementowane
   void initialzie_meeting(); // nie zimplementowane
   void get_resource();       // Pobiera risors od kogoś w drzewie
   void handle(Message msg);  // Przetwarza wiadomość i generuje odpowiedź jeśli to konieczn
   void get_accepatnce();     // nie zaimplementowane
   void meet();               // nie zaimplementowane

   // synchronization
   bool STOP_;                // czy powinniśmy przestać.
};

// Helper żeby łatwiej logować. Wywoływac jak printf.
#define DASH "=====================================\n"
#define LOG(msg, ...) printf(DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n"  \
                             DASH,                \
                             this->ID_, ##__VA_ARGS__)

#endif //PR_NODE_H
