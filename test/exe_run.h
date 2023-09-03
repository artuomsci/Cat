#pragma once

#include <assert.h>

#include "../include/node.h"
#include "executor.h"
#include "parser.h"
#include "register.h"

namespace cat {
//============================================================
// Testing of executor
//============================================================
void test_executor() {

  Node cat("cat", Node::EType::eSCategory);

  Node a("a", Node::EType::eObject);
  Node b("b", Node::EType::eObject);
  Node c("c", Node::EType::eObject);

  a.SetValue(2);

  Arrow ab("a", "b", "incr");
  Arrow bc("b", "c", "mlt");

  cat.AddNodes({a, b, c});
  cat.AddArrows({ab, bc});

  cat.SolveCompositions();

  Register::Inst().Reg(ab, [](TSetValue val) {
    return std::get<(int)ESetTypes::eInt>(val) + 4;
  });
  Register::Inst().Reg(bc, [](TSetValue val) {
    return std::get<(int)ESetTypes::eInt>(val) * 2;
  });

  assert(Executor::Inst().Exec(cat));

  Node::List retList = cat.QueryNodes("c");
  auto control = std::get<(int)ESetTypes::eInt>(retList.front().GetValue());
  assert(control == 12);
}
} // namespace cat
