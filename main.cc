#include "src/node.h"
#include "src/config.h"
#include "src/utils.h"
#include "src/comm.h"


vector<Node> create_tree(int n, Config &cfg) {
   auto nodes = vector<Node>((uint) n);

   int cnt = 0;
   int idx = 0;
   int n_children;

   // topology
   while (cnt < n) {
      auto& node = nodes[idx];

      n_children       = randint(1, min(n - cnt, cfg.max_children));
      node.ID_         = idx;

      for (int i = 0; i < n_children; ++i) {
         nodes[cnt + i].parent_ = idx;
         nodes[cnt + i].level_  = node.level_ + 1;
         node.children_.insert(cnt+i);
      }

      cnt += n_children;
      idx += 1;
   }

   // neighbours
   int start{0}, stop{0};
   while (start < n) {
      start = stop;

      // TODO: If neighbours_ have to have the same parent_, just swap .level_ for .parent_
      while ((stop < nodes.size()) and (nodes[stop].level_ == nodes[start].level_)) {
         stop++;
      }

      int n1, n2;
      int diff = stop - start;
      for (int i = 0; i < diff * cfg.avg_neighbours; ++i) {
         n1 = (rand() % diff) + start;
         n2 = (rand() % diff) + start;
         if (n1 == n2)
            continue;

         nodes[n1].neighbours_.insert(n2);
         nodes[n2].neighbours_.insert(n1);
      }
   }

   return nodes;
}

int main (int argc, char* argv[])
{
   auto cfg     = Config(argc, argv);
   auto manager = Manager(cfg);
   Node    local;

   if (manager.is_root()) {
      auto tree = create_tree(manager.size_, cfg);
      local     = tree[0];

      for (int i = 1; i < manager.size_; ++i) {
         manager.send_node(tree[i], i);
      }

   } else {
      local = manager.recv_node(manager.root());
   }

   local.start_event_loop();
   return 0;
}
