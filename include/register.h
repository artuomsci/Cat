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

  void reg(const Arrow &arrow_, const TFn &fn_);
  void unreg(const Arrow &arrow_);
  const TFn &get(const Arrow &arrow_);

private:
  Register() = default;

  std::map<Arrow, TFn> m_functions;
};
} // namespace cat
