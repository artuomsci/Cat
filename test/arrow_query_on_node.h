#ifndef ARROW_QUERY_ON_NODE_H
#define ARROW_QUERY_ON_NODE_H

#include <assert.h>

#include "../include/node.h"
#include "../include/log.h"

namespace cat
{
   void test_arrow_query_on_node()
   {
      // Source category
      Node C0("C0", Node::EType::eSCategory);
      Node a0("a0", Node::EType::eObject), b0("b0", Node::EType::eObject);

      C0.AddNodes({a0, b0});
      C0.EmplaceArrow(a0, b0);

      // Target category
      Node C1("C1", Node::EType::eSCategory);
      Node a1("a1", Node::EType::eObject), b1("b1", Node::EType::eObject);

      C1.AddNodes({a1, b1});
      C1.EmplaceArrow(a1, b1);

      // Category of categories
      Node ccat("Cat", Node::EType::eLCategory);
      ccat.AddNode(C0);
      ccat.AddNode(C1);

      Arrow functor(C0, C1);
      functor.EmplaceArrow(a0, a1);
      functor.EmplaceArrow(b0, b1);

      assert(ccat.AddArrow(functor));

      auto ret = ccat.QueryArrows(Arrow(C0.Name(), C1.Name()).AsQuery());
      assert(ret.front().Name() == Arrow(C0, C1).Name());
   }
}

#endif // ARROW_QUERY_ON_NODE_H
