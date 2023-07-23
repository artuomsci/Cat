#include "parser.h"

#include <string.h>
#include <fstream>
#include <sstream>

#include "log.h"

using namespace cat;

static const char* comment_begin = "/*";
static const char* comment_end   = "*/";

//-----------------------------------------------------------------------------------------
bool Parser::parse_statement(TKIt& it_, TKIt end_, NodePtr pNode_) const
{
   while (it_ != end_)
   {
      TToken& tk = *it_;

      // End of statement
      if (std::holds_alternative<END_CBR>(tk))
         return true;

      // Small category declaration
      if (std::holds_alternative<SCAT>(tk))
      {
         if (pNode_->Type() != Node::EType::eLCategory)
         {
            print_error("Incorrect SCAT declaration");
            return false;
         }

         if (!parse_CAT(it_, end_, cat::Node::EType::eSCategory, pNode_))
         {
            return false;
         }
      }

      // Object declaration
      if (std::holds_alternative<OBJ>(tk))
      {
         if (pNode_->Type() != Node::EType::eSCategory)
         {
            print_error("Incorrect OBJ declaration");
            return false;
         }

         if (!parse_OBJ(it_, end_, pNode_))
         {
            return false;
         }
      }

      // Word declaration
      if (std::holds_alternative<std::string>(tk) || std::holds_alternative<ASTERISK>(tk))
      {
         if (!parse_arrow(it_, end_, pNode_))
         {
            print_error("Incorrect arrow declaration");
            return false;
         }
      }

      ++it_;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_CAT(TKIt& it_, TKIt end_, cat::Node::EType type_, NodePtr& pNode_) const
{
   ++it_;

   if (!std::holds_alternative<std::string>(*it_))
   {
      print_error("Incorrect CAT declaration, name expected");
      return false;
   }

   std::string name = std::get<std::string>(*it_);

   ++it_;

   if (std::holds_alternative<SEMICOLON>(*it_))
   {
      if (pNode_)
         return pNode_->AddNode(Node(name, type_));

      pNode_.reset(new Node(name, type_));
      return true;
   }

   if (!std::holds_alternative<BEGIN_CBR>(*it_))
   {
      print_error("Incorrect CAT declaration");
      return false;
   }

   NodePtr pNewNode(new Node(name, type_));

   if (!parse_statement(it_, end_, pNewNode))
      return false;

   if (!pNode_)
      pNode_ = pNewNode;
   else
      pNode_->AddNode(*pNewNode);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_OBJ(TKIt& it_, TKIt end_, NodePtr pNode_) const
{
   while (true)
   {
      ++it_;

      std::string name;

      if (std::holds_alternative<std::string>(*it_))
      {
         name = std::get<std::string>(*it_);
      }
      else if (std::holds_alternative<int>(*it_))
      {
         name = std::to_string(std::get<int>(*it_));
      }
      else
      {
         print_error("Incorrect OBJ declaration, name expected");
         return false;
      }

      Node obj(name, Node::EType::eObject);

      pNode_->AddNode(obj);

      ++it_;

      if (std::holds_alternative<SEMICOLON>(*it_))
      {
         break;
      }
      else if (!std::holds_alternative<COMMA>(*it_))
      {
         print_error("Incorrect OBJ declaration");
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_arrow(TKIt& it_, TKIt end_, NodePtr pNode_) const
{
   if (!pNode_)
      return false;

   Arrow::List arrows;
   if (!parse_arrow(it_, end_, arrows))
      return false;

   std::string any(1, ASTERISK::id);

   for (auto& arrow : arrows)
   {
      if (arrow.Name() == any)
      {
         arrow = Arrow(arrow.Source(), arrow.Target());
      }
   }

   for (const auto& arrow : arrows)
   {
      if (!pNode_->AddArrow(arrow))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_arrow(TKIt& it_, TKIt end_, Arrow::List& arrows_)
{
   while (true)
   {
      if (it_ == end_)
         return true;

      std::string source;

      if (std::holds_alternative<ASTERISK>(*it_))
      {
         source = ASTERISK::id;
      }
      else if (std::holds_alternative<std::string>(*it_))
      {
         source = std::get<std::string>(*it_);
      }
      else if (std::holds_alternative<int>(*it_))
      {
         source = std::to_string(std::get<int>(*it_));
      }
      else
      {
         print_error("Incorrect arrow declaration");
         return false;
      }

      ++it_;
      if (it_ == end_)
         return false;

      ++it_;
      if (it_ == end_)
         return false;

      TToken name_tk = *it_;

      ++it_;
      if (it_ == end_)
         return false;

      ++it_;
      if (it_ == end_)
         return false;

      std::string target;

      if (std::holds_alternative<ASTERISK>(*it_))
      {
         target = ASTERISK::id;
      }
      else if (std::holds_alternative<std::string>(*it_))
      {
         target = std::get<std::string>(*it_);
      }
      else if (std::holds_alternative<int>(*it_))
      {
         target = std::to_string(std::get<int>(*it_));
      }
      else
      {
         print_error("Incorrect arrow declaration");
         return false;
      }

      std::string name;

      if (std::holds_alternative<std::string>(name_tk))
      {
         name = std::get<std::string>(name_tk);
      }
      else if (std::holds_alternative<int>(name_tk))
      {
         name = std::to_string(std::get<int>(name_tk));
      }
      else if (std::holds_alternative<ASTERISK>(name_tk))
      {
         name = ASTERISK::id;
      }
      else
      {
         print_error("Incorrect arrow declaration. Name expected.");
         return false;
      }

      Arrow arrow(source, target, name);

      arrows_.push_back(arrow);

      ++it_;
      if (it_ == end_)
         return false;

      if (std::holds_alternative<SEMICOLON>(*it_))
      {
         break;
      }
      else if (std::holds_alternative<BEGIN_SINGLE_ARROW>(*it_))
      {
         --it_;
      }
      else
      {
         print_error("Incorrect arrow declaration");
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
static std::optional<std::string> remove_comments(std::string string_)
{
   size_t initial = 0;
   while (true)
   {
      auto begin     = string_.find(comment_begin  , initial);
      auto new_end   = string_.find(comment_end    , initial);

      if (new_end < begin)
      {
         print_error("Comment's section is not opened.");
         return std::optional<std::string>();
      }

      if (begin != std::string::npos)
      {
         initial = begin;

         auto end       = string_.find(comment_end    , begin);
         auto new_start = string_.find(comment_begin  , begin + 1);

         if (end == std::string::npos || (new_start < end))
         {
            print_error("Comment's section is not closed.");
            return std::optional<std::string>();
         }

         string_.erase(begin, end - begin + strlen(comment_end));
      }
      else
         break;
   }

   return string_;
}

//-----------------------------------------------------------------------------------------
bool Parser::Parse(const std::string& filename_)
{
   std::ifstream file(filename_);
   if (!file.is_open())
   {
      print_error("Error opening file");
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();

   return ParseSource(buffer.str());
}

//-----------------------------------------------------------------------------------------
bool Parser::ParseSource(const std::string& src_)
{
   auto prep = remove_comments(src_);
   if (!prep)
      return false;

   std::list<TToken> tokens = Tokenizer::Process(prep.value());

   TKIt it = tokens.begin();
   while (it != tokens.end())
   {
      TToken& tk = *it;

      if  (std::holds_alternative<LCAT>(tk))
      {
         if (!parse_CAT(it, tokens.end(), cat::Node::EType::eLCategory, m_pNode))
            return false;
      }
      else if (std::holds_alternative<SCAT>(tk))
      {
         if (!parse_CAT(it, tokens.end(), cat::Node::EType::eSCategory, m_pNode))
            return false;
      }
      else if (std::holds_alternative<OBJ>(tk))
      {
         if (!parse_CAT(it, tokens.end(), cat::Node::EType::eObject, m_pNode))
            return false;
      }
      else if (std::holds_alternative<SEMICOLON>(tk))
      {
      }
      else
         return false;

      ++it;
   }
}

//-----------------------------------------------------------------------------------------
std::shared_ptr<Node> Parser::Data() const
{
   return m_pNode;
};
