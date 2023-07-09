#ifndef COMPOSITION_TEST_H
#define COMPOSITION_TEST_H

#include <assert.h>
#include <algorithm>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of morphism compositions
   //============================================================
   void test_composition()
   {
      {
         auto fnCheckArrow = [](const Arrow::List& morphs_, const Arrow& morph_)
         {
            auto it = std::find_if(morphs_.begin(), morphs_.end(), [&](const Arrow::List::value_type& element_)
            {
               return element_.Name() == morph_.Name() && element_.Source() == morph_.Source() && element_.Target() == morph_.Target();
            });

            return it != morphs_.end();
         };

         Node cat("cat", Node::EType::eSCategory);

         cat.SolveCompositions();

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c, d});

         cat.SolveCompositions();

         cat.AddArrows({
                              Arrow(a, b, "f0")
                          ,   Arrow(b, c, "f1")
                          ,   Arrow(c, d, "f2")});
         cat.AddArrows({
                              Arrow(d, c, "f3")
                          ,   Arrow(c, b, "f4")
                          ,   Arrow(b, a, "f5")});

         cat.SolveCompositions();

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, c)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, d)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(b, d)));

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(d, b)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(d, a)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(c, a)));

         auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
         cat.SolveCompositions();
         auto new_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();

         assert(new_count == prev_count);
      }

      {
         Node lcat("lcat", Node::EType::eLCategory);

         Node scatA("scatA", Node::EType::eSCategory);
         Node scatB("scatB", Node::EType::eSCategory);
         Node scatC("scatC", Node::EType::eSCategory);

         scatA.EmplaceNode("xa", Node::EType::eObject);
         scatA.EmplaceNode("ya", Node::EType::eObject);
         lcat.AddNode(scatA);

         scatB.EmplaceNode("xb", Node::EType::eObject);
         scatB.EmplaceNode("yb", Node::EType::eObject);
         lcat.AddNode(scatB);

         scatC.EmplaceNode("xc", Node::EType::eObject);
         scatC.EmplaceNode("yc", Node::EType::eObject);
         lcat.AddNode(scatC);

         Arrow AB("scatA", "scatB");
         AB.EmplaceArrow("xa", "xb");
         AB.EmplaceArrow("ya", "yb");
         lcat.AddArrow(AB);

         Arrow BC("scatB", "scatC");
         BC.EmplaceArrow("xb", "xc");
         BC.EmplaceArrow("yb", "yc");
         lcat.AddArrow(BC);

         lcat.SolveCompositions();

         auto ret = lcat.QueryArrows(Arrow("*", "*").AsQuery());
         assert(ret.size() == 6);
         auto retAC = lcat.QueryArrows(Arrow("scatA", "scatC", "*").AsQuery());
         assert(!retAC.empty());
         assert(!retAC.front().QueryArrows(Arrow("xa", "xc", "*").AsQuery()).empty());
         assert(!retAC.front().QueryArrows(Arrow("ya", "yc", "*").AsQuery()).empty());
      }
   }
}

#endif // COMPOSITION_TEST_H
