#ifndef ARROW_QUERY_TEST_H
#define ARROW_QUERY_TEST_H

#include <assert.h>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of arrow query (morphisms)
   //============================================================
   void test_arrow_query()
   {
      Arrow arrow("A", "B");

      arrow.EmplaceArrow("a", "b");
      arrow.EmplaceArrow("c", "d");
      arrow.EmplaceArrow("e", "f", "ef_arrow");
      arrow.EmplaceArrow("g", "f");

      auto ret = arrow.QueryArrows(Arrow("a", "b").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow("a", "b"));

      ret = arrow.QueryArrows(Arrow("*", "b").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow("a", "b"));

      ret = arrow.QueryArrows(Arrow("a", "*").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow("a", "b"));

      ret = arrow.QueryArrows(Arrow("*", "*").AsQuery());
      assert(ret.size() == 4);

      ret = arrow.QueryArrows(Arrow("*", "f").AsQuery());
      assert(ret.size() == 2);

      ret = arrow.QueryArrows(Arrow("*", "*", "ef_arrow").AsQuery());
      assert(ret.front() == Arrow("e", "f", "ef_arrow"));

      ret = arrow.QueryArrows(Arrow("*", "f", "ef_arrow").AsQuery());
      assert(ret.front() == Arrow("e", "f", "ef_arrow"));

      ret = arrow.QueryArrows(Arrow("*", "f").AsQuery(), 1);
      assert(ret.size() == 1);
   }
}

#endif // ARROW_QUERY_TEST_H
