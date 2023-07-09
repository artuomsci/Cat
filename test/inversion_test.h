#ifndef INVERSION_TEST_H
#define INVERSION_TEST_H

#include <assert.h>
#include <algorithm>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of inversion
   //============================================================
   void test_inversion()
   {
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c, d});

         cat.EmplaceArrow(a, b, "f0");
         cat.EmplaceArrow(a, c, "f1");

         cat.EmplaceArrow(b, d, "f2");
         cat.EmplaceArrow(c, d, "f3");

         cat.Inverse();

         assert(cat.QueryArrows(Arrow("b", "a", "f0").AsQuery()).size() == 1);
         assert(cat.QueryArrows(Arrow("c", "a", "f1").AsQuery()).size() == 1);

         assert(cat.QueryArrows(Arrow("d", "b", "f2").AsQuery()).size() == 1);
         assert(cat.QueryArrows(Arrow("d", "c", "f3").AsQuery()).size() == 1);
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("c", "d");
         arrow.EmplaceArrow("e", "f", "ef_arrow");

         assert(arrow.IsInvertible());

         arrow.Inverse();

         assert(arrow.QueryArrows(Arrow("b", "a").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow("d", "c").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow("*", "*", "ef_arrow").AsQuery()).size() == 1);
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("c", "b");
         arrow.EmplaceArrow("e", "b");

         assert(!arrow.IsInvertible());
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("a", "d");
         arrow.EmplaceArrow("a", "f");

         assert(arrow.IsInvertible());
      }
   }
}

#endif // INVERSION_TEST_H
