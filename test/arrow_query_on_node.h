#ifndef ARROW_QUERY_ON_NODE_H
#define ARROW_QUERY_ON_NODE_H

#include <assert.h>
#include <algorithm>

#include "../include/node.h"
#include "parser.h"

namespace cat
{
   void test_arrow_query_on_node()
   {
      auto fnCheckArrow = [](const Arrow::List& arrows_, std::optional<std::string> source_, std::optional<std::string> target_, std::optional<std::string> name_)
      {
         auto it = std::find_if(arrows_.begin(), arrows_.end(), [&](const Arrow::List::value_type& element_)
         {
            bool source{true};
            if (source_)
            {
               source = element_.Source() == source_;
            }

            bool target{true};
            if (target_)
            {
               target = element_.Target() == target_;
            }

            bool name{true};
            if (name_)
            {
               name = element_.Name() == name_;
            }


            return source && target && name;
         });

         return it != arrows_.end();
      };

      auto src = R"(
SCAT C
{
OBJ a, b, c;

a-[*]->b;
b-[*]->c;
a-[*]->c;
}
      )";

      Parser prs;
      prs.ParseSource(src);

      Node C = *prs.Data();

      Arrow::List ret = C.QueryArrows(Arrow("a", "b", "*").AsQuery());
      assert(ret.size() == 1);
      assert(fnCheckArrow(ret, "a", "b", {}));

      ret = C.QueryArrows(Arrow("a", "*", "*").AsQuery());

      assert(ret.size() == 3);
      assert(fnCheckArrow(ret, "a", "a", {}));
      assert(fnCheckArrow(ret, "a", "b", {}));
      assert(fnCheckArrow(ret, "a", "c", {}));

      ret = C.QueryArrows(Arrow("*", "c", "*").AsQuery());
      assert(ret.size() == 3);
      assert(fnCheckArrow(ret, "c", "c", {}));
      assert(fnCheckArrow(ret, "b", "c", {}));
      assert(fnCheckArrow(ret, "a", "c", {}));

      ret = C.QueryArrows(Arrow("*", "*", "b_c").AsQuery());
      assert(ret.size() == 1);
      assert(fnCheckArrow(ret, "b", "c", {}));
   }
}

#endif // ARROW_QUERY_ON_NODE_H
