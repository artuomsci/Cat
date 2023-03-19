#include "parser.h"

#include <fstream>
#include <sstream>

#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
bool Parser::parse_statement(TKIt& it_, TKIt end_, NodePtr pNode_)
{
   while (it_ != end_)
   {
      TToken& tk = *it_;

      // End of statement
      if (std::holds_alternative<END_BR>(tk))
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
bool Parser::parse_CAT(TKIt& it_, TKIt end_, cat::Node::EType type_, NodePtr& pNode_)
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

   if (!std::holds_alternative<BEGIN_BR>(*it_))
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
bool Parser::parse_OBJ(TKIt& it_, TKIt end_, NodePtr pNode_)
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
bool Parser::parse_arrow(TKIt& it_, TKIt end_, NodePtr pNode_)
{
   TToken name_tk = *it_;

   ++it_;

   if (!std::holds_alternative<COLON>(*it_))
   {
      print_error("Incorrect arrow declaration");
      return false;
   }

   ++it_;

   std::string source;

   if (std::holds_alternative<std::string>(*it_))
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

   Arrow::EType type { Arrow::EType::eUndefined };

   if (std::holds_alternative<SINGLE_ARROW>(*it_))
   {
      if (pNode_->Type() != Node::EType::eSCategory)
      {
         print_error("Incorrect arrow declaration");
         return false;
      }

      type = Arrow::EType::eMorphism;
   }
   else if (std::holds_alternative<DOUBLE_ARROW>(*it_))
   {
      if (pNode_->Type() != Node::EType::eLCategory)
      {
         print_error("Incorrect arrow declaration");
         return false;
      }

      type = Arrow::EType::eFunctor;
   }

   ++it_;

   std::string target;

   if (std::holds_alternative<std::string>(*it_))
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
   else if (std::holds_alternative<ASTERISK>(name_tk))
   {
      name = Arrow::DefaultArrowName(source, target);
   }
   else
   {
      print_error("Incorrect arrow declaration. Name expected.");
      return false;
   }

   Arrow arrow(type, source, target, name);

   if (!pNode_->AddArrow(arrow))
      return false;

   ++it_;

   if (!std::holds_alternative<SEMICOLON>(*it_))
   {
      print_error("Incorrect arrow declaration");
      return false;
   }

   return true;
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

   std::list<TToken> tokens = Tokenizer::Process(buffer.str());

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

   return true;
}
