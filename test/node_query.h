#pragma once

#include <assert.h>

#include "../include/node.h"
#include "parser.h"

namespace cat
{
   //============================================================
   // Testing of node query
   //============================================================
   void test_node_query()
   {
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

      auto src = R"(
SCAT cat
{
   OBJ a, b, c, d;
}
      )";

      Parser prs;
      prs.ParseSource(src);

      Node S = *prs.Data();

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
}
