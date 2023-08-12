#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of mapping generator
//============================================================
void test_arrow_generator() {
  {
    auto src = R"(
LCAT Cat
{
   SCAT A
   {
      OBJ a0, a1;
   }

   SCAT B
   {
      OBJ b0, b1;
   }
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node ccat = *prs.Data();

    Arrow::List ret = ccat.ProposeArrows("A", "B");

    assert(ret.size() == 4);

    auto fnFindArrow = [&](Arrow arrow_, const std::string &source_,
                           const std::string &target_) {
      auto morphisms = arrow_.QueryArrows(Arrow("*", "*").AsQuery());

      auto it = std::find_if(
          morphisms.begin(), morphisms.end(), [&](const Arrow &arrow) {
            return arrow.Source() == source_ && arrow.Target() == target_;
          });

      return it != ret.end();
    };

    auto head = ret.begin();

    assert(fnFindArrow(*head, "a0", "b0"));
    assert(fnFindArrow(*head, "a1", "b0"));

    head++;

    assert(fnFindArrow(*head, "a0", "b1"));
    assert(fnFindArrow(*head, "a1", "b0"));

    head++;

    assert(fnFindArrow(*head, "a0", "b0"));
    assert(fnFindArrow(*head, "a1", "b1"));

    head++;

    assert(fnFindArrow(*head, "a0", "b1"));
    assert(fnFindArrow(*head, "a1", "b1"));
  }

  {
    auto src = R"(
LCAT Cat
{
   SCAT A
   {
      OBJ a;
   }

   SCAT B
   {
      OBJ b0, b1, b2;
   }
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node ccat = *prs.Data();

    Arrow::List ret = ccat.ProposeArrows("A", "B");

    assert(ret.size() == 3);

    ret = ccat.ProposeArrows("B", "A");

    assert(ret.size() == 1);
  }
}
} // namespace cat
