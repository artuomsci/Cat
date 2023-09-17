#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Test of determination problem
//============================================================
void test_determination() {

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

   A -[*]-> B
   {
      a0 -[*]-> b0 {};
      a1 -[*]-> b1 {};
      a2 -[*]-> b1 {};
   }

   A -[*]-> C
   {
      a0 -[*]-> c0 {};
      a1 -[*]-> c1 {};
      a2 -[*]-> c1 {};
   }
}
         )";

  Parser prs;
  prs.ParseSource(src);

  Node cat = *prs.Data();

  Arrow::List determ =
      cat.SolveDetermination(Arrow::AName("A_B"), Arrow::AName("A_C"));
  assert(determ.size() == 1);

  Arrow detBC = determ.front();

  assert(detBC.Source() == "B");
  assert(detBC.Target() == "C");
  assert(detBC.QueryArrows(Arrow("b0", "c0", "*").AsQuery()).size() == 1);
  assert(detBC.QueryArrows(Arrow("b1", "c1", "*").AsQuery()).size() == 1);
}
} // namespace cat
