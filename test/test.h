#pragma once

#include <algorithm>
#include <assert.h>

#include "../include/log.h"
#include "../include/node.h"

#include "arrow_addition.h"
#include "arrow_application.h"
#include "arrow_application_native.h"
#include "arrow_associativity.h"
#include "arrow_composition.h"
#include "arrow_deletion.h"
#include "arrow_generator.h"
#include "arrow_inversion.h"
#include "arrow_query.h"
#include "arrow_query_on_node.h"
#include "arrow_sequence.h"
#include "arrow_validation.h"
#include "determination.h"
#include "exe_run.h"
#include "node_addition.h"
#include "node_deletion.h"
#include "node_initial_terminal.h"
#include "node_query.h"
#include "node_query_by_arrow.h"
#include "parsing.h"

#include "parser.h"

namespace cat {
void test() {
  ELogMode lmode = get_log_mode();

  set_log_mode(ELogMode::eQuiet);

  print_info("Start test");

  test_parsing();

  test_object_addition();

  test_object_deletion();

  test_arrow_addition();

  test_arrow_deletion();

  test_arrow_application();

  test_arrow_application_native();

  test_arrow_generator();

  test_composition();

  test_sequence();

  test_inversion();

  test_initial_terminal();

  test_arrow_query();

  test_arrow_query_on_node();

  test_node_query_by_arrow();

  test_node_query();

  test_associativity();

  test_arrow_validation();

  test_executor();

  test_determination();

  print_info("End test");

  set_log_mode(lmode);
}
} // namespace cat
