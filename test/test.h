#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "../include/log.h"

#include "node_query_test.h"
#include "node_initial_terminal_test.h"
#include "composition_test.h"
#include "inversion_test.h"
#include "arrow_query_test.h"
#include "sequence_test.h"
#include "node_query_by_arrow_test.h"
#include "arrow_query_on_node.h"
#include "equivalence.h"
#include "arrow_validation.h"
#include "arrow_generator.h"

namespace cat
{
   void test()
   {
      ELogMode lmode = get_log_mode();

      set_log_mode(ELogMode::eQuiet);

      print_info("Start test");

      //============================================================
      // Testing of object addition methods
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject);

         Node A("A", Node::EType::eObject);

         Node PK(u8"Павка Корчагин", Node::EType::eObject);

         assert(cat.QueryNodes("*").size() == 0);

         assert(cat.AddNodes({a, b, A, PK}));

         assert(cat.QueryNodes("*").size() == 4);

         assert(cat.AddNode(a) == false);

         assert(cat.QueryNodes("*").size() == 4);
      }

      {
         Node cat("cat", Node::EType::eSCategory);

         Node a("a", Node::EType::eObject);

         assert(cat.AddNode(a));

         assert(cat.QueryNodes("*").size() == 1);

         assert(cat.AddNode(a) == false);
      }

      //============================================================
      // Testing of object deletion methods
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject);

         cat.AddNodes({a, b});

         cat.AddArrow(Arrow(a, b));

         assert(cat.EraseNode(a.Name()) == true);

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 1);

         assert(cat.EraseNode(b.Name()) == true);

         assert(cat.QueryArrows(Arrow("*", "*").AsQuery()).size() == 0);

         assert(cat.EraseNode(c.Name()) == false);

         assert(cat.QueryNodes("*").size() == 0);

         cat.AddNodes({a, b});

         cat.EraseNodes();

         assert(cat.QueryNodes("*").size() == 0);
         assert(cat.QueryArrows("* -> *").size() == 0);
      }

      auto fnCheckArrow = [](const Arrow::List& morphs_, const Arrow& morph_)
      {
         auto it = std::find_if(morphs_.begin(), morphs_.end(), [&](const Arrow::List::value_type& element_)
         {
            return element_.Name() == morph_.Name() && element_.Source() == morph_.Source() && element_.Target() == morph_.Target();
         });

         return it != morphs_.end();
      };

      //============================================================
      // Testing of morphism addition methods
      //============================================================
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

      //============================================================
      // Testing of morphism deletion methods
      //============================================================
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


      //============================================================
      // Testing of morphism deletion methods
      //============================================================
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

      //============================================================
      // Testing functor
      //============================================================
      {
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

      //============================================================
      // Testing of equivalence
      //============================================================
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

         {
            Arrow arrow_left("A", "B");
            Arrow arrow_right("A", "B");

            Arrow arrow_0("a0", "b0");

            arrow_0.EmplaceArrow("x", "y", "first");
            arrow_0.EmplaceArrow("x", "y", "second");

            arrow_left.AddArrow(arrow_0);
            arrow_right.AddArrow(arrow_0);

            assert(arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left("A", "B");
            Arrow arrow_right("A", "B");

            Arrow arrow_00("a0", "b0");
            arrow_00.EmplaceArrow("x", "y", "first");
            arrow_00.EmplaceArrow("x", "y", "second");

            Arrow arrow_01("a0", "b0");
            arrow_01.EmplaceArrow("x", "y", "first");
            arrow_01.EmplaceArrow("f", "g", "second");

            arrow_left.AddArrow(arrow_00);
            arrow_right.AddArrow(arrow_01);

            assert(!arrow_left.IsEquivalent(arrow_right));
         }
      }

      test_arrow_generator();

      test_composition();

      test_sequence();

      test_inversion();

      test_initial_terminal();

      test_arrow_query();

      test_arrow_query_on_node();

      test_node_query_by_arrow();

      test_node_query();

      test_equivalence();

      test_arrow_validation();

      print_info("End test");

      set_log_mode(lmode);
   }
}
