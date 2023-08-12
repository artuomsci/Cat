#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of inversion
//============================================================
void test_inversion() {
  {
    auto src = R"(
SCAT cat
{
   OBJ a, b, c, d;

   a-[f0]->b{};
   a-[f1]->c{};

   b-[f2]->d{};
   c-[f3]->d{};
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    cat.Inverse();

    assert(cat.QueryArrows(Arrow("b", "a", "f0").AsQuery()).size() == 1);
    assert(cat.QueryArrows(Arrow("c", "a", "f1").AsQuery()).size() == 1);

    assert(cat.QueryArrows(Arrow("d", "b", "f2").AsQuery()).size() == 1);
    assert(cat.QueryArrows(Arrow("d", "c", "f3").AsQuery()).size() == 1);
  }

  {
    Arrow arrow("A", "B");

    arrow.EmplaceArrow("a", "b");
    arrow.EmplaceArrow("c", "d");
    arrow.EmplaceArrow("e", "f", "ef_arrow");

    assert(arrow.IsInvertible());

    arrow.Inverse();

    assert(arrow.QueryArrows(Arrow("b", "a").AsQuery()).size() == 1);
    assert(arrow.QueryArrows(Arrow("d", "c").AsQuery()).size() == 1);
    assert(arrow.QueryArrows(Arrow("*", "*", "ef_arrow").AsQuery()).size() ==
           1);
  }

  {
    Arrow arrow("A", "B");

    arrow.EmplaceArrow("a", "b");
    arrow.EmplaceArrow("c", "b");
    arrow.EmplaceArrow("e", "b");

    assert(!arrow.IsInvertible());
  }

  {
    Arrow arrow("A", "B");

    arrow.EmplaceArrow("a", "b");
    arrow.EmplaceArrow("a", "d");
    arrow.EmplaceArrow("a", "f");

    assert(arrow.IsInvertible());
  }
}
} // namespace cat
