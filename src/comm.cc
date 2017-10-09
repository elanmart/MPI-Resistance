#include "comm.h"

// --- ctors ---

Manager::Manager(Config cfg) {
   mpi_init();

   cfg_         = cfg;
   MSG_Dataype_ = get_mpi_message_dtype(cfg_);

   _incoming_queue_mutex = unique_ptr<mutex>(new mutex());
   _outgoing_queue_mutex = unique_ptr<mutex>(new mutex());

   STOP_        = false;
}

Manager::~Manager() {
   mpi_exit();
}

void Manager::mpi_init() {
   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size_);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
}

void Manager::mpi_exit() {
   MPI_Finalize();
}

void Manager::start() {
   this->start_sender();
   this->start_reciever();
}

// --- setup ---

void Manager::send_node(Node &n, int dest, Config& cfg) {
   auto msg = n.serialize(cfg);
   MPI_Send(msg.second, msg.first, MPI_INT, dest, NOTAG, MPI_COMM_WORLD);
}

Node Manager::recv_node(int src) {
   int* buffer = new int[BUFFER_SIZE];
   MPI_Recv(buffer, BUFFER_SIZE, MPI_INT, src, NOTAG, MPI_COMM_WORLD, NULL);

   Node n = Node(buffer);

   return n;
}

// --- queues ---

bool Manager::get(Message *msg) {
   std::lock_guard<std::mutex> guard(*_incoming_queue_mutex);

   if (not incoming.empty()) {
      (*msg) = incoming.front();
      incoming.pop();

      return true;
   }

   return false;
}

void Manager::put(Message msg, int dest) {
   std::lock_guard<std::mutex> guard(*_outgoing_queue_mutex);

   if (msg.__from__ != dest and dest >= 0) {
      msg.__to__ = dest;
      outgoing.push(msg);
   }
}

// --- comms ---
void Manager::start_sender() {
   _sender_thread = unique_ptr<thread>(new thread(&Manager::_sender_loop, this));
}

void Manager::start_reciever() {
   _reciever_thread = unique_ptr<thread>(new thread(&Manager::_reciever_loop, this));
}

void Manager::_sender_loop() {
   while (not STOP_) {
      _send_msg();
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }
}

void Manager::_send_msg() {
   std::lock_guard<std::mutex> guard(*_outgoing_queue_mutex);

   if (not outgoing.empty()){
      Message msg = outgoing.front();
      MPI_Send(&msg, 1, MSG_Dataype_, msg.__to__, NOTAG, MPI_COMM_WORLD);

      outgoing.pop();
   }
}

void Manager::_reciever_loop() {
   while (not STOP_) {
      _recv_msg();
   }
}

void Manager::_recv_msg() {
   Message msg;
   MPI_Status status;

   MPI_Recv(&msg, 1, MSG_Dataype_, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   int src = status.MPI_SOURCE;
   msg.__from__ = src;

   std::lock_guard<std::mutex> guard(*_incoming_queue_mutex);
   incoming.push(msg);
}

// --- utils ---

bool Manager::is_root() {
   return (rank_ == ROOT);
}

int Manager::root() {
   return ROOT;
}

MPI_Datatype get_mpi_message_dtype(Config &cfg) {
   const int k = 6;

   int          sizes[k]   = {1, 1, 1, 1, 1, 8};
   MPI_Datatype types[k]   = {MPI_UINT64_T, MPI_INT,  MPI_INT,  MPI_INT, MPI_INT, MPI_INT};
   MPI_Aint     offsets[k] = {offsetof(Message, timestamp), offsetof(Message, number),
                              offsetof(Message, sender), offsetof(Message, destination),
                              offsetof(Message, word),   offsetof(Message, payload)};

   MPI_Datatype dtype;
   MPI_Type_create_struct(k, sizes, offsets, types, &dtype);
   MPI_Type_commit(&dtype);

   return dtype;
}

