#include "parser.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string.h>

#include "log.h"

using namespace cat;

static const char *comment_begin = "/*";
static const char *comment_end = "*/";

//-----------------------------------------------------------------------------------------
bool Parser::parse_statement(TKIt &it_, TKIt end_, NodePtr pNode_) const {
  if (it_ == end_) {
    return false;
  }

  while (true) {
    // Small category declaration
    if (std::holds_alternative<SCAT>(*it_)) {
      if (pNode_->Type() != Node::EType::eLCategory) {
        print_error("Incorrect SCAT declaration");
        return false;
      }

      if (!parse_CAT(it_, end_, cat::Node::EType::eSCategory, pNode_)) {
        return false;
      }
    }

    // Object declaration
    if (std::holds_alternative<OBJ>(*it_)) {
      if (pNode_->Type() != Node::EType::eSCategory) {
        print_error("Incorrect OBJ declaration");
        return false;
      }

      if (!parse_OBJ(it_, end_, pNode_)) {
        return false;
      }
    }

    // Word declaration
    if (std::holds_alternative<std::string>(*it_) ||
        std::holds_alternative<ASTERISK>(*it_)) {
      if (!parse_arrow(it_, end_, pNode_)) {
        print_error("Incorrect arrow declaration");
        return false;
      }
    }

    // End of statement
    if (std::holds_alternative<END_CBR>(*it_)) {
      ++it_;

      if (std::holds_alternative<SEMICOLON>(*it_)) {
        ++it_;
        break;
      } else {
        break;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_CAT(TKIt &it_, TKIt end_, cat::Node::EType type_,
                       NodePtr &pNode_) const {
  ++it_;

  if (!std::holds_alternative<std::string>(*it_)) {
    print_error("Incorrect CAT declaration, name expected");
    return false;
  }

  std::string name = std::get<std::string>(*it_);

  ++it_;

  if (std::holds_alternative<SEMICOLON>(*it_)) {
    if (pNode_)
      return pNode_->AddNode(Node(name, type_));

    pNode_.reset(new Node(name, type_));
    return true;
  }

  if (!std::holds_alternative<BEGIN_CBR>(*it_)) {
    print_error("Incorrect CAT declaration");
    return false;
  }

  NodePtr pNewNode(new Node(name, type_));

  if (!parse_statement(++it_, end_, pNewNode))
    return false;

  if (!pNode_)
    pNode_ = pNewNode;
  else
    pNode_->AddNode(*pNewNode);

  return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_OBJ(TKIt &it_, TKIt end_, NodePtr pNode_) const {
  while (true) {
    ++it_;

    std::string name;

    if (std::holds_alternative<std::string>(*it_)) {
      name = std::get<std::string>(*it_);
    } else if (std::holds_alternative<int>(*it_)) {
      name = std::to_string(std::get<int>(*it_));
    } else {
      print_error("Incorrect OBJ declaration, name expected");
      return false;
    }

    Node obj(name, Node::EType::eObject);

    pNode_->AddNode(obj);

    ++it_;

    if (std::holds_alternative<SEMICOLON>(*it_)) {
      ++it_;
      break;
    } else if (!std::holds_alternative<COMMA>(*it_)) {
      print_error("Incorrect OBJ declaration");
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::parse_arrow(TKIt &it_, TKIt end_, NodePtr pNode_) const {
  if (!pNode_)
    return false;

  Arrow::List arrows;
  if (!ParseArrow(it_, end_, arrows))
    return false;

  std::string any(1, ASTERISK::id);

  for (auto &arrow : arrows) {
    if (arrow.Name() == any) {
      arrow.SetDefaultName();
    }
  }

  for (const auto &arrow : arrows) {
    if (!pNode_->AddArrow(arrow))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Parser::ParseArrow(TKIt &it_, TKIt end_, Arrow::List &arrows_) {
  if (it_ == end_) {
    return true;
  }

  if (std::holds_alternative<END_CBR>(*it_)) {
    return true;
  }

  while (true) {
    if (it_ == end_)
      return true;

    if (std::holds_alternative<END_CBR>(*it_)) {
      if (++it_ == end_) {
        break;
      }

      if (std::holds_alternative<SEMICOLON>(*it_)) {
        ++it_;
        break;
      } else {
        break;
      }

      break;
    }

    std::string source;

    if (std::holds_alternative<ASTERISK>(*it_)) {
      source = ASTERISK::id;
    } else if (std::holds_alternative<std::string>(*it_)) {
      source = std::get<std::string>(*it_);
    } else if (std::holds_alternative<int>(*it_)) {
      source = std::to_string(std::get<int>(*it_));
    } else {
      print_error("Incorrect arrow declaration");
      return false;
    }

    if (++it_ == end_)
      return false;

    if (++it_ == end_)
      return false;

    TToken name_tk = *it_;

    if (++it_ == end_)
      return false;

    if (++it_ == end_)
      return false;

    std::string target;

    if (std::holds_alternative<ASTERISK>(*it_)) {
      target = ASTERISK::id;
    } else if (std::holds_alternative<std::string>(*it_)) {
      target = std::get<std::string>(*it_);
    } else if (std::holds_alternative<int>(*it_)) {
      target = std::to_string(std::get<int>(*it_));
    } else {
      print_error("Incorrect arrow declaration");
      return false;
    }

    std::string name;

    if (std::holds_alternative<std::string>(name_tk)) {
      name = std::get<std::string>(name_tk);
    } else if (std::holds_alternative<int>(name_tk)) {
      name = std::to_string(std::get<int>(name_tk));
    } else if (std::holds_alternative<ASTERISK>(name_tk)) {
      name = ASTERISK::id;
    } else {
      print_error("Incorrect arrow declaration. Name expected.");
      return false;
    }

    Arrow arrow(source, target, name);

    if (++it_ == end_) {
      return false;
    }

    if (std::holds_alternative<BEGIN_SINGLE_ARROW>(*it_)) {
      arrows_.push_back(arrow);
      --it_;
    } else if (std::holds_alternative<BEGIN_CBR>(*it_)) {
      if (++it_ == end_) {
        return false;
      }

      while (it_ != end_) {
        if (std::holds_alternative<END_CBR>(*it_)) {
          break;
        }

        Arrow::List arrows;
        if (!ParseArrow(it_, end_, arrows)) {
          return false;
        }

        for (const auto &it : arrows) {
          arrow.AddArrow(it);
        }

        if (std::holds_alternative<END_CBR>(*it_)) {
          break;
        }
      }

      arrows_.push_back(arrow);
    } else {
      print_error("Incorrect arrow declaration");
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
static std::vector<Arrow> resolve_arrows(const std::string &name_,
                                         const std::string &source_,
                                         const std::string &target_,
                                         const Node::List &domain_,
                                         const Node::List &codomain_) {
  std::vector<Arrow> ret;

  auto fnCheckSource = [&]() {
    auto it_d =
        std::find_if(domain_.begin(), domain_.end(), [&](const Node &element_) {
          return element_.Name() == source_;
        });

    if (it_d == domain_.end()) {
      print_error("No such source: " + source_);
      return false;
    }
    return true;
  };

  auto fnCheckTarget = [&]() {
    auto it_c = std::find_if(
        codomain_.begin(), codomain_.end(),
        [&](const Node &element_) { return element_.Name() == target_; });

    if (it_c == codomain_.end()) {
      print_error("No such target: " + target_);
      return false;
    }

    return true;
  };

  std::string sAny(1, ASTERISK::id);

  // f :: a -> b
  if (name_ != sAny && source_ != sAny && target_ != sAny) {
    if (!fnCheckSource())
      return ret;
    if (!fnCheckTarget())
      return ret;

    ret.push_back(Arrow(source_, target_, name_));
  }
  // * :: a -> b
  else if (name_ == sAny && source_ != sAny && target_ != sAny) {
    if (!fnCheckSource())
      return ret;
    if (!fnCheckTarget())
      return ret;

    ret.push_back(Arrow(source_, target_));
  }
  // * :: * -> *
  else if (name_ == sAny && source_ == sAny && target_ == sAny) {
    for (const Node &dnode : domain_) {
      for (const auto &cnode : codomain_) {
        ret.push_back(Arrow(dnode, cnode));
      }
    }
  }
  // * :: * -> b
  else if (name_ == sAny && source_ == sAny && target_ != sAny) {
    if (!fnCheckTarget())
      return ret;

    for (const auto &dnode : domain_)
      ret.push_back(Arrow(dnode.Name(), target_));
  }
  // * :: a -> *
  else if (name_ == sAny && source_ != sAny && target_ == sAny) {
    if (!fnCheckSource())
      return ret;

    for (const auto &cnode : codomain_)
      ret.push_back(Arrow(source_, cnode.Name()));
  }
  // f :: a -> *
  else if (name_ != sAny && source_ != sAny && target_ == sAny) {
    print_error("Incorrect definition");
  }
  // f :: * -> b
  else if (name_ != sAny && source_ == sAny && target_ != sAny) {
    if (!fnCheckTarget())
      return ret;

    for (const auto &dnode : domain_)
      ret.push_back(Arrow(dnode.Name(), target_, name_));
  }
  // f :: * -> *
  else if (name_ != sAny && source_ == sAny && target_ == sAny) {
    print_error("Incorrect definition");
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
std::vector<Arrow> Parser::GetArrows(const std::string &line_,
                                     const Node::List &domain_,
                                     const Node::List &codomain_,
                                     bool resolve_) {
  std::list<TToken> tks = Tokenizer::Process(line_);

  Arrow::List arrows;
  auto start_tk = tks.begin();
  Parser::ParseArrow(start_tk, tks.end(), arrows);

  std::vector<Arrow> ret;
  ret.reserve(arrows.size());
  for (const auto &arrow : arrows) {
    if (resolve_) {
      for (const auto &it : resolve_arrows(arrow.Name(), arrow.Source(),
                                           arrow.Target(), domain_, codomain_))
        ret.push_back(it);
    } else
      ret.push_back(arrow);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Parser::QueryArrows(const std::string &query_,
                                const Arrow::List &arrows_,
                                std::optional<size_t> matchCount_) {
  if (matchCount_ && matchCount_ == 0)
    return Arrow::List();

  auto qarrow = Parser::GetArrows(query_, Node::List(), Node::List(), false);
  if (qarrow.size() != 1)
    return Arrow::List();

  const auto &source = qarrow[0].Source();
  const auto &target = qarrow[0].Target();
  const auto &name = qarrow[0].Name();

  Arrow::List ret;

  std::string sAny(1, ASTERISK::id);

  bool name_check = name != sAny;

  if (source == sAny && target == sAny) {
    if (!name_check && !matchCount_)
      return arrows_;

    for (const auto &arrow : arrows_) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Name() == name) {
        ret.push_back(arrow);

        if (matchCount_ && ret.size() == matchCount_)
          break;
      }
    }
  } else if (source != sAny && target == sAny) {
    for (const auto &arrow : arrows_) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Source() == source) {
        ret.push_back(arrow);

        if (matchCount_ && ret.size() == matchCount_)
          break;
      }
    }
  } else if (source == sAny && target != sAny) {
    for (const auto &arrow : arrows_) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Target() == target) {
        ret.push_back(arrow);

        if (matchCount_ && ret.size() == matchCount_)
          break;
      }
    }
  } else if (source != sAny && target != sAny) {
    for (const auto &arrow : arrows_) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Source() == source && arrow.Target() == target) {
        ret.push_back(arrow);

        if (matchCount_ && ret.size() == matchCount_)
          break;
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
static std::optional<std::string> remove_comments(std::string string_) {
  size_t initial = 0;
  while (true) {
    auto begin = string_.find(comment_begin, initial);
    auto new_end = string_.find(comment_end, initial);

    if (new_end < begin) {
      print_error("Comment's section is not opened.");
      return std::optional<std::string>();
    }

    if (begin != std::string::npos) {
      initial = begin;

      auto end = string_.find(comment_end, begin);
      auto new_start = string_.find(comment_begin, begin + 1);

      if (end == std::string::npos || (new_start < end)) {
        print_error("Comment's section is not closed.");
        return std::optional<std::string>();
      }

      string_.erase(begin, end - begin + strlen(comment_end));
    } else
      break;
  }

  return string_;
}

//-----------------------------------------------------------------------------------------
bool Parser::Parse(const std::string &filename_) {
  std::ifstream file(filename_);
  if (!file.is_open()) {
    print_error("Error opening file");
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return ParseSource(buffer.str());
}

//-----------------------------------------------------------------------------------------
bool Parser::ParseSource(const std::string &src_) {
  auto prep = remove_comments(src_);
  if (!prep)
    return false;

  std::list<TToken> tokens = Tokenizer::Process(prep.value());

  TKIt it = tokens.begin();
  while (it != tokens.end()) {
    if (std::holds_alternative<LCAT>(*it)) {
      if (!parse_CAT(it, tokens.end(), cat::Node::EType::eLCategory, m_pNode))
        return false;
    } else if (std::holds_alternative<SCAT>(*it)) {
      if (!parse_CAT(it, tokens.end(), cat::Node::EType::eSCategory, m_pNode))
        return false;
    } else if (std::holds_alternative<OBJ>(*it)) {
      if (!parse_CAT(it, tokens.end(), cat::Node::EType::eObject, m_pNode))
        return false;
    } else if (std::holds_alternative<std::string>(*it) ||
               std::holds_alternative<ASTERISK>(*it)) {
      if (!parse_arrow(it, tokens.end(), m_pNode)) {
        print_error("Incorrect arrow declaration");
        return false;
      }
    } else if (std::holds_alternative<SEMICOLON>(*it)) {
      ++it;
    } else {
      return false;
    }
  }
}

//-----------------------------------------------------------------------------------------
std::shared_ptr<Node> Parser::Data() const { return m_pNode; };
