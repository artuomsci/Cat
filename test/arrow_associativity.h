#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of associativity
//============================================================
void test_associativity() {
  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    assert(arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B", "fff");
    Arrow arrow_right("A", "B", "aaa");

    assert(arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "C");

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "C");

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    arrow_left.EmplaceArrow("a0", "b0");
    arrow_right.EmplaceArrow("a0", "b0");

    assert(arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    arrow_left.EmplaceArrow("a0", "b0");
    arrow_right.EmplaceArrow("a1", "b1");

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    arrow_left.EmplaceArrow("a0", "b0");
    arrow_left.EmplaceArrow("a1", "b1");

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    arrow_right.EmplaceArrow("a0", "b0");
    arrow_right.EmplaceArrow("a1", "b1");

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    Arrow arrow_0("a0", "b0");

    arrow_0.EmplaceArrow("x", "y", "first");
    arrow_0.EmplaceArrow("x", "y", "second");

    arrow_left.AddArrow(arrow_0);
    arrow_right.AddArrow(arrow_0);

    assert(arrow_left.IsAssociative(arrow_right));
  }

  {
    Arrow arrow_left("A", "B");
    Arrow arrow_right("A", "B");

    Arrow arrow_00("a0", "b0");
    arrow_00.EmplaceArrow("x", "y", "first");
    arrow_00.EmplaceArrow("x", "y", "second");

    Arrow arrow_01("a0", "b0");
    arrow_01.EmplaceArrow("x", "y", "first");
    arrow_01.EmplaceArrow("f", "g", "second");

    arrow_left.AddArrow(arrow_00);
    arrow_right.AddArrow(arrow_01);

    assert(!arrow_left.IsAssociative(arrow_right));
  }

  {
    auto src = R"(
   LCAT LCat
   {
      SCAT A
      {
         OBJ a0, a1;
      }

      SCAT B
      {
         OBJ b0, b1;
      }

      SCAT C
      {
         OBJ c0, c1;
      }

      SCAT D
      {
         OBJ d0, d1;
      }

      A -[*]-> B
      {
         a0 -[*]-> b0 {};
         a1 -[*]-> b1 {};
      }

      B -[*]-> D
      {
         b0 -[*]-> d0 {};
         b1 -[*]-> d1 {};
      }

      A -[*]-> C
      {
         a0 -[*]-> c0 {};
         a1 -[*]-> c1 {};
      }

      C -[*]-> D
      {
         c0 -[*]-> d1 {};
         c1 -[*]-> d0 {};
      }
   }
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    cat.SolveCompositions();

    Arrow::List arrows = cat.QueryArrows(Arrow("A", "D", "*").AsQuery());
    assert(!arrows.front().IsAssociative(arrows.back()));
  }
}
} // namespace cat
