#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>

#include "cat_export.h"
#include "node.h"
#include "tokenizer.h"

namespace cat {
class CAT_EXPORT Parser {
public:
  using TKIt = std::list<TToken>::iterator;

  bool Parse(const std::string &filename_);
  bool ParseSource(const std::string &src_);
  std::shared_ptr<Node> Data() const;

  static bool parse_arrow(TKIt &it_, TKIt end_, Arrow::List &arrows_);

private:
  using NodePtr = std::shared_ptr<Node>;

  bool parse_CAT(TKIt &it_, TKIt end_, cat::Node::EType type_,
                 NodePtr &pNode_) const;
  bool parse_OBJ(TKIt &it_, TKIt end_, NodePtr pNode_) const;
  bool parse_arrow(TKIt &it_, TKIt end_, NodePtr pNode_) const;
  bool parse_statement(TKIt &it_, TKIt end_, NodePtr pNode_) const;

  NodePtr m_pNode{};
};
} // namespace cat

#endif // PARSER_H
