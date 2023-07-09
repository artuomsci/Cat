#ifndef ARROW_APPLICATION_H
#define ARROW_APPLICATION_H

#include <assert.h>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of arrow application
   //============================================================
   void test_arrow_application()
   {
      {
         Arrow arrow("A", "B");

         Node A("A", Node::EType::eObject);

         auto ret = arrow(A);
         assert(ret.has_value());

         assert(ret->Name() == "B");
      }

      {
         Arrow arrow("A", "B");
         arrow.EmplaceArrow("a0", "b0");
         arrow.EmplaceArrow("a1", "b1");

         Node A("A", Node::EType::eSCategory);
         A.EmplaceNode("a0", A.InternalNode());
         A.EmplaceNode("a1", A.InternalNode());
         A.EmplaceArrow("a0", "a1");

         auto ret = arrow(A);
         assert(ret->Name() == "B");
         assert(ret->QueryNodes("*").size() == 2);

         assert(!ret->QueryNodes("b0").empty());
         assert(ret->QueryNodes("b0").front().Name() == "b0");

         assert(!ret->QueryNodes("b1").empty());
         assert(ret->QueryNodes("b1").front().Name() == "b1");

         auto arrows = ret->QueryArrows(Arrow("b0", "b1", "*").AsQuery());
         assert(!arrows.empty());
         assert(arrows.front().Source() == "b0");
         assert(arrows.front().Target() == "b1");
      }
   }
}

#endif // ARROW_APPLICATION_H
