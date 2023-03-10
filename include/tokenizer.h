#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <list>
#include <variant>

#include "str_utils.h"

namespace cat
{
   // Keywords
   struct LCAT          {
      static const constexpr char* const id   = "LCAT";
   };
   struct SCAT          {
      static const constexpr char* const id   = "SCAT";
   };
   struct OBJ           {
      static const constexpr char* const id   = "OBJ";
   };

   // Service symbols
   struct DOUBLE_ARROW  {
      static const constexpr char* const id   = "=>";
   };
   struct SINGLE_ARROW  {
      static const constexpr char* const id   = "->";
   };

   // Misc
   struct COMMENT       {
      static const constexpr char* const id   = "--";
   };

   // Key symbols
   struct ASTERISK      {
      static const constexpr char id   = '*';
   };
   struct QUOTE         {
      static const constexpr char id   = '"';
   };

   // Skip symbols
   struct SPACE         {
      static const constexpr char id   = ' ';
   };
   struct TAB           {
      static const constexpr char id   = '\t';
   };
   struct NEXT_LINE     {
      static const constexpr char id   = '\n';
   };

   // Delimeters
   struct COMMA         {
      static const constexpr char id   = ',';
   };
   struct BEGIN_BR      {
      static const constexpr char id   = '{';
   };
   struct END_BR        {
      static const constexpr char id   = '}';
   };
   struct SEMICOLON     {
      static const constexpr char id   = ';';
   };
   struct COLON         {
      static const constexpr char id   = ':';
   };

   using TServiceT    = std::tuple<DOUBLE_ARROW, SINGLE_ARROW>;
   using TDelimeterT  = std::tuple<COMMA, BEGIN_BR, END_BR, SEMICOLON, COLON>;
   using TKeyT        = std::tuple<ASTERISK, QUOTE>;
   using TSkipT       = std::tuple<SPACE, TAB, NEXT_LINE>;

   using TToken = std::variant<
         // Keyword types
         LCAT, SCAT, OBJ,
         // Misc
         COMMENT,
         // Service types
         DOUBLE_ARROW, SINGLE_ARROW,
         // Delimeter types
         COMMA, BEGIN_BR, END_BR, SEMICOLON, COLON,
         // Key types
         ASTERISK, QUOTE,
         // Skip types
         SPACE, TAB, NEXT_LINE,
         // Basic types
         int, float, double, std::string
      >;

   class Tokenizer
   {
   public:
      static std::list<TToken> Process(const std::string& string_);
      static std::string TokenLog(const TToken& tk_, bool append_value_ = false);
   };

}

#endif // TOKENIZER_H
