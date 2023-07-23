#pragma once

#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "parser.h"

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

         auto src = R"(
SCAT cat
{
   OBJ a, b, c, d;

   a-[f0]->b;
   b-[f1]->c;
   c-[f2]->d;
   d-[f3]->c;
   c-[f4]->b;
   b-[f5]->a;
}
         )";

         Parser prs;
         prs.ParseSource(src);

         Node cat = *prs.Data();

         cat.SolveCompositions();

         cat.SolveCompositions();

         cat.SolveCompositions();

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("a", "c")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("a", "d")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("b", "d")));

         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("d", "b")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("d", "a")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow("c", "a")));

         auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
         cat.SolveCompositions();
         auto new_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();

         assert(new_count == prev_count);
      }

      {
         auto src = R"(
LCAT lcat
{
   SCAT scatA
   {
      OBJ xa, ya;
   }

   SCAT scatB
   {
      OBJ xb, yb;
   }

   SCAT scatC
   {
      OBJ xc, yc;
   }
}
         )";

         Parser prs;
         prs.ParseSource(src);

         Node lcat = *prs.Data();

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
