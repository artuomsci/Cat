#ifndef ARROW_VALIDATION_TEST_H
#define ARROW_VALIDATION_TEST_H

#include <assert.h>

#include "../include/node.h"

namespace cat
{
   //============================================================
   // Testing of arrow validation
   //============================================================
   void test_arrow_validation()
   {
      {
         Arrow arrow("A", "B");
         assert(arrow.IsValid());
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("c", "d");

         assert(arrow.IsValid());
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("a", "d");
         arrow.EmplaceArrow("a", "c");

         assert(!arrow.IsValid());
      }

      {
         Arrow arrow("A", "B");

         arrow.EmplaceArrow("a", "b");
         arrow.EmplaceArrow("c", "b");

         assert(arrow.IsValid());
      }
   }
}

#endif // ARROW_VALIDATION_TEST_H
