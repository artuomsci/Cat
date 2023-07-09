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

         arrow_left.EmplaceArrow("a0", "b0");
         arrow_right.EmplaceArrow("a0", "b0");

         assert(arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         arrow_left.EmplaceArrow("a0", "b0");
         arrow_right.EmplaceArrow("a1", "b1");

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         arrow_left.EmplaceArrow("a0", "b0");
         arrow_left.EmplaceArrow("a1", "b1");

         assert(!arrow_left.IsEquivalent(arrow_right));
      }

      {
         Arrow arrow_left("A", "B");
         Arrow arrow_right("A", "B");

         arrow_right.EmplaceArrow("a0", "b0");
         arrow_right.EmplaceArrow("a1", "b1");

         assert(!arrow_left.IsEquivalent(arrow_right));
      }
   }
}

#endif
