#pragma once

#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "../include/log.h"

#include "node_query.h"
#include "node_initial_terminal.h"
#include "composition.h"
#include "inversion.h"
#include "arrow_query.h"
#include "sequence.h"
#include "node_query_by_arrow.h"
#include "arrow_query_on_node.h"
#include "equivalence.h"
#include "arrow_validation.h"
#include "arrow_generator.h"
#include "arrow_application.h"
#include "object_addition.h"
#include "object_deletion.h"
#include "arrow_addition.h"
#include "arrow_deletion.h"

namespace cat
{
   void test()
   {
      ELogMode lmode = get_log_mode();

      set_log_mode(ELogMode::eQuiet);

      print_info("Start test");

      test_object_addition();

      test_object_deletion();

      test_morphism_addition();

      test_morphism_deletion();

      test_arrow_application();

      test_arrow_generator();

      test_composition();

      test_sequence();

      test_inversion();

      test_initial_terminal();

      test_arrow_query();

      test_arrow_query_on_node();

      test_node_query_by_arrow();

      test_node_query();

      test_equivalence();

      test_arrow_validation();

      print_info("End test");

      set_log_mode(lmode);
   }
}
