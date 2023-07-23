#pragma once

#include <assert.h>
#include <algorithm>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of morphism deletion methods
   //============================================================
   void test_morphism_deletion()
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
               ,  c("c", Node::EType::eObject);

         cat.AddNodes({a, b, c});

         cat.AddArrow(Arrow(a, b, "f0"));
         cat.AddArrow(Arrow(a, c, "f1"));

         // Deleting morphisms one at a time
         {
            auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
            assert(cat.EraseArrow("f0") == true);
            assert(!fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, b, "f0")));
            assert(prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size() - 1);

            assert(cat.EraseArrow("f1") == true);
            assert(!fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, c, "f1")));
            assert(prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
            assert(cat.EraseArrow(Arrow::IdArrowName(a.Name())) == false);
            auto new_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
            assert(prev_count == new_count);
            assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, a, Arrow::IdArrowName(a.Name()))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
            assert(cat.EraseArrow("Fake") == false);
            auto new_count = cat.QueryArrows(Arrow("*", "*").AsQuery()).size();
            assert(prev_count == new_count);
         }
      }

      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject);

         cat.AddNodes({a, b, c});

         cat.EmplaceArrow(a, b, "f0");
         cat.EmplaceArrow(a, c, "f1");

         // Erasing all arrows
         {
            cat.EraseArrows();

            assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 3);
         }
      }
   }
}
