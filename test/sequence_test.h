#ifndef SEQUENCE_TEST_H
#define SEQUENCE_TEST_H

#include <assert.h>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   void test_sequence()
   {
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
   }
}

#endif // SEQUENCE_TEST_H
