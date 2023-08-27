#pragma once

#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of arrow application
//============================================================
void test_arrow_application() {
  {
    auto ret = Arrow("A", "B").Map(Node("A", Node::EType::eSCategory));
    assert(ret.has_value());
    assert(ret->Name() == "B");
  }

  {
    auto ret = Arrow("a", "b").Map(Node("a", Node::EType::eObject));
    assert(ret.has_value());
    assert(ret->Name() == "b");
  }

  {
    auto ret = Arrow("0", "2").Map(Node("0", Node::EType::eSet));
    assert(ret.has_value());
    assert(ret->Name() == "2");
  }

  {
    auto src = R"(
SCAT A
{
  OBJ a0, a1;

  a0-[*]->a1
  {
    0 -[*]-> 2 {};
    1 -[*]-> 4 {};
  };
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node A = *prs.Data();

    Arrow arrow("A", "B");
    arrow.EmplaceArrow("a0", "b0");
    arrow.EmplaceArrow("a1", "b1");

    auto ret = arrow.Map(A);
    assert(ret->Name() == "B");
    assert(ret->QueryNodes("*").size() == 2);

    assert(!ret->QueryNodes("b0").empty());
    assert(ret->QueryNodes("b0").front().Name() == "b0");

    assert(!ret->QueryNodes("b1").empty());
    assert(ret->QueryNodes("b1").front().Name() == "b1");

    auto arrows = ret->QueryArrows(Arrow("b0", "b1", "*").AsQuery());
    assert(arrows.size() == 1);
    assert(arrows.front().Source() == "b0");
    assert(arrows.front().Target() == "b1");
    assert(arrows.front().QueryArrows(Arrow("0", "2", "*").AsQuery()).size() ==
           1);
    assert(arrows.front().QueryArrows(Arrow("1", "4", "*").AsQuery()).size() ==
           1);
  }
}
} // namespace cat
