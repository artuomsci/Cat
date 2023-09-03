#include "node.h"

#include "register.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Register &Register::Inst() {
  static Register reg;
  return reg;
}

//-----------------------------------------------------------------------------------------
void Register::Reg(const Arrow &arrow_, const TFn &fn_) {
  m_functions[arrow_] = fn_;
}

//-----------------------------------------------------------------------------------------
void Register::Unreg(const Arrow &arrow_) { m_functions.erase(arrow_); }

//-----------------------------------------------------------------------------------------
auto Register::Get(const Arrow &arrow_) -> const TFn & {
  static TFn stub = [](auto arg_) { return arg_; };
  auto it = m_functions.find(arrow_);
  if (it != m_functions.end()) {
    return it->second;
  }
  return stub;
}
