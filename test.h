#include <assert.h>

#include "cat.h"

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

         cat.AddMorphism(MorphDef(a, b));

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

      //============================================================
      // Testing of morphism addition methods
      //============================================================
      {
         Cat cat("cat");

         Obj a("a"), b("b"), c("c"), d("d");

         cat.AddObjects(a, b, c);

         assert(cat.GetMorphisms().find(MorphDef(a, a)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(b, b)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(c, c)) != cat.GetMorphisms().end());

         assert(cat.GetMorphisms().size() == 3);

         assert(cat.AddMorphism(a, b, "f0"));
         assert(cat.GetMorphisms().find(MorphDef(a, b, "f0")) != cat.GetMorphisms().end());

         assert(cat.AddMorphism(a, c, "f1"));
         assert(cat.GetMorphisms().find(MorphDef(a, c, "f1")) != cat.GetMorphisms().end());

         assert(cat.GetMorphisms().size() == 5);

         assert(cat.AddMorphism(a, d, "f2") == false);
         assert(cat.GetMorphisms().find(MorphDef(a, d, "f2")) == cat.GetMorphisms().end());
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
            assert(cat.GetMorphisms().find(MorphDef(a, b, "f0")) == cat.GetMorphisms().end());
            assert(prev_count = cat.GetMorphisms().size() - 1);

            assert(cat.EraseMorphism("f1") == true);
            assert(cat.GetMorphisms().find(MorphDef(a, c, "f1")) == cat.GetMorphisms().end());
            assert(prev_count = cat.GetMorphisms().size() - 2);
         }

         // It is not allowed to delete identity morphism,
         // before the object has been removed
         {
            auto prev_count = cat.GetMorphisms().size();
            assert(cat.EraseMorphism(id_morph_name(a)) == false);
            auto new_count = cat.GetMorphisms().size();
            assert(prev_count == new_count);
            assert(cat.GetMorphisms().find(MorphDef(a, a, id_morph_name(a))) != cat.GetMorphisms().end());
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

         cat.AddMorphisms(MorphDef(a, b, "f0"), MorphDef(b, c, "f1"), MorphDef(c, d, "f2"));
         cat.AddMorphisms(MorphDef(d, c, "f3"), MorphDef(c, b, "f4"), MorphDef(b, a, "f5"));

         solve_compositions(cat);

         assert(cat.GetMorphisms().find(MorphDef(a, c)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(a, d)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(b, d)) != cat.GetMorphisms().end());

         assert(cat.GetMorphisms().find(MorphDef(d, b)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(d, a)) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(c, a)) != cat.GetMorphisms().end());

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
                     MorphDef(a, b)
                  ,  MorphDef(b, a)
                  ,  MorphDef(a, c)
                  ,  MorphDef(b, d)
                  ,  MorphDef(c, d)
                  ,  MorphDef(c, f)
                  ,  MorphDef(d, e));

         ObjVec seq = solve_sequence(cat, a, e);

         assert(seq.size() == 4);

         std::vector<MorphDef> morphs = map_obj2morphism(seq, cat);
         assert(morphs[0] == MorphDef(a, b));
         assert(morphs[1] == MorphDef(b, d));
         assert(morphs[2] == MorphDef(d, e));

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
                     MorphDef(a, b)
                  ,  MorphDef(b, a)
                  ,  MorphDef(a, c)
                  ,  MorphDef(b, d)
                  ,  MorphDef(c, d)
                  ,  MorphDef(c, f)
                  ,  MorphDef(d, e)
                  ,  MorphDef(f, e));

         std::vector<ObjVec> seqs = solve_sequences(cat, a, e);

         assert(seqs.size() == 3);

         {
            std::vector<MorphDef> morphs = map_obj2morphism(seqs[0], cat);
            assert(morphs[0] == MorphDef(a, b));
            assert(morphs[1] == MorphDef(b, d));
            assert(morphs[2] == MorphDef(d, e));
         }

         {
            std::vector<MorphDef> morphs = map_obj2morphism(seqs[1], cat);
            assert(morphs[0] == MorphDef(a, c));
            assert(morphs[1] == MorphDef(c, d));
            assert(morphs[2] == MorphDef(d, e));
         }

         {
            std::vector<MorphDef> morphs = map_obj2morphism(seqs[2], cat);
            assert(morphs[0] == MorphDef(a, c));
            assert(morphs[1] == MorphDef(c, f));
            assert(morphs[2] == MorphDef(f, e));
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

         assert(cat.GetMorphisms().find(MorphDef(b, a, "f0")) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(c, a, "f1")) != cat.GetMorphisms().end());

         assert(cat.GetMorphisms().find(MorphDef(d, b, "f2")) != cat.GetMorphisms().end());
         assert(cat.GetMorphisms().find(MorphDef(d, c, "f3")) != cat.GetMorphisms().end());
      }

      //============================================================
      // Testing of initial/terminal objects
      //============================================================
      {
         Cat cat("cat");

         Obj a0("a0"), a1("a1"), b("b"), c("c"), d0("d0"), d1("d1");

         cat.AddObjects(a0, a1, b, c, d0, d1);

         cat.AddMorphisms(MorphDef(a0, a1), MorphDef(a1, a0));

         cat.AddMorphism(MorphDef(a0, b));
         cat.AddMorphism(MorphDef(a0, c));

         cat.AddMorphism(MorphDef(a1, b));
         cat.AddMorphism(MorphDef(a1, c));

         cat.AddMorphism(MorphDef(b, d0));
         cat.AddMorphism(MorphDef(c, d0));

         cat.AddMorphism(MorphDef(b, d1));
         cat.AddMorphism(MorphDef(c, d1));

         cat.AddMorphisms(MorphDef(d0, d1), MorphDef(d1, d0));

         solve_compositions(cat);

         ObjVec initial_obj = initial(cat);
         assert(initial_obj.size() == 2);
         assert(initial_obj[0] == a1);
         assert(initial_obj[1] == a0);

         ObjVec terminal_obj = terminal(cat);
         assert(terminal_obj.size() == 2);
         assert(terminal_obj[0] == d1);
         assert(terminal_obj[1] == d0);

         cat.EraseObjects();

         cat.AddObject(a0);
         initial_obj = initial(cat);
         terminal_obj = terminal(cat);
         assert(initial_obj.size() == terminal_obj.size() == 1);
         assert(initial_obj[0] == terminal_obj[0]);
      }

      print_info("End test");

      set_log_mode(lmode);
   }
}
