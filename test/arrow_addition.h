#pragma once

#include <assert.h>
#include <algorithm>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of morphism addition methods
   //============================================================
   void test_morphism_addition()
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

         cat.AddNodes({a, b, c});

         assert(!cat.QueryArrows(Arrow("a", "a").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow("b", "b").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow("c", "c").AsQuery()).empty());

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 3);

         assert(cat.AddArrow(Arrow(a, b, "f0")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, b, "f0")));

         assert(cat.AddArrow(Arrow(a, c, "f1")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, c, "f1")));

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 5);

         assert(cat.AddArrow(Arrow(a, d, "f2")) == false);
         assert(!fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, d, "f2")));
         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 5);

         assert(cat.AddArrow(Arrow(b, c, "f0")) == false);
         assert(cat.AddArrow(Arrow(a, b, "f0")) == false);
      }

      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c});

         assert(!cat.QueryArrows(Arrow("a", "a").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow("b", "b").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow("c", "c").AsQuery()).empty());

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 3);

         assert(cat.EmplaceArrow(a, b, "f0"));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, b, "f0")));

         assert(cat.EmplaceArrow(a, c, "f1"));
         assert(fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, c, "f1")));

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 5);

         assert(cat.EmplaceArrow(a, d, "f2") == false);
         assert(!fnCheckArrow(cat.QueryArrows(Arrow("*", "*").AsQuery()), Arrow(a, d, "f2")));
         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 5);

         assert(cat.EmplaceArrow(b, c, "f0") == false);
         assert(cat.EmplaceArrow(a, b, "f0") == false);
      }

      // Correct functor
      {
         // Source category
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

         C0.AddNodes({a0, b0});
         C0.EmplaceArrow(a0, b0);

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});
         C1.EmplaceArrow(a1, b1);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(C0, C1);
         functor.AddArrow(Arrow(a0, a1));
         functor.AddArrow(Arrow(b0, b1));

         assert(ccat.AddArrow(functor));
      }

      // Missing arrow in target category
      {
         // Source category
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

         C0.AddNodes({a0, b0});
         C0.AddArrow(Arrow(a0, b0));

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(C0, C1);
         functor.AddArrow(Arrow(a0, a1));
         functor.AddArrow(Arrow(b0, b1));

         assert(!ccat.AddArrow(functor));
      }

      // Incorrect arrow direction in target category
      {
         // Source category
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

         C0.AddNodes({a0, b0});
         C0.AddArrow(Arrow(a0, b0));

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});
         C1.EmplaceArrow(b1, a1);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(C0, C1);
         functor.EmplaceArrow(a0, a1);
         functor.EmplaceArrow(b0, b1);

         assert(!ccat.AddArrow(functor));
      }

      // Checking identity arrow
      {
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

         C0.AddNodes({a0, b0});
         C0.EmplaceArrow(a0, b0);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);

         auto arrows = ccat.QueryArrows(Arrow("*", "*").AsQuery());
         assert(arrows.size() == 1);
         auto arrows_btw_objects = arrows.front().QueryArrows(Arrow("*", "*").AsQuery());
         assert(arrows_btw_objects.size() == 2);
      }

      // Missing arrow in functor
      {
         // Source category
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject), c0("c0", Node::EType::eObject);

         C0.AddNodes({a0, b0, c0});
         C0.EmplaceArrow(a0, b0);

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});
         C1.EmplaceArrow(a1, b1);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(C0, C1);
         functor.AddArrow(Arrow(a0, a1));
         functor.AddArrow(Arrow(b0, b1));

         assert(!ccat.AddArrow(functor));
      }

      // Mapping the same node multiple times
      {
         // Source category
         Node C0("C0", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

         C0.AddNodes({a0, b0});
         C0.EmplaceArrow(a0, b0);

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});
         C1.EmplaceArrow(a1, b1);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(C0, C1);
         functor.EmplaceArrow(a0, a1, "f");
         functor.EmplaceArrow(a0, b1, "f");
         functor.EmplaceArrow(b0, b1);

         assert(!ccat.AddArrow(functor));
      }
   }
}
