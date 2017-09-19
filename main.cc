#include "src/node.h"
#include "src/utils.h"


vector<Node> create_tree(Config &cfg) {
   auto nodes = vector<Node>((uint) cfg.tree_size);
   for (int i = 0; i < nodes.size(); ++i) {
      nodes[i].ID_ = i;
   }

   int n   = cfg.tree_size;
   int cnt = 1;
   int idx = 0;
   int n_children;

   // topology
   while (cnt < n) {
      auto& node = nodes[idx];
      n_children = randint(1, min(n - cnt, cfg.max_children));

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

   // resource
   for (int i = 0; i < cfg.n_resources; ++i) {
      nodes[randint(0, (int) nodes.size())].resource_count_ += 1;
   }

   // acceptor token
   nodes[0].is_acceptor_ = 1;
   for (int i = 0; i < cfg.n_acceptors; ++i) {
      nodes[randint(0, (int) nodes.size())].is_acceptor_ = 1;
   }

   return nodes;
}

int main (int argc, char* argv[])
{
   Config  cfg(argc, argv);
   Manager manager(cfg);
   Node    local;

   if (manager.is_root()) {
      cfg.tree_size = manager.size_;

      auto tree = create_tree(cfg);
      local     = tree[0];

      for (int i = 1; i < manager.size_; ++i) {
         manager.send_node(tree[i], i);
      }

   } else {
      local = manager.recv_node(manager.root());
   }

   local.set_manager(&manager);
   local.start_event_loop();
   return 0;
}
