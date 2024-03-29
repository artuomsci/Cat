#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of morphism compositions
//============================================================
void test_composition() {
  {
    auto src = R"(
SCAT cat
{
   OBJ a, b, c, d;

   a-[f0]->b{};
   b-[f1]->c{};
   c-[f2]->d{};
   d-[f3]->c{};
   c-[f4]->b{};
   b-[f5]->a{};
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    cat.SolveCompositions();

    cat.SolveCompositions();

    cat.SolveCompositions();

    assert(!cat.QueryArrows(Arrow("a", "c", "*").AsQuery()).empty());
    assert(!cat.QueryArrows(Arrow("a", "d", "*").AsQuery()).empty());
    assert(!cat.QueryArrows(Arrow("b", "d", "*").AsQuery()).empty());

    assert(!cat.QueryArrows(Arrow("d", "b", "*").AsQuery()).empty());
    assert(!cat.QueryArrows(Arrow("d", "a", "*").AsQuery()).empty());
    assert(!cat.QueryArrows(Arrow("c", "a", "*").AsQuery()).empty());

    auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
    cat.SolveCompositions();
    auto new_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();

    assert(new_count == prev_count);
  }

  {
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

   A -[*]-> B
   {
      a0 -[*]-> b0 {};
      a1 -[*]-> b1 {};
      a2 -[*]-> b1 {};
   }

   B -[*]-> A
   {
      b0 -[*]-> a0 {};
      b1 -[*]-> a1 {};
   }
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    cat.SolveCompositions();

    Arrow::List arrows = cat.QueryArrows(Arrow("*", "*").AsQuery());
    assert(arrows.size() == 6);

    Arrow::List arrowsAA = cat.QueryArrows(Arrow("A", "A", "*").AsQuery());
    assert(arrowsAA.size() == 2);

    Arrow::List arrowsBB = cat.QueryArrows(Arrow("B", "B", "*").AsQuery());
    assert(arrowsBB.size() == 2);
  }

  {
    Arrow f0("A", "B");
    f0.AddArrow(Arrow("a0", "b0"));
    f0.AddArrow(Arrow("a1", "b1"));

    Arrow f1("B", "C");
    f1.AddArrow(Arrow("b0", "c0"));
    f1.AddArrow(Arrow("b1", "c1"));

    std::optional<Arrow> cmp = f1.Compose(f0);
    assert(cmp.has_value());
    assert(cmp.value().Source() == "A");
    assert(cmp.value().Target() == "C");
    assert(cmp.value().QueryArrows(Arrow("a0", "c0", "*").AsQuery()).size() ==
           1);
    assert(cmp.value().QueryArrows(Arrow("a1", "c1", "*").AsQuery()).size() ==
           1);
  }
}
} // namespace cat
