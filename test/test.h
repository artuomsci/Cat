#include <assert.h>
#include <algorithm>

#include "../include/cat.h"
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
         Cat cat("cat");

         Obj a("a"), b("b");

         assert(cat.Nodes().size() == 0);

         assert(cat.AddNodes({a, b}));

         assert(cat.Nodes().size() == 2);

         assert(cat.AddNode(a) == false);

         assert(cat.Nodes().size() == 2);
      }

      //============================================================
      // Testing of object deletion methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c");

         cat.AddNodes({a, b});

         cat.AddArrow(Morph(a, b));

         assert(cat.EraseNode(a) == true);

         assert(cat.Arrows().size() == 1);

         assert(cat.EraseNode(b) == true);

         assert(cat.Arrows().size() == 0);

         assert(cat.EraseNode(c) == false);

         assert(cat.Nodes().size() == 0);

         cat.AddNodes({a, b});

         cat.EraseNodes();

         assert(cat.Nodes().size() == 0);
         assert(cat.Arrows().size() == 0);
      }

      auto fnCheckMorph = [](const MorphVec& morphs_, const Morph& morph_)
      {
         auto it = std::find_if(morphs_.begin(), morphs_.end(), [&](const MorphVec::value_type& element_)
         {
            return element_.name == morph_.name && element_.source == morph_.source && element_.target == morph_.target;
         });

         return it != morphs_.end();
      };

      //============================================================
      // Testing of morphism addition methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c});

         assert(cat.MatchArrow(a, a));
         assert(cat.MatchArrow(b, b));
         assert(cat.MatchArrow(c, c));

         assert(cat.Arrows().size() == 3);

         assert(cat.AddArrow(Morph(a, b, "f0")));
         assert(fnCheckMorph(cat.Arrows(), Morph(a, b, "f0")));

         assert(cat.AddArrow(Morph(a, c, "f1")));
         assert(fnCheckMorph(cat.Arrows(), Morph(a, c, "f1")));

         assert(cat.Arrows().size() == 5);

         assert(cat.AddArrow(Morph(a, d, "f2")) == false);
         assert(!fnCheckMorph(cat.Arrows(), Morph(a, d, "f2")));
         assert(cat.Arrows().size() == 5);

         assert(cat.AddArrow(Morph(b, c, "f0")) == false);
         assert(cat.AddArrow(Morph(a, b, "f0")));
      }

      //============================================================
      // Testing of morphism deletion methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c");

         cat.AddNodes({a, b, c});

         cat.AddArrow(Morph(a, b, "f0"));
         cat.AddArrow(Morph(a, c, "f1"));

         // Deleting morphisms one at a time
         {
            auto prev_count = cat.Arrows().size();
            assert(cat.EraseArrow("f0") == true);
            assert(!fnCheckMorph(cat.Arrows(), Morph(a, b, "f0")));
            assert(prev_count = cat.Arrows().size() - 1);

            assert(cat.EraseArrow("f1") == true);
            assert(!fnCheckMorph(cat.Arrows(), Morph(a, c, "f1")));
            assert(prev_count = cat.Arrows().size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.Arrows().size();
            assert(cat.EraseArrow(id_arrow_name(a.GetName())) == false);
            auto new_count = cat.Arrows().size();
            assert(prev_count == new_count);
            assert(fnCheckMorph(cat.Arrows(), Morph(a, a, id_arrow_name(a.GetName()))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.Arrows().size();
            assert(cat.EraseArrow("Fake") == false);
            auto new_count = cat.Arrows().size();
            assert(prev_count == new_count);
         }
      }

      //============================================================
      // Testing of morphism compositions
      //============================================================
      {
         Cat cat("cat");

         solve_compositions(cat);

         Obj a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c, d});

         solve_compositions(cat);

         cat.AddArrows({Morph(a, b, "f0"), Morph(b, c, "f1"), Morph(c, d, "f2")});
         cat.AddArrows({Morph(d, c, "f3"), Morph(c, b, "f4"), Morph(b, a, "f5")});

         solve_compositions(cat);

         assert(fnCheckMorph(cat.Arrows(), Morph(a, c)));
         assert(fnCheckMorph(cat.Arrows(), Morph(a, d)));
         assert(fnCheckMorph(cat.Arrows(), Morph(b, d)));

         assert(fnCheckMorph(cat.Arrows(), Morph(d, b)));
         assert(fnCheckMorph(cat.Arrows(), Morph(d, a)));
         assert(fnCheckMorph(cat.Arrows(), Morph(c, a)));

         auto prev_count = cat.Arrows().size();
         solve_compositions(cat);
         auto new_count = cat.Arrows().size();

         assert(new_count == prev_count);
      }

      //============================================================
      // Testing morphism sequence
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d"), e("e"), f("f");

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Morph(a, b)
                  ,  Morph(b, a)
                  ,  Morph(a, c)
                  ,  Morph(b, d)
                  ,  Morph(c, d)
                  ,  Morph(c, f)
                  ,  Morph(d, e)});

         ObjVec seq = solve_sequence(cat, a, e);

         assert(seq.size() == 4);

         std::vector<Morph> morphs = map_obj2morphism(seq, cat);
         assert(morphs[0] == Morph(a, b));
         assert(morphs[1] == Morph(b, d));
         assert(morphs[2] == Morph(d, e));

         seq = solve_sequence(cat, e, a);

         assert(seq.size() == 0);
      }

      //============================================================
      // Testing morphism sequences
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d"), e("e"), f("f");

         cat.AddNodes({a, b, c, d, e, f});

         cat.AddArrows({
                     Morph(a, b)
                  ,  Morph(b, a)
                  ,  Morph(a, c)
                  ,  Morph(b, d)
                  ,  Morph(c, d)
                  ,  Morph(c, f)
                  ,  Morph(d, e)
                  ,  Morph(f, e)});

         std::vector<ObjVec> seqs = solve_sequences(cat, a, e);

         assert(seqs.size() == 3);

         {
            std::vector<Morph> morphs = map_obj2morphism(seqs[0], cat);
            assert(morphs[0] == Morph(a, b));
            assert(morphs[1] == Morph(b, d));
            assert(morphs[2] == Morph(d, e));
         }

         {
            std::vector<Morph> morphs = map_obj2morphism(seqs[1], cat);
            assert(morphs[0] == Morph(a, c));
            assert(morphs[1] == Morph(c, d));
            assert(morphs[2] == Morph(d, e));
         }

         {
            std::vector<Morph> morphs = map_obj2morphism(seqs[2], cat);
            assert(morphs[0] == Morph(a, c));
            assert(morphs[1] == Morph(c, f));
            assert(morphs[2] == Morph(f, e));
         }

         seqs = solve_sequences(cat, e, a);

         assert(seqs.size() == 0);
      }

      //============================================================
      // Testing of inversion
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d");

         cat.AddNodes({a, b, c, d});

         cat.AddArrow(Morph(a, b, "f0"));
         cat.AddArrow(Morph(a, c, "f1"));

         cat.AddArrow(Morph(b, d, "f2"));
         cat.AddArrow(Morph(c, d, "f3"));

         inverse(cat);

         assert(fnCheckMorph(cat.Arrows(), Morph(b, a, "f0")));
         assert(fnCheckMorph(cat.Arrows(), Morph(c, a, "f1")));

         assert(fnCheckMorph(cat.Arrows(), Morph(d, b, "f2")));
         assert(fnCheckMorph(cat.Arrows(), Morph(d, c, "f3")));
      }

      //============================================================
      // Testing of initial/terminal objects
      //============================================================
      {
         Cat cat("cat");

         Obj a0("a0"), a1("a1"), b("b"), c("c"), d0("d0"), d1("d1");

         cat.AddNodes({a0, a1, b, c, d0, d1});

         cat.AddArrows({Morph(a0, a1), Morph(a1, a0)});

         cat.AddArrow(Morph(a0, b));
         cat.AddArrow(Morph(a0, c));

         cat.AddArrow(Morph(a1, b));
         cat.AddArrow(Morph(a1, c));

         cat.AddArrow(Morph(b, d0));
         cat.AddArrow(Morph(c, d0));

         cat.AddArrow(Morph(b, d1));
         cat.AddArrow(Morph(c, d1));

         cat.AddArrows({Morph(d0, d1), Morph(d1, d0)});

         solve_compositions(cat);

         ObjVec initial_obj = initial(cat);
         assert(initial_obj.size() == 2);
         std::sort(initial_obj.begin(), initial_obj.end());
         assert(initial_obj[0] == a0);
         assert(initial_obj[1] == a1);

         ObjVec terminal_obj = terminal(cat);
         assert(terminal_obj.size() == 2);
         std::sort(terminal_obj.begin(), terminal_obj.end());
         assert(terminal_obj[0] == d0);
         assert(terminal_obj[1] == d1);

         cat.EraseNodes();

         cat.AddNode(a0);
         initial_obj = initial(cat);
         terminal_obj = terminal(cat);
         assert(initial_obj.size() == terminal_obj.size() == 1);
         assert(initial_obj[0] == terminal_obj[0]);
      }

      //============================================================
      // Coproduct test
      //============================================================
      {
         Obj a("a");
         a.SetValue("a");
         Obj b("b");
         b.SetValue("b");
         assert(coproduct(a, b)->Value() == Obj::TSet("ab"));

         Obj f("f");
         f.SetValue(4);
         Obj s("s");
         s.SetValue(5);
         assert(coproduct(f, s)->Value() == Obj::TSet(9));

         Obj fd("0.1");
         fd.SetValue(0.1);
         Obj sd("0.5");
         sd.SetValue(0.5);
         double result = std::get<double>(coproduct(fd, sd)->Value());
         assert(std::abs(result - 0.6) < std::numeric_limits<double>::epsilon());
      }

      //============================================================
      // Product test
      //============================================================
      {
         Obj a("ac");
         a.SetValue("ac");
         Obj b("bd");
         b.SetValue("bd");
         assert(product(a, b)->Value() == Obj::TSet("abadcbcd"));

         Obj f("4");
         f.SetValue(4);
         Obj s("5");
         s.SetValue(5);
         assert(product(f, s)->Value() == Obj::TSet(20));

         Obj fd("1.1");
         fd.SetValue(1.1);
         Obj sd("5");
         sd.SetValue(5.0);
         double result = std::get<double>(product(fd, sd)->Value());
         assert(std::abs(result - 5.5) < std::numeric_limits<double>::epsilon());
      }

      //============================================================
      // Testing functor
      //============================================================
      {
//         Cat C0("C0");
//         Obj a0("a0"), b0("b0");

//         C0.AddNodes(a0, b0);

//         Cat C1("C1");
//         Obj a1("a1"), b1("b1");
         
//         C1.AddNodes(a1, b1);

//         CACat ccat;
//         ccat.AddArrow(C0);
//         ccat.AddArrow(C1);

//         Func fn(C0.GetName(), C1.GetName());
      }

      print_info("End test");

      set_log_mode(lmode);
   }
}
