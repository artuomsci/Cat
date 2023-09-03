#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <list>
#include <string>
#include <variant>

#include "cat_export.h"

namespace cat {
// Keywords
struct LCAT {
  static const constexpr char *const id = "LCAT";
};
struct SCAT {
  static const constexpr char *const id = "SCAT";
};
struct OBJ {
  static const constexpr char *const id = "OBJ";
};

struct BEGIN_SINGLE_ARROW {
  static const constexpr char *const id = "-[";
};
struct END_SINGLE_ARROW {
  static const constexpr char *const id = "]->";
};
struct EQ {
  static const constexpr char *const id = "==";
};
struct NEQ {
  static const constexpr char *const id = "!=";
};

// Misc
struct COMMENT {
  static const constexpr char *const id = "--";
};

// Key symbols
struct ASTERISK {
  static const constexpr char id = '*';
};
struct QUOTE {
  static const constexpr char id = '"';
};
struct OR {
  static const constexpr char id = '|';
};
struct AND {
  static const constexpr char id = '&';
};
struct NEG {
  static const constexpr char id = '~';
};

// Skip symbols
struct SPACE {
  static const constexpr char id = ' ';
};
struct TAB {
  static const constexpr char id = '\t';
};
struct NEXT_LINE {
  static const constexpr char id = '\n';
};

// Delimeters
struct COMMA {
  static const constexpr char id = ',';
};
struct BEGIN_CBR {
  static const constexpr char id = '{';
};
struct END_CBR {
  static const constexpr char id = '}';
};
struct BEGIN_BR {
  static const constexpr char id = '(';
};
struct END_BR {
  static const constexpr char id = ')';
};
struct SEMICOLON {
  static const constexpr char id = ';';
};
struct COLON {
  static const constexpr char id = ':';
};

using TServiceT = std::tuple<BEGIN_SINGLE_ARROW, END_SINGLE_ARROW, EQ, NEQ>;
using TDelimeterT =
    std::tuple<COMMA, BEGIN_CBR, END_CBR, BEGIN_BR, END_BR, SEMICOLON, COLON>;
using TKeyT = std::tuple<ASTERISK, QUOTE, OR, AND, NEG>;
using TSkipT = std::tuple<SPACE, TAB, NEXT_LINE>;

using TToken = std::variant<
    // Keyword types
    LCAT, SCAT, OBJ,
    // Misc
    COMMENT,
    // Service types
    BEGIN_SINGLE_ARROW, END_SINGLE_ARROW, EQ, NEQ,
    // Delimeter types
    COMMA, BEGIN_CBR, END_CBR, BEGIN_BR, END_BR, SEMICOLON, COLON,
    // Key types
    ASTERISK, QUOTE, OR, AND, NEG,
    // Skip types
    SPACE, TAB, NEXT_LINE,
    // Basic types
    int, float, double, std::string>;

class CAT_EXPORT Tokenizer {
public:
  static std::list<TToken> Process(const std::string &string_);
  static std::string TokenLog(const TToken &tk_, bool append_value_ = false);
  static int Token2Precedence(const TToken &tk_);
  static bool IsOperand(const TToken &tk_);
  static std::list<TToken> Expr2RPN(const std::list<TToken> &expr_);
};

} // namespace cat

#endif // TOKENIZER_H
