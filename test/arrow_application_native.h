#pragma once

#include <assert.h>

#include "../include/node.h"
#include "parser.h"
#include "register.h"

namespace cat {
//============================================================
// Testing of native arrow application
//============================================================
void test_arrow_application_native() {

  Node monoid("int", Node::EType::eObject);
  monoid.SetValue(10);

  Arrow apply("int", "int", "inc");

  Register::Inst().reg(apply, [](TSetValue arg_) {
    return std::get<(int)ESetTypes::eInt>(arg_) + 1;
  });

  monoid = apply.Map(monoid).value();

  assert(std::get<(int)ESetTypes::eInt>(monoid.GetValue()) == 11);
}
} // namespace cat
