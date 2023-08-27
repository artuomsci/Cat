#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
//============================================================
// Testing of parsing
//============================================================
void test_parsing() {

  {
    auto src = R"(
LCAT Cat
{
  SCAT A
  {
    OBJ a0, a1;

    a0 -[*]-> a1 {};
  }

  SCAT B
  {
    OBJ b0, b1;

    b0 -[*]-> b1 {};
  }

  A -[*]-> B
  {
    a0 -[*]-> b0 {};
    a1 -[*]-> b1 {};
  }
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node node = *prs.Data();

    Arrow::List functors = node.QueryArrows(Arrow("A", "B", "*").AsQuery());
    assert(functors.size() == 1);
    assert(
        functors.front().QueryArrows(Arrow("a0", "b0", "*").AsQuery()).size() ==
        1);
    assert(
        functors.front().QueryArrows(Arrow("a1", "b1", "*").AsQuery()).size() ==
        1);

    assert(node.Name() == "Cat");

    Node::List categories = node.QueryNodes("*");
    assert(categories.size() == 2);

    auto itA = std::find_if(categories.begin(), categories.end(),
                            [](auto element) { return element.Name() == "A"; });
    assert(itA != categories.end());

    Node::List objectsA = itA->QueryNodes("*");

    assert(std::find_if(objectsA.begin(), objectsA.end(), [](auto element) {
             return element.Name() == "a0";
           }) != objectsA.end());

    assert(std::find_if(objectsA.begin(), objectsA.end(), [](auto element) {
             return element.Name() == "a1";
           }) != objectsA.end());

    auto itB = std::find_if(categories.begin(), categories.end(),
                            [](auto element) { return element.Name() == "B"; });
    assert(itB != categories.end());

    Node::List objectsB = itB->QueryNodes("*");

    assert(std::find_if(objectsB.begin(), objectsB.end(), [](auto element) {
             return element.Name() == "b0";
           }) != objectsB.end());

    assert(std::find_if(objectsB.begin(), objectsB.end(), [](auto element) {
             return element.Name() == "b1";
           }) != objectsB.end());
  }
}
} // namespace cat
