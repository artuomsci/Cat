#include <assert.h>
#include <algorithm>

#include "../include/common.h"
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
         Node cat("cat");

         Node a("a"), b("b");

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
         Node cat("cat");

         Node a("a"), b("b"), c("c");

         cat.AddNodes({a, b});

         cat.AddArrow(Arrow(a.Name(), b.Name()));

         assert(cat.EraseNode(a.Name()) == true);

         assert(cat.QueryArrows("* -> *").size() == 1);

         assert(cat.EraseNode(b.Name()) == true);

         assert(cat.QueryArrows("* -> *").size() == 0);

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
         Node cat("cat");

         Node a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c});

         assert(!cat.QueryArrows("a -> a").empty());
         assert(!cat.QueryArrows("b -> b").empty());
         assert(!cat.QueryArrows("c -> c").empty());

         assert(cat.QueryArrows("* -> *").size() == 3);

         assert(cat.AddArrow(Arrow(a.Name(), b.Name(), "f0")));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), b.Name(), "f0")));

         assert(cat.AddArrow(Arrow(a.Name(), c.Name(), "f1")));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), c.Name(), "f1")));

         assert(cat.QueryArrows("* -> *").size() == 5);

         assert(cat.AddArrow(Arrow(a.Name(), d.Name(), "f2")) == false);
         assert(!fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), d.Name(), "f2")));
         assert(cat.QueryArrows("* -> *").size() == 5);

         assert(cat.AddArrow(Arrow(b.Name(), c.Name(), "f0")) == false);
         assert(cat.AddArrow(Arrow(a.Name(), b.Name(), "f0")) == false);
      }

      //============================================================
      // Testing of morphism deletion methods
      //============================================================
      {
         Node cat("cat");

         Node a("a"), b("b"), c("c");

         cat.AddNodes({a, b, c});

         cat.AddArrow(Arrow(a.Name(), b.Name(), "f0"));
         cat.AddArrow(Arrow(a.Name(), c.Name(), "f1"));

         // Deleting morphisms one at a time
         {
            auto prev_count = cat.QueryArrows("* -> *").size();
            assert(cat.EraseArrow("f0") == true);
            assert(!fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), b.Name(), "f0")));
            assert(prev_count = cat.QueryArrows("* -> *").size() - 1);

            assert(cat.EraseArrow("f1") == true);
            assert(!fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), c.Name(), "f1")));
            assert(prev_count = cat.QueryArrows("* -> *").size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.QueryArrows("* -> *").size();
            assert(cat.EraseArrow(id_arrow_name(a.Name())) == false);
            auto new_count = cat.QueryArrows("* -> *").size();
            assert(prev_count == new_count);
            assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), a.Name(), id_arrow_name(a.Name()))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.QueryArrows("* -> *").size();
            assert(cat.EraseArrow("Fake") == false);
            auto new_count = cat.QueryArrows("* -> *").size();
            assert(prev_count == new_count);
         }
      }

      //============================================================
      // Testing of morphism compositions
      //============================================================
      {
         Node cat("cat");

         cat.SolveCompositions();

         Node a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c, d});

         cat.SolveCompositions();

         cat.AddArrows({Arrow(a.Name(), b.Name(), "f0"), Arrow(b.Name(), c.Name(), "f1"), Arrow(c.Name(), d.Name(), "f2")});
         cat.AddArrows({Arrow(d.Name(), c.Name(), "f3"), Arrow(c.Name(), b.Name(), "f4"), Arrow(b.Name(), a.Name(), "f5")});

         cat.SolveCompositions();

         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), c.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(a.Name(), d.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(b.Name(), d.Name())));

         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(d.Name(), b.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(d.Name(), a.Name())));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(c.Name(), a.Name())));

         auto prev_count = cat.QueryArrows("* -> *").size();
         cat.SolveCompositions();
         auto new_count = cat.QueryArrows("* -> *").size();

         assert(new_count == prev_count);
      }

      //============================================================
      // Testing morphism sequence
      //============================================================
      {
         Node cat("cat");

         Node a("a"), b("b"), c("c"), d("d"), e("e"), f("f");

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Arrow(a.Name(), b.Name())
                  ,  Arrow(b.Name(), a.Name())
                  ,  Arrow(a.Name(), c.Name())
                  ,  Arrow(b.Name(), d.Name())
                  ,  Arrow(c.Name(), d.Name())
                  ,  Arrow(c.Name(), f.Name())
                  ,  Arrow(d.Name(), e.Name())});

         Node::List seq = cat.SolveSequence(a, e);

         assert(seq.size() == 4);

         Arrow::List morphs = cat.MapNodes2Arrows(seq);
         auto it = morphs.begin();
         assert(*(it) == Arrow(a.Name(), b.Name()));
         assert(*(++it) == Arrow(b.Name(), d.Name()));
         assert(*(++it) == Arrow(d.Name(), e.Name()));

         seq = cat.SolveSequence(e, a);

         assert(seq.size() == 0);
      }

      //============================================================
      // Testing morphism sequences
      //============================================================
      {
         Node cat("cat");

         Node a("a"), b("b"), c("c"), d("d"), e("e"), f("f");

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Arrow(a.Name(), b.Name())
                  ,  Arrow(b.Name(), a.Name())
                  ,  Arrow(a.Name(), c.Name())
                  ,  Arrow(b.Name(), d.Name())
                  ,  Arrow(c.Name(), d.Name())
                  ,  Arrow(c.Name(), f.Name())
                  ,  Arrow(d.Name(), e.Name())
                  ,  Arrow(f.Name(), e.Name())});

         std::list<Node::List> seqs = cat.SolveSequences(a, e);

         assert(seqs.size() == 3);

         auto it = seqs.begin();

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*it == Arrow(a.Name(), b.Name()));
            assert(*(++it) == Arrow(b.Name(), d.Name()));
            assert(*(++it) == Arrow(d.Name(), e.Name()));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it) == Arrow(a.Name(), c.Name()));
            assert(*(++it) == Arrow(c.Name(), d.Name()));
            assert(*(++it) == Arrow(d.Name(), e.Name()));
         }

         it = std::next(it);

         {
            Arrow::List morphs = cat.MapNodes2Arrows(*it);
            auto it = morphs.begin();
            assert(*(it) == Arrow(a.Name(), c.Name()));
            assert(*(++it) == Arrow(c.Name(), f.Name()));
            assert(*(++it) == Arrow(f.Name(), e.Name()));
         }

         seqs = cat.SolveSequences(e, a);

         assert(seqs.size() == 0);
      }

      //============================================================
      // Testing of inversion
      //============================================================
      {
         Node cat("cat");

         Node a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c, d});

         cat.AddArrow(Arrow(a.Name(), b.Name(), "f0"));
         cat.AddArrow(Arrow(a.Name(), c.Name(), "f1"));

         cat.AddArrow(Arrow(b.Name(), d.Name(), "f2"));
         cat.AddArrow(Arrow(c.Name(), d.Name(), "f3"));

         cat.Inverse();

         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(b.Name(), a.Name(), "f0")));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(c.Name(), a.Name(), "f1")));

         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(d.Name(), b.Name(), "f2")));
         assert(fnCheckArrow(cat.QueryArrows("* -> *"), Arrow(d.Name(), c.Name(), "f3")));
      }

      //============================================================
      // Testing of initial/terminal objects
      //============================================================
      {
         Node cat("cat");

         Node a0("a0"), a1("a1"), b("b"), c("c"), d0("d0"), d1("d1");

         cat.AddNodes({a0, a1, b, c, d0, d1});

         cat.AddArrows({Arrow(a0.Name(), a1.Name()), Arrow(a1.Name(), a0.Name())});

         cat.AddArrow(Arrow(a0.Name(), b.Name()));
         cat.AddArrow(Arrow(a0.Name(), c.Name()));

         cat.AddArrow(Arrow(a1.Name(), b.Name()));
         cat.AddArrow(Arrow(a1.Name(), c.Name()));

         cat.AddArrow(Arrow(b.Name(), d0.Name()));
         cat.AddArrow(Arrow(c.Name(), d0.Name()));

         cat.AddArrow(Arrow(b.Name(), d1.Name()));
         cat.AddArrow(Arrow(c.Name(), d1.Name()));

         cat.AddArrows({Arrow(d0.Name(), d1.Name()), Arrow(d1.Name(), d0.Name())});

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
      // Testing of arrow query
      //============================================================
      {
         Arrow arrow("A", "B");

         arrow.AddArrow(Arrow("a", "b"));
         arrow.AddArrow(Arrow("c", "d"));
         arrow.AddArrow(Arrow("e", "f", "ef_arrow"));
         arrow.AddArrow(Arrow("g", "f"));

         auto ret = arrow.QueryArrows("a -> b");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow("a", "b"));

         ret = arrow.QueryArrows("* -> b");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow("a", "b"));

         ret = arrow.QueryArrows("a -> *");
         assert(ret.size() == 1);
         assert(ret.front() == Arrow("a", "b"));

         ret = arrow.QueryArrows("* -> *");
         assert(ret.size() == 4);

         ret = arrow.QueryArrows("* -> f");
         assert(ret.size() == 2);

         ret = arrow.QueryArrows("* -[ ef_arrow ]-> *");
         assert(ret.front() == Arrow("e", "f", "ef_arrow"));

         ret = arrow.QueryArrows("* -[ ef_arrow ]-> f");
         assert(ret.front() == Arrow("e", "f", "ef_arrow"));
      }

      //============================================================
      // Testing of arrow inversion
      //============================================================
      {
         Arrow arrow("A", "B");

         arrow.AddArrow(Arrow("a", "b"));
         arrow.AddArrow(Arrow("c", "d"));
         arrow.AddArrow(Arrow("e", "f", "ef_arrow"));

         arrow.inverse();

         assert(arrow.QueryArrows("b -> a").size() == 1);
         assert(arrow.QueryArrows("d -> c").size() == 1);
         assert(arrow.QueryArrows("f -[ ef_arrow ]-> e").size() == 1);
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
//      //============================================================
//      // Testing functor
//      //============================================================
//      {
////         Cat C0("C0");
////         Obj a0("a0"), b0("b0");
//
////         C0.AddNodes(a0, b0);
//
////         Cat C1("C1");
////         Obj a1("a1"), b1("b1");
//         
////         C1.AddNodes(a1, b1);
//
////         CACat ccat;
////         ccat.AddArrow(C0);
////         ccat.AddArrow(C1);
//
////         Func fn(C0.GetName(), C1.GetName());
//      }

      print_info("End test");

      set_log_mode(lmode);
   }
}
