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

         assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 1);

         assert(cat.EraseNode(b.Name()) == true);

         assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 0);

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

         assert(!cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "a").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "b", "b").AsQuery()).empty());
         assert(!cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "c", "c").AsQuery()).empty());

         assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 3);

         assert(cat.AddArrow(Arrow(a, b, "f0")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, b, "f0")));

         assert(cat.AddArrow(Arrow(a, c, "f1")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, c, "f1")));

         assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 5);

         assert(cat.AddArrow(Arrow(a, d, "f2")) == false);
         assert(!fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, d, "f2")));
         assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 5);

         assert(cat.AddArrow(Arrow(b, c, "f0")) == false);
         assert(cat.AddArrow(Arrow(a, b, "f0")) == false);
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
            auto prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
            assert(cat.EraseArrow("f0") == true);
            assert(!fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, b, "f0")));
            assert(prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() - 1);

            assert(cat.EraseArrow("f1") == true);
            assert(!fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, c, "f1")));
            assert(prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
            assert(cat.EraseArrow(Arrow::IdArrowName(a.Name())) == false);
            auto new_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
            assert(prev_count == new_count);
            assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, a, Arrow::IdArrowName(a.Name()))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
            assert(cat.EraseArrow("Fake") == false);
            auto new_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
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

         cat.AddArrow(Arrow(a, b, "f0"));
         cat.AddArrow(Arrow(a, c, "f1"));

         // Erasing all arrows
         {
            cat.EraseArrows();

            assert(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size() == 3);
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
            C0.AddArrow(Arrow(a0, b0));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(a1, b1));

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
            C1.AddArrow(Arrow(b1, a1));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(C0, C1);
            functor.AddArrow(Arrow(a0, a1));
            functor.AddArrow(Arrow(b0, b1));

            assert(!ccat.AddArrow(functor));
         }

         // Checking identity arrow
         {
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

            C0.AddNodes({a0, b0});
            C0.AddArrow(Arrow(a0, b0));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);

            auto arrows = ccat.QueryArrows(Arrow(Arrow::EType::eFunctor, "*", "*").AsQuery());
            assert(arrows.size() == 1);
            auto arrows_btw_objects = arrows.front().QueryArrows(Arrow(Arrow::EType::eFunctor, "*", "*").AsQuery());
            assert(arrows_btw_objects.size() == 2);
         }

         // Missing arrow in functor
         {
            // Source category
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject), c0("c0", Node::EType::eObject);

            C0.AddNodes({a0, b0, c0});
            C0.AddArrow(Arrow(a0, b0));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(a1, b1));

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
            C0.AddArrow(Arrow(a0, b0));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(a1, b1));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(C0, C1);
            functor.AddArrow(Arrow(a0, a1, "f"));
            functor.AddArrow(Arrow(a0, b1, "f"));
            functor.AddArrow(Arrow(b0, b1));

            assert(!ccat.AddArrow(functor));
         }
      }

      //============================================================
      // Testing of equivalence
      //============================================================
      {
         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            assert(arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B", "fff");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B", "aaa");

            assert(arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "C");

            assert(!arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eMorphism, "A", "C");

            assert(!arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            Arrow arrow_0(Arrow::EType::eMorphism, "a0", "b0");

            arrow_left.AddArrow(arrow_0);
            arrow_right.AddArrow(arrow_0);

            assert(arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            Arrow arrow_0(Arrow::EType::eMorphism, "a0", "b0");
            Arrow arrow_1(Arrow::EType::eMorphism, "a1", "b1");

            arrow_left.AddArrow(arrow_0);
            arrow_right.AddArrow(arrow_1);

            assert(!arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            Arrow arrow_0(Arrow::EType::eMorphism, "a0", "b0");
            Arrow arrow_1(Arrow::EType::eMorphism, "a1", "b1");

            arrow_left.AddArrow(arrow_0);
            arrow_left.AddArrow(arrow_1);

            assert(!arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            Arrow arrow_0(Arrow::EType::eMorphism, "a0", "b0");
            Arrow arrow_1(Arrow::EType::eMorphism, "a1", "b1");

            arrow_right.AddArrow(arrow_0);
            arrow_right.AddArrow(arrow_1);

            assert(!arrow_left.IsEquivalent(arrow_right));
         }

         {
            Arrow arrow_left(Arrow::EType::eFunctor, "A", "B");
            Arrow arrow_right(Arrow::EType::eFunctor, "A", "B");

            Arrow arrow_0(Arrow::EType::eMorphism, "a0", "b0");

            Arrow arrow_1(Arrow::EType::eFunction, "x", "y", "first");
            Arrow arrow_2(Arrow::EType::eFunction, "x", "y", "second");

            arrow_0.AddArrow(arrow_1);
            arrow_0.AddArrow(arrow_2);

            arrow_left.AddArrow(arrow_0);
            arrow_right.AddArrow(arrow_0);

            assert(arrow_left.IsEquivalent(arrow_right));
         }
      }

      //============================================================
      // Testing of mapping generator
      //============================================================
      {
         // Source category
         Node A("A", Node::EType::eSCategory);
         Node a0("a0", Node::EType::eObject), a1("a1", Node::EType::eObject);

         A.AddNodes({a0, a1});

         // Target category
         Node B("B", Node::EType::eSCategory);
         Node b0("b0", Node::EType::eObject), b1("b1", Node::EType::eObject);

         B.AddNodes({b0, b1});

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(A);
         ccat.AddNode(B);

         Arrow::List ret = ccat.ProposeArrows("A", "B");

         assert(ret.size() == 4);

         auto fnFindArrow = [&](Arrow arrow_, const std::string& source_, const std::string& target_)
         {
            auto morphisms = arrow_.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery());

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

      test_composition();

      test_sequence();

      test_inversion();

      test_initial_terminal();

      test_arrow_query();

      test_arrow_query_on_node();

      test_node_query_by_arrow();

      test_node_query();

      test_equivalence();

      print_info("End test");

      set_log_mode(lmode);
   }
}
