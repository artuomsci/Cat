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

      //============================================================
      // Testing of object addition methods
      //============================================================
      {
         Node cat("cat", Node::EType::eSCategory);

         Node     a("a", Node::EType::eObject)
               ,  b("b", Node::EType::eObject);

         assert(cat.QueryNodes("*").size() == 0);

         assert(cat.AddNodes({a, b}));

         assert(cat.QueryNodes("*").size() == 2);

         assert(cat.AddNode(a) == false);

         assert(cat.QueryNodes("*").size() == 2);
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

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name()));

         assert(cat.EraseNode(a.Name()) == true);

         assert(cat.QueryArrows("* :: * -> *").size() == 1);

         assert(cat.EraseNode(b.Name()) == true);

         assert(cat.QueryArrows("* :: * -> *").size() == 0);

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

         assert(!cat.QueryArrows("* :: a -> a").empty());
         assert(!cat.QueryArrows("* :: b -> b").empty());
         assert(!cat.QueryArrows("* :: c -> c").empty());

         assert(cat.QueryArrows("* :: * -> *").size() == 3);

         assert(cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0")));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0")));

         assert(cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1")));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1")));

         assert(cat.QueryArrows("* :: * -> *").size() == 5);

         assert(cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), d.Name(), "f2")) == false);
         assert(!fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), d.Name(), "f2")));
         assert(cat.QueryArrows("* :: * -> *").size() == 5);

         assert(cat.AddArrow(Arrow(Arrow::EType::eMorphism, b.Name(), c.Name(), "f0")) == false);
         assert(cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0")) == false);
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

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0"));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1"));

         // Deleting morphisms one at a time
         {
            auto prev_count = cat.QueryArrows("* -> *").size();
            assert(cat.EraseArrow("f0") == true);
            assert(!fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0")));
            assert(prev_count = cat.QueryArrows("* :: * -> *").size() - 1);

            assert(cat.EraseArrow("f1") == true);
            assert(!fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1")));
            assert(prev_count = cat.QueryArrows("* :: * -> *").size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.QueryArrows("* :: * -> *").size();
            assert(cat.EraseArrow(Arrow::IdArrowName(a.Name())) == false);
            auto new_count = cat.QueryArrows("* :: * -> *").size();
            assert(prev_count == new_count);
            assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), a.Name(), Arrow::IdArrowName(a.Name()))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.QueryArrows("* :: * -> *").size();
            assert(cat.EraseArrow("Fake") == false);
            auto new_count = cat.QueryArrows("* :: * -> *").size();
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

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0"));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1"));

         // Erasing all arrows
         {
            cat.EraseArrows();

            assert(cat.QueryArrows("* :: * -> *").size() == 3);
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
                              Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0")
                          ,   Arrow(Arrow::EType::eMorphism, b.Name(), c.Name(), "f1")
                          ,   Arrow(Arrow::EType::eMorphism, c.Name(), d.Name(), "f2")});
         cat.AddArrows({
                              Arrow(Arrow::EType::eMorphism, d.Name(), c.Name(), "f3")
                          ,   Arrow(Arrow::EType::eMorphism, c.Name(), b.Name(), "f4")
                          ,   Arrow(Arrow::EType::eMorphism, b.Name(), a.Name(), "f5")});

         cat.SolveCompositions();

         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), c.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, a.Name(), d.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, b.Name(), d.Name())));

         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, d.Name(), b.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, d.Name(), a.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, c.Name(), a.Name())));

         auto prev_count = cat.QueryArrows("* :: * -> *").size();
         cat.SolveCompositions();
         auto new_count = cat.QueryArrows("* :: * -> *").size();

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
                     Arrow(Arrow::EType::eMorphism, a.Name(), b.Name())
                  ,  Arrow(Arrow::EType::eMorphism, b.Name(), a.Name())
                  ,  Arrow(Arrow::EType::eMorphism, a.Name(), c.Name())
                  ,  Arrow(Arrow::EType::eMorphism, b.Name(), d.Name())
                  ,  Arrow(Arrow::EType::eMorphism, c.Name(), d.Name())
                  ,  Arrow(Arrow::EType::eMorphism, c.Name(), f.Name())
                  ,  Arrow(Arrow::EType::eMorphism, d.Name(), e.Name())});

         Node::List seq = cat.SolveSequence(a, e);

         assert(seq.size() == 4);

         Arrow::List morphs = cat.MapNodes2Arrows(seq);
         auto it = morphs.begin();
         assert(*(it) == Arrow(Arrow::EType::eMorphism, a.Name(), b.Name()));
         assert(*(++it) == Arrow(Arrow::EType::eMorphism, b.Name(), d.Name()));
         assert(*(++it) == Arrow(Arrow::EType::eMorphism, d.Name(), e.Name()));

         seq = cat.SolveSequence(e, a);

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
                     Arrow(Arrow::EType::eMorphism, a.Name(), b.Name())
                  ,  Arrow(Arrow::EType::eMorphism, b.Name(), a.Name())
                  ,  Arrow(Arrow::EType::eMorphism, a.Name(), c.Name())
                  ,  Arrow(Arrow::EType::eMorphism, b.Name(), d.Name())
                  ,  Arrow(Arrow::EType::eMorphism, c.Name(), d.Name())
                  ,  Arrow(Arrow::EType::eMorphism, c.Name(), f.Name())
                  ,  Arrow(Arrow::EType::eMorphism, d.Name(), e.Name())
                  ,  Arrow(Arrow::EType::eMorphism, f.Name(), e.Name())});

         std::list<Node::List> seqs = cat.SolveSequences(a, e);

         assert(seqs.size() == 3);

         auto it = seqs.begin();

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*it == Arrow(Arrow::EType::eMorphism, a.Name(), b.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, b.Name(), d.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, d.Name(), e.Name()));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it) == Arrow(Arrow::EType::eMorphism, a.Name(), c.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, c.Name(), d.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, d.Name(), e.Name()));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it) == Arrow(Arrow::EType::eMorphism, a.Name(), c.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, c.Name(), f.Name()));
            assert(*(++it) == Arrow(Arrow::EType::eMorphism, f.Name(), e.Name()));
         }

         seqs = cat.SolveSequences(e, a);

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

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), b.Name(), "f0"));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a.Name(), c.Name(), "f1"));

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, b.Name(), d.Name(), "f2"));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, c.Name(), d.Name(), "f3"));

         cat.Inverse();

         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, b.Name(), a.Name(), "f0")));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, c.Name(), a.Name(), "f1")));

         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, d.Name(), b.Name(), "f2")));
         assert(fnCheckArrow(cat.QueryArrows("* :: * -> *"), Arrow(Arrow::EType::eMorphism, d.Name(), c.Name(), "f3")));
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

         cat.AddArrows({Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()), Arrow(Arrow::EType::eMorphism, a1.Name(), a0.Name())});

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b.Name()));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), c.Name()));

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), b.Name()));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), c.Name()));

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, b.Name(), d0.Name()));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, c.Name(), d0.Name()));

         cat.AddArrow(Arrow(Arrow::EType::eMorphism, b.Name(), d1.Name()));
         cat.AddArrow(Arrow(Arrow::EType::eMorphism, c.Name(), d1.Name()));

         cat.AddArrows({Arrow(Arrow::EType::eMorphism, d0.Name(), d1.Name()), Arrow(Arrow::EType::eMorphism, d1.Name(), d0.Name())});

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

         auto ret = arrow.QueryArrows("* :: a -> b");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows("* :: * -> b");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows("* :: a -> *");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "a", "b"));

         ret = arrow.QueryArrows("* :: * -> *");
         assert(ret.size() == 4);

         ret = arrow.QueryArrows("* :: * -> f");
         assert(ret.size() == 2);

         ret = arrow.QueryArrows("ef_arrow :: * -> *");
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

         ret = arrow.QueryArrows("ef_arrow :: * -> f");
         assert(ret.front() == Arrow(Arrow::EType::eMorphism, "e", "f", "ef_arrow"));

         ret = arrow.QueryArrows("* :: * -> f", 1);
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
         C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

         // Target category
         Node C1("C1", Node::EType::eSCategory);
         Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

         C1.AddNodes({a1, b1});
         C1.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), b1.Name()));

         // Category of categories
         Node ccat("Cat", Node::EType::eLCategory);
         ccat.AddNode(C0);
         ccat.AddNode(C1);

         Arrow functor(Arrow::EType::eFunctor, C0.Name(), C1.Name());
         functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()));
         functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

         assert(ccat.AddArrow(functor));

         auto ret = ccat.QueryArrows("* :: " + C0.Name() + " => " + C1.Name());
         assert(ret.front().Name() == Arrow(Arrow::EType::eFunctor, C0.Name(), C1.Name()).Name());
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

         assert(arrow.QueryArrows("* :: b -> a").size() == 1);
         assert(arrow.QueryArrows("* :: d -> c").size() == 1);
         assert(arrow.QueryArrows("ef_arrow :: f -> e").size() == 1);
      }

      //============================================================
      // Coproduct test
      //============================================================
      {
         //Obj a("a");
         //a.SetValue("a");
         //Obj b("b");
         //b.SetValue("b");
         //assert(coproduct(a, b)->Value() == Obj::TSet("ab"));

         //Obj f("f");
         //f.SetValue(4);
         //Obj s("s");
         //s.SetValue(5);
         //assert(coproduct(f, s)->Value() == Obj::TSet(9));

         //Obj fd("0.1");
         //fd.SetValue(0.1);
         //Obj sd("0.5");
         //sd.SetValue(0.5);
         //double result = std::get<double>(coproduct(fd, sd)->Value());
         //assert(std::abs(result - 0.6) < std::numeric_limits<double>::epsilon());
      }

//      //============================================================
//      // Product test
//      //============================================================
//      {
//         Obj a("ac");
//         a.SetValue("ac");
//         Obj b("bd");
//         b.SetValue("bd");
//         assert(product(a, b)->Value() == Obj::TSet("abadcbcd"));
//
//         Obj f("4");
//         f.SetValue(4);
//         Obj s("5");
//         s.SetValue(5);
//         assert(product(f, s)->Value() == Obj::TSet(20));
//
//         Obj fd("1.1");
//         fd.SetValue(1.1);
//         Obj sd("5");
//         sd.SetValue(5.0);
//         double result = std::get<double>(product(fd, sd)->Value());
//         assert(std::abs(result - 5.5) < std::numeric_limits<double>::epsilon());
//      }
//
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
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), b1.Name()));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(Arrow::EType::eFunctor, C0.Name(), C1.Name());
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

            assert(ccat.AddArrow(functor));
         }

         // Missing arrow in target category
         {
            // Source category
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

            C0.AddNodes({a0, b0});
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(Arrow::EType::eFunctor,  C0.Name(), C1.Name());
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

            assert(!ccat.AddArrow(functor));
         }

         // Incorrect arrow direction in target category
         {
            // Source category
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

            C0.AddNodes({a0, b0});
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(Arrow::EType::eMorphism, b1.Name(), a1.Name()));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(Arrow::EType::eFunctor, C0.Name(), C1.Name());
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

            assert(!ccat.AddArrow(functor));
         }

         // Checking identity arrow
         {
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

            C0.AddNodes({a0, b0});
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);

            auto arrows = ccat.QueryArrows("* :: * => *");
            assert(arrows.size() == 1);
            auto arrows_btw_objects = arrows.front().QueryArrows("* :: * -> *");
            assert(arrows_btw_objects.size() == 2);
         }

         // Missing arrow in functor
         {
            // Source category
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject), c0("c0", Node::EType::eObject);

            C0.AddNodes({a0, b0, c0});
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), b1.Name()));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(Arrow::EType::eFunctor, C0.Name(), C1.Name());
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name()));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

            assert(!ccat.AddArrow(functor));
         }

         // Mapping the same node multiple times
         {
            // Source category
            Node C0("C0", Node::EType::eSCategory);
            Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

            C0.AddNodes({a0, b0});
            C0.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b0.Name()));

            // Target category
            Node C1("C1", Node::EType::eSCategory);
            Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

            C1.AddNodes({a1, b1});
            C1.AddArrow(Arrow(Arrow::EType::eMorphism, a1.Name(), b1.Name()));

            // Category of categories
            Node ccat("Cat", Node::EType::eLCategory);
            ccat.AddNode(C0);
            ccat.AddNode(C1);

            Arrow functor(Arrow::EType::eFunctor, C0.Name(), C1.Name());
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), a1.Name(), "f"));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, a0.Name(), b1.Name(), "f"));
            functor.AddArrow(Arrow(Arrow::EType::eMorphism, b0.Name(), b1.Name()));

            assert(!ccat.AddArrow(functor));
         }
      }

      print_info("End test");

      set_log_mode(lmode);
   }
}
