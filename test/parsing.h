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

    a0 -[*]-> a1
    {
      0 -[*]-> 2 {};
      1 -[*]-> 4 {};
    };
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

    // Checking functor
    Arrow::List functors = node.QueryArrows(Arrow("A", "B", "*").AsQuery());
    assert(functors.size() == 1);
    // Checking morphisms inside functor
    assert(
        functors.front().QueryArrows(Arrow("a0", "b0", "*").AsQuery()).size() ==
        1);
    assert(
        functors.front().QueryArrows(Arrow("a1", "b1", "*").AsQuery()).size() ==
        1);

    assert(node.Name() == "Cat");

    // Checking categories
    Node::List categories = node.QueryNodes("*");
    assert(categories.size() == 2);

    // Checking category A
    auto itA = std::find_if(categories.begin(), categories.end(),
                            [](auto element) { return element.Name() == "A"; });
    assert(itA != categories.end());

    // Checking morphisms inside category
    Arrow::List morphismsA = itA->QueryArrows(Arrow("a0", "a1", "*").AsQuery());
    assert(morphismsA.size() == 1);

    // Checking functions inside morphism
    assert(
        morphismsA.front().QueryArrows(Arrow("0", "2", "*").AsQuery()).size() ==
        1);
    assert(
        morphismsA.front().QueryArrows(Arrow("1", "4", "*").AsQuery()).size() ==
        1);

    // Checking objects inside category
    Node::List objectsA = itA->QueryNodes("*");

    assert(std::find_if(objectsA.begin(), objectsA.end(), [](auto element) {
             return element.Name() == "a0";
           }) != objectsA.end());

    assert(std::find_if(objectsA.begin(), objectsA.end(), [](auto element) {
             return element.Name() == "a1";
           }) != objectsA.end());

    // Checking category B
    auto itB = std::find_if(categories.begin(), categories.end(),
                            [](auto element) { return element.Name() == "B"; });
    assert(itB != categories.end());

    // Checking morphisms inside category
    Arrow::List morphismsB = itB->QueryArrows(Arrow("b0", "b1", "*").AsQuery());
    assert(morphismsB.size() == 1);

    // Checking objects inside category
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
