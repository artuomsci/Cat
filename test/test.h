#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   void test()
   {
      ELogMode lmode = get_log_mode();

      set_log_mode(ELogMode::eQuiet);

      print_info("Start test");

      auto match_nodes = [](const Node::List& list_, std::vector<Node::NName> names_)
      {
         if (names_.size() != list_.size())
            return false;

         for (const auto& name : names_)
         {
            int counter {};
            for (const auto& node : list_)
            {
               if (node.Name() == name)
                  counter++;
            }

            if (counter != 1)
               return false;

            counter = 0;
         }

         return true;
      };

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
      // Testing of morphism compositions
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         cat.SolveCompositions();

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c, d});

         cat.SolveCompositions();

         cat.AddArrows({
                              Arrow(a, b, "f0")
                          ,   Arrow(b, c, "f1")
                          ,   Arrow(c, d, "f2")});
         cat.AddArrows({
                              Arrow(d, c, "f3")
                          ,   Arrow(c, b, "f4")
                          ,   Arrow(b, a, "f5")});

         cat.SolveCompositions();

         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, c)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(a, d)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(b, d)));

         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(d, b)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(d, a)));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(c, a)));

         auto prev_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();
         cat.SolveCompositions();
         auto new_count = cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()).size();

         assert(new_count == prev_count);
      }

      //============================================================
      // Testing morphism sequence
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject)
               ,  e("e", Node::EType::eObject)
               ,  f("f", Node::EType::eObject);

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Arrow(a, b)
                  ,  Arrow(b, a)
                  ,  Arrow(a, c)
                  ,  Arrow(b, d)
                  ,  Arrow(c, d)
                  ,  Arrow(c, f)
                  ,  Arrow(d, e)});

         std::list<Node::NName> seq = cat.SolveSequence(a.Name(), e.Name());

         assert(seq.size() == 4);

         Arrow::List morphs = cat.MapNodes2Arrows(seq);
         auto it = morphs.begin();
         assert(*(it)   == Arrow(a, b));
         assert(*(++it) == Arrow(b, d));
         assert(*(++it) == Arrow(d, e));

         seq = cat.SolveSequence(e.Name(), a.Name());

         assert(seq.size() == 0);
      }

      //============================================================
      // Testing morphism sequences
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject)
               ,  e("e", Node::EType::eObject)
               ,  f("f", Node::EType::eObject);

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Arrow(a, b)
                  ,  Arrow(b, a)
                  ,  Arrow(a, c)
                  ,  Arrow(b, d)
                  ,  Arrow(c, d)
                  ,  Arrow(c, f)
                  ,  Arrow(d, e)
                  ,  Arrow(f, e)});

         std::list<std::list<Node::NName>> seqs = cat.SolveSequences(a.Name(), e.Name());

         assert(seqs.size() == 3);

         auto it = seqs.begin();

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*it     == Arrow(a, b));
            assert(*(++it) == Arrow(b, d));
            assert(*(++it) == Arrow(d, e));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it)   == Arrow(a, c));
            assert(*(++it) == Arrow(c, d));
            assert(*(++it) == Arrow(d, e));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it)   == Arrow(a, c));
            assert(*(++it) == Arrow(c, f));
            assert(*(++it) == Arrow(f, e));
         }

         seqs = cat.SolveSequences(e.Name(), a.Name());

         assert(seqs.size() == 0);
      }

      //============================================================
      // Testing of inversion
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d("d", Node::EType::eObject);

         cat.AddNodes({a, b, c, d});

         cat.AddArrow(Arrow(a, b, "f0"));
         cat.AddArrow(Arrow(a, c, "f1"));

         cat.AddArrow(Arrow(b, d, "f2"));
         cat.AddArrow(Arrow(c, d, "f3"));

         cat.Inverse();

         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(b, a, "f0")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(c, a, "f1")));

         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(d, b, "f2")));
         assert(fnCheckArrow(cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()), Arrow(d, c, "f3")));
      }

      //============================================================
      // Testing of initial/terminal objects
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a0("a0", Node::EType::eObject)
               ,  a1("a1", Node::EType::eObject)
               ,  b("b", Node::EType::eObject)
               ,  c("c", Node::EType::eObject)
               ,  d0("d0", Node::EType::eObject)
               ,  d1("d1", Node::EType::eObject);

         cat.AddNodes({a0, a1, b, c, d0, d1});

         cat.AddArrows({Arrow(a0, a1), Arrow(a1, a0)});

         cat.AddArrow(Arrow(a0, b));
         cat.AddArrow(Arrow(a0, c));

         cat.AddArrow(Arrow(a1, b));
         cat.AddArrow(Arrow(a1, c));

         cat.AddArrow(Arrow(b, d0));
         cat.AddArrow(Arrow(c, d0));

         cat.AddArrow(Arrow(b, d1));
         cat.AddArrow(Arrow(c, d1));

         cat.AddArrows({Arrow(d0, d1), Arrow(d1, d0)});

         cat.SolveCompositions();

         Node::List initial_obj = cat.Initial();
         assert(initial_obj.size() == 2);
         initial_obj.sort();
         auto it_initial = initial_obj.begin();
         assert(*it_initial == a0);
         assert(*(++it_initial) == a1);

         Node::List terminal_obj = cat.Terminal();
         assert(terminal_obj.size() == 2);
         terminal_obj.sort();
         auto it_terminal = terminal_obj.begin();
         assert(*(it_terminal) == d0);
         assert(*(++it_terminal) == d1);

         cat.EraseNodes();

         cat.AddNode(a0);
         initial_obj = cat.Initial();
         terminal_obj = cat.Terminal();
         assert(initial_obj.size() == terminal_obj.size() == 1);
         assert(*initial_obj.begin() == *terminal_obj.begin());
      }

      //============================================================
      // Testing of arrow query (morphisms)
      //============================================================
      {
         Arrow arrow(Arrow::EType::eFunctor, "A", "B");

         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "a", "b"));
         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "c", "d"));
         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));
         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "g", "f"));

         auto ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "b").AsQuery());
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "b").AsQuery());
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "*").AsQuery());
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery());
         assert(ret.size() == 4);

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f").AsQuery());
         assert(ret.size() == 2);

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*", "ef_arrow").AsQuery());
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f", "ef_arrow").AsQuery());
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

         ret = arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "f").AsQuery(), 1);
         assert(ret.size() == 1);
      }

      //============================================================
      // Testing of arrow query (functors)
      //============================================================
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

         auto ret = ccat.QueryArrows(Arrow(Arrow::EType::eFunctor, C0.Name(), C1.Name()).AsQuery());
         assert(ret.front().Name() == Arrow(C0, C1).Name());
      }

      //============================================================
      // Testing of arrow query (patterns)
      //============================================================
      {
         Node S("S", Node::EType::eSCategory);
         Node a("a", Node::EType::eObject), b("b", Node::EType::eObject);
         Node c("c", Node::EType::eObject), d("d", Node::EType::eObject);

         S.AddNodes({a, b, c, d});
         S.AddArrow(Arrow(a, b));
         S.AddArrow(Arrow(a, c));
         S.AddArrow(Arrow(c, d));

         Node ret = S.Query(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery());
         assert(ret.QueryNodes("*").size() == 4);

         ret = S.Query(Arrow(Arrow::EType::eMorphism, "a", "*").AsQuery());
         assert(ret.QueryNodes("*").size() == 3);
         assert(ret.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "b").AsQuery()).size() == 1);
         assert(ret.QueryArrows(Arrow(Arrow::EType::eMorphism, "a", "c").AsQuery()).size() == 1);

         ret = S.Query(Arrow(Arrow::EType::eMorphism, "c", "d").AsQuery());
         assert(ret.QueryNodes("*").size() == 2);
         assert(ret.QueryArrows(Arrow(Arrow::EType::eMorphism, "c", "d").AsQuery()).size() == 1);
      }

      //============================================================
      // Testing of node query
      //============================================================
      {
         Node S("S", Node::EType::eSCategory);

         Node a("a", Node::EType::eObject), b("b", Node::EType::eObject);
         Node c("c", Node::EType::eObject), d("d", Node::EType::eObject);

         S.AddNodes({a, b, c, d});

         assert(S.QueryNodes("()").size() == 0);
         assert(match_nodes(S.QueryNodes("*"), {"a", "b", "c", "d"}));
         assert(match_nodes(S.QueryNodes("a"), {"a"}));
         assert(match_nodes(S.QueryNodes("a | a"), {"a"}));
         assert(match_nodes(S.QueryNodes("e | f | a | m | g"), {"a"}));
         assert(match_nodes(S.QueryNodes("e & f | a | m & g"), {"a"}));
         assert(match_nodes(S.QueryNodes("a | b | m"), {"a", "b"}));
         assert(match_nodes(S.QueryNodes("a & b & c"), {"a", "b", "c"}));
         assert(S.QueryNodes("a & h").size() == 0);
         assert(match_nodes(S.QueryNodes("a & b | h"), {"a", "b"}));
         assert(match_nodes(S.QueryNodes("a & (b | h)"), {"a", "b"}));
         assert(match_nodes(S.QueryNodes("a & b | c & d"), {"a", "b", "c", "d"}));
         assert(match_nodes(S.QueryNodes("(a & b) | (c & d)"), {"a", "b", "c", "d"}));
         assert(match_nodes(S.QueryNodes("~a"), {"b", "c", "d"}));
         assert(match_nodes(S.QueryNodes("~f"), {"a", "b", "c", "d"}));
         assert(match_nodes(S.QueryNodes("~(a & b)"), {"c", "d"}));
      }

      //============================================================
      // Testing of arrow inversion
      //============================================================
      {
         Arrow arrow(Arrow::EType::eFunctor, "A", "B");

         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "a", "b"));
         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "c", "d"));
         arrow.AddArrow(Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

         arrow.Inverse();

         assert(arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "b", "a").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "d", "c").AsQuery()).size() == 1);
         assert(arrow.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*", "ef_arrow").AsQuery()).size() == 1);
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

      print_info("End test");

      set_log_mode(lmode);
   }
}
