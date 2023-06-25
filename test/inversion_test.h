#ifndef INVERSION_TEST_H
#define INVERSION_TEST_H

#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   //============================================================
   // Testing of inversion
   //============================================================
   void test_inversion()
   {
      auto fnCheckArrow = [](const Arrow::List& morphs_, const Arrow& morph_)
      {
         auto it = std::find_if(morphs_.begin(), morphs_.end(), [&](const Arrow::List::value_type& element_)
         {
            return element_.Name() == morph_.Name() && element_.Source() == morph_.Source() && element_.Target() == morph_.Target();
         });

         return it != morphs_.end();
      };

      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c, d});

         cat.AddArrow(Arrow(a, b, "f0"));
         cat.AddArrow(Arrow(a, c, "f1"));

         cat.AddArrow(Arrow(b, d, "f2"));
         cat.AddArrow(Arrow(c, d, "f3"));

         cat.Inverse();

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(b, a, "f0")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(c, a, "f1")));

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(d, b, "f2")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(d, c, "f3")));
      }

      {
         Arrow arrow("A", "B");

         arrow.AddArrow(Arrow("a", "b"));
         arrow.AddArrow(Arrow("c", "d"));
         arrow.AddArrow(Arrow("e", "f", "ef_arrow"));

         arrow.Inverse();

         assert(arrow.QueryArrows(Arrow("b", "a").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow("d", "c").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow("*", "*", "ef_arrow").AsQuery()).size() == 1);
      }
   }
}

#endif // INVERSION_TEST_H
