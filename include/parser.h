#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>

#include "tokenizer.h"
#include "node.h"

namespace cat
{
   class Parser
   {
   public:
      bool Parse(const std::string& filename_);
      std::shared_ptr<Node> Data() const { return m_pNode; };

   private:

      using TKIt     = std::list<TToken>::iterator;
      using NodePtr  = std::shared_ptr<Node>;

      bool parse_CAT(TKIt& it_, TKIt end_, cat::Node::EType type_, NodePtr& pNode_);
      bool parse_OBJ(TKIt& it_, TKIt end_, NodePtr pNode_);
      bool parse_arrow(TKIt& it_, TKIt end_, NodePtr pNode_);
      bool parse_statement(TKIt& it_, TKIt end_, NodePtr pNode_);

      NodePtr m_pNode {};
   };
}

#endif // PARSER_H