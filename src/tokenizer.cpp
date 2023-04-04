#include "tokenizer.h"

#include <regex>
#include <tuple>

using namespace cat;

//-----------------------------------------------------------------------------------------
enum TokenGroup
{
   eDelimeter,
   eKey,
   eSkip
};

//-----------------------------------------------------------------------------------------
template <typename T, size_t sz>
static bool IsInGroup(const T (&group_)[sz], std::string::value_type symbol_)
{
   for (const auto& key : group_)
      if (key == symbol_)
         return true;

   return false;
}

template <TokenGroup>
static bool IsToken(std::string::value_type symbol_);

template <>
bool IsToken<TokenGroup::eDelimeter>(std::string::value_type symbol_)
{
   return IsInGroup({ COMMA::id, BEGIN_BR::id, END_BR::id, SEMICOLON::id, COLON::id }, symbol_);
}

template <>
bool IsToken<TokenGroup::eKey>(std::string::value_type symbol_)
{
   return IsInGroup({ ASTERISK::id, QUOTE::id }, symbol_);
}

template <>
bool IsToken<TokenGroup::eSkip>(std::string::value_type symbol_)
{
   return IsInGroup({ TAB::id, SPACE::id, NEXT_LINE::id }, symbol_);
}

//-----------------------------------------------------------------------------------------
template <typename T>
static void conditional_add(std::list<TToken>& tokens_, std::string::value_type symbol_, T t_)
{
   if (t_.id == symbol_)
      tokens_.push_back(t_);
}

//-----------------------------------------------------------------------------------------
template <typename T>
static void AddByGroup(std::list<TToken>& tokens_, std::string::value_type symbol_, T stub)
{
   std::apply([&](auto&&... args_) {
      ((conditional_add(tokens_, symbol_, args_)), ...);
   }, stub);
}

template <TokenGroup>
static void AddSpecialToken(std::list<TToken>& tokens_, std::string::value_type symbol_);

template <>
void AddSpecialToken<TokenGroup::eDelimeter>(std::list<TToken>& tokens_, std::string::value_type symbol_)
{
   static const TDelimeterT stub;
   AddByGroup(tokens_, symbol_, stub);
}

template <>
void AddSpecialToken<TokenGroup::eKey>(std::list<TToken>& tokens_, std::string::value_type symbol_)
{
   static const TKeyT stub;
   AddByGroup(tokens_, symbol_, stub);
}

//-----------------------------------------------------------------------------------------
struct SMNode
{
   SMNode(char ch_) : chr(ch_){}
   SMNode(){}

   std::vector<SMNode>  child;
   char                 chr { NEXT_LINE::id };
};

//-----------------------------------------------------------------------------------------
static SMNode* find_symbol_node(std::vector<SMNode>& nodes_, char symbol_)
{
   for (auto& node : nodes_)
   {
      if (node.chr == symbol_)
         return &node;
   }

   return nullptr;
}

//-----------------------------------------------------------------------------------------
static void build_seq_tree(SMNode& root_)
{
   std::vector<std::string> seqs
   {
         BEGIN_SINGLE_ARROW::id
      ,  END_SINGLE_ARROW::id
      ,  BEGIN_DOUBLE_ARROW::id
      ,  END_DOUBLE_ARROW::id
   };

   for (const auto& seq : seqs)
   {
      SMNode* crt = &root_;

      for (const auto& symbol : seq)
      {
         SMNode* pNextSymbol = find_symbol_node(crt->child, symbol);

         if (!pNextSymbol)
         {
            crt->child.push_back(symbol);

            crt = &crt->child.back();
         }
         else
            crt = pNextSymbol;
      }

      crt->child.push_back(NEXT_LINE::id);
   }
}

//-----------------------------------------------------------------------------------------
static bool AddToken(std::list<TToken>& tokens_, std::string::const_iterator begin_, std::string::const_iterator end_)
{
   if (begin_ == end_)
      return true;

   std::string tk(begin_, end_);

   // Int
   if       (std::regex_match(tk, std::regex("^-?[0-9]+$")))
   {
      tokens_.push_back(TToken(std::stoi(tk)));
   }
   // Float
   else if  (std::regex_match(tk, std::regex("^-?[0-9]*\\.[0-9]+f$")))
   {
      tokens_.push_back(TToken(std::stof(tk)));
   }
   // Double
   else if  (std::regex_match(tk, std::regex("^-?[0-9]*\\.[0-9]+$")))
   {
      tokens_.push_back(TToken(std::stod(tk)));
   }
   // Word
   else if  (std::regex_match(tk, std::regex("^[a-zA-Z_]+[0-9]*$")))
   {
      if       (tk == LCAT::id)
         tokens_.push_back(LCAT());
      else if  (tk == SCAT::id)
         tokens_.push_back(SCAT());
      else if  (tk == OBJ::id)
         tokens_.push_back(OBJ());
      else
         tokens_.push_back(TToken(tk));
   }
   else
      return false;

   return true;
}

//-----------------------------------------------------------------------------------------
static bool AddStringToken(std::list<TToken>& tokens_, std::string::const_iterator begin_, std::string::const_iterator end_)
{
   if (begin_ == end_)
      return true;

   std::string tk(begin_, end_);

   tokens_.push_back(TToken(tk));

   return true;
}

//-----------------------------------------------------------------------------------------
static bool AddServiceToken(std::list<TToken>& tokens_, std::string::const_iterator begin_, std::string::const_iterator end_)
{
   if (begin_ == end_)
      return true;

   std::string tk(begin_, end_);

   if       (tk == BEGIN_DOUBLE_ARROW::id)
      tokens_.push_back(BEGIN_DOUBLE_ARROW());
   else if  (tk == END_DOUBLE_ARROW::id)
      tokens_.push_back(END_DOUBLE_ARROW());
   else if  (tk == BEGIN_SINGLE_ARROW::id)
      tokens_.push_back(BEGIN_SINGLE_ARROW());
   else if  (tk == END_SINGLE_ARROW::id)
      tokens_.push_back(END_SINGLE_ARROW());
   else
      return false;

   return true;
}

//-----------------------------------------------------------------------------------------
enum class EParseState
{
   eInitial,
   eToken,
   eTokenString,
   eService
};

//-----------------------------------------------------------------------------------------
std::list<TToken> Tokenizer::Process(const std::string& string_)
{
   SMNode*  crtSMNode   {};
   SMNode   root;
   build_seq_tree(root);

   std::list<TToken> tokens;

   auto begin = string_.begin();
   auto end   = begin;

   EParseState state { EParseState::eInitial };

   while (true)
   {
      switch (state)
      {
         case EParseState::eInitial:
         {
            if       (end == string_.end())
            {
               return tokens;
            }
            else if  (IsToken<TokenGroup::eDelimeter>(*end))
            {
               AddSpecialToken<TokenGroup::eDelimeter>(tokens, *end);
            }
            else if  (IsToken<TokenGroup::eKey>(*end))
            {
               AddSpecialToken<TokenGroup::eKey>(tokens, *end);

               if (*end == QUOTE::id)
                  state = EParseState::eTokenString;
            }
            else if  (find_symbol_node(root.child, *end))
            {
               state = EParseState::eService;
               crtSMNode = &root;
               continue;
            }
            else if  (IsToken<TokenGroup::eSkip>(*end))
            {
            }
            else
            {
               state = EParseState::eToken;
               continue;
            }

            begin = ++end;
         }
         break;

         case EParseState::eToken:
         {
            if (end == string_.end())
            {
               if (!AddToken(tokens, begin, end))
                  return std::list<TToken>();
               return tokens;
            }
            else if (IsToken<TokenGroup::eDelimeter>(*end))
            {
               if (!AddToken(tokens, begin, end))
                  return std::list<TToken>();
               AddSpecialToken<TokenGroup::eDelimeter>(tokens, *end);

               state = EParseState::eInitial;
               begin = ++end;
            }
            else if (IsToken<TokenGroup::eKey>(*end))
            {
               if (!AddToken(tokens, begin, end))
                  return std::list<TToken>();
               AddSpecialToken<TokenGroup::eKey>(tokens, *end);

               state = EParseState::eInitial;
               begin = ++end;
            }
            else if (IsToken<TokenGroup::eSkip>(*end))
            {
               if (!AddToken(tokens, begin, end))
                  return std::list<TToken>();

               state = EParseState::eInitial;
               begin = ++end;
            }
            else if (find_symbol_node(root.child, *end))
            {
               if (!AddToken(tokens, begin, end))
                  return std::list<TToken>();

               state = EParseState::eInitial;
               begin = end;
            }
            else
               ++end;
         }
         break;

         case EParseState::eTokenString:
         {
            if (*end == QUOTE::id)
            {
               if (!AddStringToken(tokens, begin, end))
                  return std::list<TToken>();

               AddSpecialToken<TokenGroup::eKey>(tokens, *end);

               begin = ++end;

               state = EParseState::eInitial;
            }
            else
               ++end;

         }
         break;

         case EParseState::eService:
         {
            SMNode* pNext = find_symbol_node(crtSMNode->child, *end);

            if (!pNext)
            {
               if (!AddServiceToken(tokens, begin, end))
                  return std::list<TToken>();

               begin = end;

               crtSMNode = &root;

               state = EParseState::eInitial;
            }
            else
            {
               crtSMNode = pNext;
               ++end;
            }
         }
         break;
      }
   }

   return tokens;
}

//-----------------------------------------------------------------------------------------
std::string Tokenizer::TokenLog(const TToken& tk_, bool append_value_)
{
   if       (std::holds_alternative<int         >(tk_))
      return "INT"      + (append_value_ ? "(" + std::to_string(  std::get<int         >(tk_))  + ")" : "");

   else if  (std::holds_alternative<float       >(tk_))
      return "FLOAT"    + (append_value_ ? "(" + std::to_string(  std::get<float       >(tk_))  + ")" : "");

   else if  (std::holds_alternative<double      >(tk_))
      return "DOUBLE"   + (append_value_ ? "(" + std::to_string(  std::get<double      >(tk_))  + ")" : "");

   else if  (std::holds_alternative<std::string >(tk_))
      return "STRING"   + (append_value_ ? "(" +                  std::get<std::string >(tk_)   + ")" : "");

   else if  (std::holds_alternative<LCAT>(tk_))
      return std::get<LCAT>(tk_).id;
   else if  (std::holds_alternative<SCAT>(tk_))
      return std::get<SCAT>(tk_).id;
   else if  (std::holds_alternative<OBJ>(tk_))
      return std::get<OBJ>(tk_).id;
   else if  (std::holds_alternative<BEGIN_DOUBLE_ARROW>(tk_))
      return std::get<BEGIN_DOUBLE_ARROW>(tk_).id;
   else if  (std::holds_alternative<END_DOUBLE_ARROW>(tk_))
      return std::get<END_DOUBLE_ARROW>(tk_).id;
   else if  (std::holds_alternative<BEGIN_SINGLE_ARROW>(tk_))
      return std::get<BEGIN_SINGLE_ARROW>(tk_).id;
   else if  (std::holds_alternative<END_SINGLE_ARROW>(tk_))
      return std::get<END_SINGLE_ARROW>(tk_).id;
   else if  (std::holds_alternative<QUOTE>(tk_))
      return std::string(1, std::get<QUOTE>(tk_).id);
   else if  (std::holds_alternative<COMMA>(tk_))
      return std::string(1, std::get<COMMA>(tk_).id);
   else if  (std::holds_alternative<BEGIN_BR>(tk_))
      return std::string(1, std::get<BEGIN_BR>(tk_).id);
   else if  (std::holds_alternative<END_BR>(tk_))
      return std::string(1, std::get<END_BR>(tk_).id);
   else if  (std::holds_alternative<SEMICOLON>(tk_))
      return std::string(1, std::get<SEMICOLON>(tk_).id);
   else if  (std::holds_alternative<COMMENT>(tk_))
      return std::get<COMMENT>(tk_).id;
   else if  (std::holds_alternative<COLON>(tk_))
      return std::string(1, std::get<COLON>(tk_).id);
   else if  (std::holds_alternative<ASTERISK>(tk_))
      return std::string(1, std::get<ASTERISK>(tk_).id);
   else if  (std::holds_alternative<SPACE>(tk_))
      return std::string(1, std::get<SPACE>(tk_).id);
   else if  (std::holds_alternative<TAB>(tk_))
      return std::string(1, std::get<TAB>(tk_).id);
   else if  (std::holds_alternative<NEXT_LINE>(tk_))
      return std::string(1, std::get<NEXT_LINE>(tk_).id);

   return "";
}
