#pragma once

#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat {
void test_sequence() {
  //============================================================
  // Testing morphism sequence
  //============================================================
  {
    auto src = R"(
SCAT cat
{
   OBJ a, b, c, d, e, f;

   a -[*]-> b{};
   b -[*]-> a{};
   a -[*]-> c{};
   b -[*]-> d{};
   c -[*]-> d{};
   c -[*]-> f{};
   d -[*]-> e{};
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    std::list<Node::NName> seq = cat.SolveSequence("a", "e");

    assert(seq.size() == 4);

    Arrow::List morphs = cat.MapNodes2Arrows(seq);
    auto it = morphs.begin();
    assert(*(it) == Arrow("a", "b"));
    assert(*(++it) == Arrow("b", "d"));
    assert(*(++it) == Arrow("d", "e"));

    seq = cat.SolveSequence("e", "a");

    assert(seq.size() == 0);
  }

  //============================================================
  // Testing morphism sequences
  //============================================================
  {
    auto src = R"(
SCAT cat
{
   OBJ a, b, c, d, e, f;

   a -[*]-> b{};
   b -[*]-> a{};
   a -[*]-> c{};
   b -[*]-> d{};
   c -[*]-> d{};
   c -[*]-> f{};
   d -[*]-> e{};
   f -[*]-> e{};
}
         )";

    Parser prs;
    prs.ParseSource(src);

    Node cat = *prs.Data();

    std::list<std::list<Node::NName>> seqs = cat.SolveSequences("a", "e");

    assert(seqs.size() == 3);

    auto it = seqs.begin();

    {
      Arrow::List morphs = cat.MapNodes2Arrows(*it);
      auto it = morphs.begin();
      assert(*it == Arrow("a", "b"));
      assert(*(++it) == Arrow("b", "d"));
      assert(*(++it) == Arrow("d", "e"));
    }

    it = std::next(it);

    {
      Arrow::List morphs = cat.MapNodes2Arrows(*it);
      auto it = morphs.begin();
      assert(*(it) == Arrow("a", "c"));
      assert(*(++it) == Arrow("c", "d"));
      assert(*(++it) == Arrow("d", "e"));
    }

    it = std::next(it);

    {
      Arrow::List morphs = cat.MapNodes2Arrows(*it);
      auto it = morphs.begin();
      assert(*(it) == Arrow("a", "c"));
      assert(*(++it) == Arrow("c", "f"));
      assert(*(++it) == Arrow("f", "e"));
    }

    seqs = cat.SolveSequences("e", "a");

    assert(seqs.size() == 0);
  }
}
} // namespace cat
