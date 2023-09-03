#include "node.h"

#include "executor.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Executor &Executor::Inst() {
  static Executor reg;
  return reg;
}

//-----------------------------------------------------------------------------------------
bool Executor::exec(Node &node_) {
  Node::List beginNodes = node_.Initial();
  Node::List endNodes = node_.Terminal();

  for (const auto &begin : beginNodes) {
    for (const auto &end : endNodes) {
      std::list<Node::NName> nodeChain =
          node_.SolveSequence(begin.Name(), end.Name());

      auto itEnd = std::prev(nodeChain.end());
      for (auto it = nodeChain.begin(); it != itEnd; ++it) {
        auto sourceList = node_.QueryNodes(*(std::next(it, 0)));
        if (sourceList.empty()) {
          return false;
        }
        const auto &source = sourceList.front();

        auto targetList = node_.QueryNodes(*(std::next(it, 1)));
        if (targetList.empty()) {
          return false;
        }
        auto &target = targetList.front();

        auto arrow = node_.QueryArrows(
            Arrow(source.Name(), target.Name(), "*").AsQuery());
        if (arrow.empty()) {
          return false;
        }

        auto mapTarget = arrow.front().Map(source);
        if (!mapTarget.has_value()) {
          return false;
        }

        node_.ReplaceNode(mapTarget.value());
      }
    }
  }

  return true;
}
