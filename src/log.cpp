#include "log.h"

using namespace cat;

ELogMode g_log_mode{ELogMode::eConsole};

//-----------------------------------------------------------------------------------------
void cat::set_log_mode(ELogMode mode_) { g_log_mode = mode_; }

//-----------------------------------------------------------------------------------------
ELogMode cat::get_log_mode() { return g_log_mode; }

//-----------------------------------------------------------------------------------------
void cat::print_error(const std::string &msg_) {
  if (g_log_mode == ELogMode::eConsole) {
    printf("Error: %s\n", msg_.c_str());
  }
}

//-----------------------------------------------------------------------------------------
void cat::print_info(const std::string &msg_) {
  if (g_log_mode == ELogMode::eConsole) {
    printf("Info: %s\n", msg_.c_str());
  }
}
