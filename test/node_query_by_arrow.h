#pragma once

#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat
{
   //============================================================
   // Testing of arrow query (patterns)
   //============================================================
   void test_node_query_by_arrow()
   {
      auto src = R"(
SCAT cat
{
   OBJ a, b, c, d;

   a-[*]->b;
   a-[*]->c;
   c-[*]->d;
}
      )";

      Parser prs;
      prs.ParseSource(src);

      Node S = *prs.Data();

      Node ret = S.Query(Arrow("*", "*").AsQuery());
      assert(ret.QueryNodes("*").size() == 4);

      ret = S.Query(Arrow("a", "*").AsQuery());
      assert(ret.QueryNodes("*").size() == 3);
      assert(ret.QueryArrows(Arrow("a", "b").AsQuery()).size() == 1);
      assert(ret.QueryArrows(Arrow("a", "c").AsQuery()).size() == 1);

      ret = S.Query(Arrow("c", "d").AsQuery());
      assert(ret.QueryNodes("*").size() == 2);
      assert(ret.QueryArrows(Arrow("c", "d").AsQuery()).size() == 1);
   }
}
