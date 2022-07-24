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

         assert(cat.GetObjects().size() == 0);

         assert(cat.AddObjects(a, b));

         assert(cat.GetObjects().size() == 2);

         assert(cat.AddObject(a) == false);

         assert(cat.GetObjects().size() == 2);
      }

      //============================================================
      // Testing of object deletion methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c");

         cat.AddObjects(a, b);

         cat.AddMorphism(Morph(a, b));

         assert(cat.EraseObject(a) == true);

         assert(cat.GetMorphisms().size() == 1);

         assert(cat.EraseObject(b) == true);

         assert(cat.GetMorphisms().size() == 0);

         assert(cat.EraseObject(c) == false);

         assert(cat.GetObjects().size() == 0);

         cat.AddObjects(a, b);

         cat.EraseObjects();

         assert(cat.GetObjects().size() == 0);
         assert(cat.GetMorphisms().size() == 0);
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

         cat.AddObjects(a, b, c);

         assert(cat.MatchMorphism(a, a));
         assert(cat.MatchMorphism(b, b));
         assert(cat.MatchMorphism(c, c));

         assert(cat.GetMorphisms().size() == 3);

         assert(cat.AddMorphism(a, b, "f0"));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(a, b, "f0")));

         assert(cat.AddMorphism(a, c, "f1"));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(a, c, "f1")));

         assert(cat.GetMorphisms().size() == 5);

         assert(cat.AddMorphism(a, d, "f2") == false);
         assert(!fnCheckMorph(cat.GetMorphisms(), Morph(a, d, "f2")));
         assert(cat.GetMorphisms().size() == 5);

         assert(cat.AddMorphism(b, c, "f0") == false);
         assert(cat.AddMorphism(a, b, "f0"));
      }

      //============================================================
      // Testing of morphism deletion methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c");

         cat.AddObjects(a, b, c);

         cat.AddMorphism(a, b, "f0");
         cat.AddMorphism(a, c, "f1");

         // Deleting morphisms one at a time
         {
            auto prev_count = cat.GetMorphisms().size();
            assert(cat.EraseMorphism("f0") == true);
            assert(!fnCheckMorph(cat.GetMorphisms(), Morph(a, b, "f0")));
            assert(prev_count = cat.GetMorphisms().size() - 1);

            assert(cat.EraseMorphism("f1") == true);
            assert(!fnCheckMorph(cat.GetMorphisms(), Morph(a, c, "f1")));
            assert(prev_count = cat.GetMorphisms().size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.GetMorphisms().size();
            assert(cat.EraseMorphism(id_morph_name(a)) == false);
            auto new_count = cat.GetMorphisms().size();
            assert(prev_count == new_count);
            assert(fnCheckMorph(cat.GetMorphisms(), Morph(a, a, id_morph_name(a))));
         }

         // Non existent morphism can't be deleted
         {
            auto prev_count = cat.GetMorphisms().size();
            assert(cat.EraseMorphism("Fake") == false);
            auto new_count = cat.GetMorphisms().size();
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

         cat.AddObjects(a, b, c, d);

         solve_compositions(cat);

         cat.AddMorphisms(Morph(a, b, "f0"), Morph(b, c, "f1"), Morph(c, d, "f2"));
         cat.AddMorphisms(Morph(d, c, "f3"), Morph(c, b, "f4"), Morph(b, a, "f5"));

         solve_compositions(cat);

         assert(fnCheckMorph(cat.GetMorphisms(), Morph(a, c)));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(a, d)));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(b, d)));

         assert(fnCheckMorph(cat.GetMorphisms(), Morph(d, b)));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(d, a)));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(c, a)));

         auto prev_count = cat.GetMorphisms().size();
         solve_compositions(cat);
         auto new_count = cat.GetMorphisms().size();

         assert(new_count == prev_count);
      }

      //============================================================
      // Testing morphism sequence
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d"), e("e"), f("f");

         cat.AddObjects(a, b, c, d, e, f);

         cat.AddMorphisms(
                     Morph(a, b)
                  ,  Morph(b, a)
                  ,  Morph(a, c)
                  ,  Morph(b, d)
                  ,  Morph(c, d)
                  ,  Morph(c, f)
                  ,  Morph(d, e));

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

         cat.AddObjects(a, b, c, d, e, f);

         cat.AddMorphisms(
                     Morph(a, b)
                  ,  Morph(b, a)
                  ,  Morph(a, c)
                  ,  Morph(b, d)
                  ,  Morph(c, d)
                  ,  Morph(c, f)
                  ,  Morph(d, e)
                  ,  Morph(f, e));

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

         cat.AddObjects(a, b, c, d);

         cat.AddMorphism(a, b, "f0");
         cat.AddMorphism(a, c, "f1");

         cat.AddMorphism(b, d, "f2");
         cat.AddMorphism(c, d, "f3");

         inverse(cat);

         assert(fnCheckMorph(cat.GetMorphisms(), Morph(b, a, "f0")));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(c, a, "f1")));

         assert(fnCheckMorph(cat.GetMorphisms(), Morph(d, b, "f2")));
         assert(fnCheckMorph(cat.GetMorphisms(), Morph(d, c, "f3")));
      }

      //============================================================
      // Testing of initial/terminal objects
      //============================================================
      {
         Cat cat("cat");

         Obj a0("a0"), a1("a1"), b("b"), c("c"), d0("d0"), d1("d1");

         cat.AddObjects(a0, a1, b, c, d0, d1);

         cat.AddMorphisms(Morph(a0, a1), Morph(a1, a0));

         cat.AddMorphism(Morph(a0, b));
         cat.AddMorphism(Morph(a0, c));

         cat.AddMorphism(Morph(a1, b));
         cat.AddMorphism(Morph(a1, c));

         cat.AddMorphism(Morph(b, d0));
         cat.AddMorphism(Morph(c, d0));

         cat.AddMorphism(Morph(b, d1));
         cat.AddMorphism(Morph(c, d1));

         cat.AddMorphisms(Morph(d0, d1), Morph(d1, d0));

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

         cat.EraseObjects();

         cat.AddObject(a0);
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
         Obj b("b");
         assert(coproduct(a, b, eCProdType::eStr) == Obj("ab"));

         Obj f("4");
         Obj s("5");
         assert(coproduct(f, s, eCProdType::eInt) == Obj("9"));

         Obj fd("0.1");
         Obj sd("0.5");
         double result = std::stod(coproduct(fd, sd, eCProdType::eReal).GetName());
         assert(std::abs(result - 0.6) < std::numeric_limits<double>::epsilon());
      }

      //============================================================
      // Product test
      //============================================================
      {
         Obj a("ac");
         Obj b("bd");
         assert(product(a, b, eCProdType::eStr) == Obj("abadcbcd"));

         Obj f("4");
         Obj s("5");
         assert(product(f, s, eCProdType::eInt) == Obj("20"));

         Obj fd("1.1");
         Obj sd("5");
         double result = std::stod(product(fd, sd, eCProdType::eReal).GetName());
         assert(std::abs(result - 5.5) < std::numeric_limits<double>::epsilon());
      }

      //============================================================
      // Testing functor
      //============================================================
      {
         Cat C0("C0");
         Obj a0("a0"), b0("b0");

         C0.AddObjects(a0, b0);

         Cat C1("C1");
         Obj a1("a1"), b1("b1");
         
         C1.AddObjects(a1, b1);

         CACat ccat;
         ccat.AddCategory(C0);
         ccat.AddCategory(C1);

         Func fn(C0.GetName(), C1.GetName());
      }

      print_info("End test");

      set_log_mode(lmode);
   }
}
