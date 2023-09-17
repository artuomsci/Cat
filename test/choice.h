#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Test of choice problem
//============================================================
void test_choice() {

  auto src = R"(
LCAT cat
{
   SCAT A
   {
      OBJ a0, a1, a2;
   }

   SCAT B
   {
      OBJ b0, b1;
   }

   SCAT C
   {
      OBJ c0, c1;
   }

   A -[*]-> C
   {
      a0 -[*]-> c0 {};
      a1 -[*]-> c1 {};
      a2 -[*]-> c1 {};
   }

   B -[*]-> C
   {
      b0 -[*]-> c0 {};
      b1 -[*]-> c1 {};
   }
}
         )";

  Parser prs;
  prs.ParseSource(src);

  Node cat = *prs.Data();

  Arrow::List determ =
      cat.SolveChoice(Arrow::AName("B_C"), Arrow::AName("A_C"));
  assert(determ.size() == 1);

  Arrow detBC = determ.front();

  assert(detBC.Source() == "A");
  assert(detBC.Target() == "B");
  assert(detBC.QueryArrows(Arrow("a0", "b0", "*").AsQuery()).size() == 1);
  assert(detBC.QueryArrows(Arrow("a1", "b1", "*").AsQuery()).size() == 1);
  assert(detBC.QueryArrows(Arrow("a2", "b1", "*").AsQuery()).size() == 1);
}
} // namespace cat
