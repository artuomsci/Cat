#pragma once

#include <assert.h>

#include "../include/node.h"

namespace cat {
//============================================================
// Testing of object addition methods
//============================================================
void test_object_addition() {
  {
    Node cat("cat", Node::EType::eSCategory);

    Node a("a", Node::EType::eObject), b("b", Node::EType::eObject);

    Node A("A", Node::EType::eObject);

    Node PK(u8"Павка Корчагин", Node::EType::eObject);

    assert(cat.QueryNodes("*").size() == 0);

    assert(cat.AddNodes({a, b, A, PK}));

    assert(cat.QueryNodes("*").size() == 4);

    assert(cat.AddNode(a) == false);

    assert(cat.QueryNodes("*").size() == 4);
  }

  {
    Node cat("cat", Node::EType::eSCategory);

    Node a("a", Node::EType::eObject);

    assert(cat.AddNode(a));

    assert(cat.QueryNodes("*").size() == 1);

    assert(cat.AddNode(a) == false);
  }
}
} // namespace cat
