#ifndef NODE_QUERY_BY_ARROW_TEST_H
#define NODE_QUERY_BY_ARROW_TEST_H

#include <assert.h>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   //============================================================
   // Testing of arrow query (patterns)
   //============================================================
   void test_node_query_by_arrow()
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
}

#endif // NODE_QUERY_BY_ARROW_TEST_H