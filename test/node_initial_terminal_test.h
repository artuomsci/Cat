#ifndef NODE_INITIAL_TERMINAL_TEST_H
#define NODE_INITIAL_TERMINAL_TEST_H

#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat
{
   //============================================================
   // Testing of initial/terminal objects
   //============================================================
   void test_initial_terminal()
   {
      auto src = R"(
SCAT cat
{
   OBJ a0, a1, b, c, d0, d1;

   a0-[*]->a1;
   a1-[*]->a0;

   a0-[*]->b;
   a0-[*]->c;

   a1-[*]->b;
   a1-[*]->c;

   b-[*]->d0;
   c-[*]->d0;

   b-[*]->d1;
   c-[*]->d1;

   d0-[*]->d1;
   d1-[*]->d0;
}
      )";

      Parser prs;
      prs.ParseSource(src);

      Node cat = *prs.Data();

      cat.SolveCompositions();

      Node::List initial_obj = cat.Initial();
      assert(initial_obj.size() == 2);
      initial_obj.sort();
      auto it_initial = initial_obj.begin();
      assert(it_initial->Name() == "a0");
      assert((++it_initial)->Name() == "a1");

      Node::List terminal_obj = cat.Terminal();
      assert(terminal_obj.size() == 2);
      terminal_obj.sort();
      auto it_terminal = terminal_obj.begin();
      assert(it_terminal->Name() == "d0");
      assert((++it_terminal)->Name() == "d1");

      cat.EraseNodes();

      cat.AddNode(Node("a0", Node::EType::eObject));
      initial_obj = cat.Initial();
      terminal_obj = cat.Terminal();
      assert(initial_obj.size() == terminal_obj.size() == 1);
      assert(*initial_obj.begin() == *terminal_obj.begin());
   }
}

#endif // NODE_INITIAL_TERMINAL_TEST_H
