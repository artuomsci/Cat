#ifndef EQUIVALENCE_TEST_H
#define EQUIVALENCE_TEST_H

#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   //============================================================
   // Testing of equivalence
   //============================================================
   void test_equivalence()
   {
      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         assert(arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B", "fff");
         Arrow arrow_right("A", "B", "aaa");

         assert(arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "C");

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "C");

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         Arrow arrow_0("a0", "b0");

         arrow_left.AddArrow(arrow_0);
         arrow_right.AddArrow(arrow_0);

         assert(arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         Arrow arrow_0("a0", "b0");
         Arrow arrow_1("a1", "b1");

         arrow_left.AddArrow(arrow_0);
         arrow_right.AddArrow(arrow_1);

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         Arrow arrow_0("a0", "b0");
         Arrow arrow_1("a1", "b1");

         arrow_left.AddArrow(arrow_0);
         arrow_left.AddArrow(arrow_1);

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         Arrow arrow_0("a0", "b0");
         Arrow arrow_1("a1", "b1");

         arrow_right.AddArrow(arrow_0);
         arrow_right.AddArrow(arrow_1);

         assert(!arrow_left.IsEquivalent(arrow_right));
      }
   }
}

#endif
