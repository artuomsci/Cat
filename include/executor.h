#pragma once

#include "cat_export.h"
#include "node.h"

namespace cat {

class CAT_EXPORT Executor {

public:
  static Executor &Inst();

  bool exec(Node &node_);

private:
  Executor() = default;
};
} // namespace cat
