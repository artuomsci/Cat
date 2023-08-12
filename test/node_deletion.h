#pragma once

#include <assert.h>

#include "../include/node.h"

namespace cat {
//============================================================
// Testing of object deletion methods
//============================================================
void test_object_deletion() {
  Node cat("cat", Node::EType::eSCategory);

  Node a("a", Node::EType::eObject), b("b", Node::EType::eObject),
      c("c", Node::EType::eObject);

  cat.AddNodes({a, b});

  cat.AddArrow(Arrow(a, b));

  assert(cat.EraseNode(a.Name()) == true);

  assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 1);

  assert(cat.EraseNode(b.Name()) == true);

  assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 0);

  assert(cat.EraseNode(c.Name()) == false);

  assert(cat.QueryNodes("*").size() == 0);

  cat.AddNodes({a, b});

  cat.EraseNodes();

  assert(cat.QueryNodes("*").size() == 0);
  assert(cat.QueryArrows("* -> *").size() == 0);
}
} // namespace cat
