#ifndef ARROW_GENERATOR_TEST
#define ARROW_GENERATOR_TEST

#include <assert.h>
#include <algorithm>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of mapping generator
   //============================================================
   void test_arrow_generator()
   {
      {
         // Source category
         Node A("A", Node::EType::eSCategory);
         A.EmplaceNode("a0", Node::EType::eObject);
         A.EmplaceNode("a1", Node::EType::eObject);

         // Target category
         Node B("B", Node::EType::eSCategory);
         B.EmplaceNode("b0", Node::EType::eObject);
         B.EmplaceNode("b1", Node::EType::eObject);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(A);
         ccat.AddNode(B);

         Arrow::List ret = ccat.ProposeArrows("A", "B");

         assert(ret.size() == 4);

         auto fnFindArrow = [&](Arrow arrow_, const std::string& source_, const std::string& target_)
         {
            auto morphisms = arrow_.QueryArrows(Arrow("*", "*").AsQuery());

            auto it = std::find_if(morphisms.begin(), morphisms.end(), [&](const Arrow& arrow)
            {
               return arrow.Source() == source_ && arrow.Target() == target_;
            });

            return it != ret.end();
         };

         auto head = ret.begin();

         assert(fnFindArrow(*head, "a0", "b0"));
         assert(fnFindArrow(*head, "a1", "b0"));

         head++;

         assert(fnFindArrow(*head, "a0", "b1"));
         assert(fnFindArrow(*head, "a1", "b0"));

         head++;

         assert(fnFindArrow(*head, "a0", "b0"));
         assert(fnFindArrow(*head, "a1", "b1"));

         head++;

         assert(fnFindArrow(*head, "a0", "b1"));
         assert(fnFindArrow(*head, "a1", "b1"));
      }

      {
         // Source category
         Node A("A", Node::EType::eSCategory);
         A.EmplaceNode("a", Node::EType::eObject);

         // Target category
         Node B("B", Node::EType::eSCategory);
         B.EmplaceNode("b0", Node::EType::eObject);
         B.EmplaceNode("b1", Node::EType::eObject);
         B.EmplaceNode("b2", Node::EType::eObject);

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(A);
         ccat.AddNode(B);

         Arrow::List ret = ccat.ProposeArrows("A", "B");

         assert(ret.size() == 3);

         ret = ccat.ProposeArrows("B", "A");

         assert(ret.size() == 1);
      }
   }
}

#endif // ARROW_GENERATOR_TEST
