#pragma once

#include <string>

#include "cat_export.h"

namespace cat {
enum class ELogMode { eQuiet, eConsole };

CAT_EXPORT void set_log_mode(ELogMode mode_);
CAT_EXPORT ELogMode get_log_mode();
CAT_EXPORT void print_error(const std::string &msg_);
CAT_EXPORT void print_info(const std::string &msg_);
} // namespace cat
