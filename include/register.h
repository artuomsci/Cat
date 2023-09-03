#pragma once

#include <functional>
#include <map>

#include "cat_export.h"
#include "node.h"

namespace cat {

class CAT_EXPORT Register {

public:
  static Register &Inst();

  using TFn = std::function<TSetValue(TSetValue)>;

  void Reg(const Arrow &arrow_, const TFn &fn_);
  void Unreg(const Arrow &arrow_);
  const TFn &Get(const Arrow &arrow_);

private:
  Register() = default;

  std::map<Arrow, TFn> m_functions;
};
} // namespace cat
