#ifndef NODE_INITIAL_TERMINAL_TEST_H
#define NODE_INITIAL_TERMINAL_TEST_H

#include <assert.h>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of initial/terminal objects
   //============================================================
   void test_initial_terminal()
   {
      Node cat("cat", Node::EType::eSCategory);

      Node     a0("a0", Node::EType::eObject)
            ,  a1("a1", Node::EType::eObject)
            ,  b("b", Node::EType::eObject)
            ,  c("c", Node::EType::eObject)
            ,  d0("d0", Node::EType::eObject)
            ,  d1("d1", Node::EType::eObject);

      cat.AddNodes({a0, a1, b, c, d0, d1});

      cat.AddArrows({Arrow(a0, a1), Arrow(a1, a0)});

      cat.EmplaceArrow(a0, b);
      cat.EmplaceArrow(a0, c);

      cat.EmplaceArrow(a1, b);
      cat.EmplaceArrow(a1, c);

      cat.EmplaceArrow(b, d0);
      cat.EmplaceArrow(c, d0);

      cat.EmplaceArrow(b, d1);
      cat.EmplaceArrow(c, d1);

      cat.AddArrows({Arrow(d0, d1), Arrow(d1, d0)});

      cat.SolveCompositions();

      Node::List initial_obj = cat.Initial();
      assert(initial_obj.size() == 2);
      initial_obj.sort();
      auto it_initial = initial_obj.begin();
      assert(*it_initial == a0);
      assert(*(++it_initial) == a1);

      Node::List terminal_obj = cat.Terminal();
      assert(terminal_obj.size() == 2);
      terminal_obj.sort();
      auto it_terminal = terminal_obj.begin();
      assert(*(it_terminal) == d0);
      assert(*(++it_terminal) == d1);

      cat.EraseNodes();

      cat.AddNode(a0);
      initial_obj = cat.Initial();
      terminal_obj = cat.Terminal();
      assert(initial_obj.size() == terminal_obj.size() == 1);
      assert(*initial_obj.begin() == *terminal_obj.begin());
   }
}

#endif // NODE_INITIAL_TERMINAL_TEST_H
