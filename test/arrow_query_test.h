#ifndef ARROW_QUERY_TEST_H
#define ARROW_QUERY_TEST_H

#include <assert.h>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   //============================================================
   // Testing of arrow query (morphisms)
   //============================================================
   void test_arrow_query()
   {
      Arrow arrow(Arrow::EType::eFunctor, "A", "B");

      arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "a", "b"));
      arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "c", "d"));
      arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));
      arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "g", "f"));

      auto ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "b").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "b").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "*").AsQuery());
      assert(ret.size() == 1);
      assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery());
      assert(ret.size() == 4);

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f").AsQuery());
      assert(ret.size() == 2);

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*", "ef_arrow").AsQuery());
      assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f", "ef_arrow").AsQuery());
      assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

      ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f").AsQuery(), 1);
      assert(ret.size() == 1);
   }
}

#endif // ARROW_QUERY_TEST_H
