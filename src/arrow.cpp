#include "arrow.h"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stack>

#include "node.h"
#include "parser.h"
#include "register.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string &source_, const std::string &target_,
             const std::string &arrow_name_)
    : m_source(source_), m_target(target_), m_name(arrow_name_){};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string &source_, const std::string &target_)
    : m_source(source_), m_target(target_),
      m_name(DefaultArrowName(source_, target_)){};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const Node &source_, const Node &target_,
             const std::string &arrow_name_)
    : m_source(source_.Name()), m_target(target_.Name()), m_name(arrow_name_) {}

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const Node &source_, const Node &target_)
    : Arrow(source_, target_,
            DefaultArrowName(source_.Name(), target_.Name())) {}

//-----------------------------------------------------------------------------------------
bool Arrow::operator<(const Arrow &arrow_) const {
  return std::tie(m_source, m_target, m_name) <
         std::tie(arrow_.m_source, arrow_.m_target, arrow_.m_name);
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator==(const Arrow &arrow_) const {
  return m_source == arrow_.m_source && m_target == arrow_.m_target &&
         m_name == arrow_.m_name && m_arrows == arrow_.m_arrows;
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator!=(const Arrow &arrow_) const {
  return m_source != arrow_.m_source || m_target != arrow_.m_target ||
         m_name != arrow_.m_name || m_arrows != arrow_.m_arrows;
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::Map(const std::optional<Node> &node_) const {
  if (!node_.has_value() || node_->Name() != m_source) {
    return {};
  }

  Node ret(m_target, node_->Type());

  const Register::TFn &fn = Register::Inst().Get(*this);
  ret.SetValue(fn(node_->GetValue()));

  // Mapping of nodes
  for (const auto &node : node_->QueryNodes("*")) {
    auto mapped = SingleMap(node);
    if (!mapped.has_value()) {
      return {};
    }

    if (ret.QueryNodes(mapped.value().Name()).empty())
      ret.AddNode(mapped.value());
  }

  // Mapping of arrows
  for (const Arrow &arrow : node_->QueryArrows(Arrow("*", "*").AsQuery())) {
    auto source = SingleMap(Node(arrow.m_source, Node::EType::eObject));
    auto target = SingleMap(Node(arrow.m_target, Node::EType::eObject));

    Arrow mapped_arrow(*source, *target);

    Arrow::List internalArrows = arrow.QueryArrows(Arrow("*", "*").AsQuery());
    for (auto &it : internalArrows) {
      mapped_arrow.AddArrow(it);
    }

    if (ret.QueryArrows(mapped_arrow.AsQuery()).empty())
      ret.AddArrow(mapped_arrow);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::DefaultArrowName(const std::string &source_,
                                    const std::string &target_) {
  std::string sAny(1, ASTERISK::id);

  if (source_ == sAny || target_ == sAny)
    return sAny;
  else
    return source_ + "_" + target_;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::IdArrowName(const std::string &name_) {
  return DefaultArrowName(name_, name_);
}

//-----------------------------------------------------------------------------------------
void Arrow::SetDefaultName() { m_name = DefaultArrowName(m_source, m_target); }

//-----------------------------------------------------------------------------------------
const std::string &Arrow::Source() const { return m_source; }

//-----------------------------------------------------------------------------------------
void Arrow::SetSource(const std::string &source_) {
  if (DefaultArrowName(m_source, m_target) == m_name) {
    m_source = source_;
    m_name = DefaultArrowName(m_source, m_target);
  } else
    m_source = source_;
}

//-----------------------------------------------------------------------------------------
const std::string &Arrow::Target() const { return m_target; }

//-----------------------------------------------------------------------------------------
void Arrow::SetTarget(const std::string &target_) {
  if (DefaultArrowName(m_source, m_target) == m_name) {
    m_target = target_;
    m_name = DefaultArrowName(m_source, m_target);
  } else
    m_target = target_;
}

//-----------------------------------------------------------------------------------------
const Arrow::AName &Arrow::Name() const { return m_name; }

//-----------------------------------------------------------------------------------------
void Arrow::AddArrow(const Arrow &arrow_) { m_arrows.push_back(arrow_); }

//-----------------------------------------------------------------------------------------
void Arrow::EraseArrow(const Arrow::AName &arrow_) {
  auto it = std::find_if(m_arrows.begin(), m_arrows.end(),
                         [&](const List::value_type &element_) {
                           return element_.Name() == arrow_;
                         });

  if (it != m_arrows.end())
    m_arrows.erase(it);
}

//-----------------------------------------------------------------------------------------
void Arrow::EraseArrows() { m_arrows.clear(); }

//-----------------------------------------------------------------------------------------
Arrow::List Arrow::QueryArrows(const std::string &query_,
                               std::optional<size_t> matchCount_) const {
  return Parser::QueryArrows(query_, m_arrows, matchCount_);
}

//-----------------------------------------------------------------------------------------
bool Arrow::IsEmpty() const { return m_arrows.empty(); }

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::singleMapImpl(const std::string &name_) const {
  for (const Arrow &arrow : m_arrows) {
    if (arrow.Source() == name_) {
      return Node(arrow.Target(), Node::EType::eObject);
    }
  }

  return {};
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::SingleMap(const std::optional<Node> &node_) const {
  return node_ ? singleMapImpl(node_->Name()) : std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::SingleMap(const std::string &name_) const {
  return singleMapImpl(name_);
}

//-----------------------------------------------------------------------------------------
void Arrow::Inverse() {
  m_name = DefaultArrowName(m_source, m_target) == m_name
               ? DefaultArrowName(m_target, m_source)
               : m_name;

  std::swap(m_source, m_target);

  for (auto &arrow : m_arrows)
    arrow.Inverse();
}

//-----------------------------------------------------------------------------------------
bool Arrow::IsInvertible() const {
  for (auto it = m_arrows.begin(); it != m_arrows.end(); ++it) {
    for (auto it_match = it; it_match != m_arrows.end(); ++it_match) {
      if (it == it_match) {
        continue;
      }

      if (it->Target() == it_match->Target()) {
        return false;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::AsQuery() const {
  return m_source + BEGIN_SINGLE_ARROW::id + m_name + END_SINGLE_ARROW::id +
         m_target + BEGIN_CBR::id + END_CBR::id + SEMICOLON::id;
}

//-----------------------------------------------------------------------------------------
bool Arrow::IsAssociative(const Arrow &arrow) const {
  if (arrow.Source() != Source() || arrow.Target() != Target())
    return false;

  if (arrow.CountArrows() != CountArrows())
    return false;

  for (auto &right : arrow.QueryArrows(Arrow("*", "*").AsQuery())) {
    auto result =
        QueryArrows(Arrow(right.Source(), right.Target(), "*").AsQuery());
    if (result.empty())
      return false;

    bool isFound{};
    for (auto &left : result) {
      if (left.IsAssociative(right)) {
        isFound = true;
        break;
      }
    }

    if (!isFound)
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------------------
size_t Arrow::CountArrows() const { return m_arrows.size(); }

//-----------------------------------------------------------------------------------------
bool Arrow::IsValid() const {
  for (auto it = m_arrows.begin(); it != m_arrows.end(); ++it) {
    for (auto it_match = it; it_match != m_arrows.end(); ++it_match) {
      if (it == it_match) {
        continue;
      }

      if (it->Source() == it_match->Source()) {
        return false;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
std::optional<Arrow> Arrow::Compose(const Arrow &arrow_) const {

  Node slv("slv", Node::EType::eLCategory);

  Node A(arrow_.Source(), Node::EType::eSCategory);
  Node B(arrow_.Target(), Node::EType::eSCategory);
  for (const auto &arrow : arrow_.QueryArrows(Arrow("*", "*").AsQuery())) {
    A.AddNode(Node(arrow.Source(), Node::EType::eSCategory));
    B.AddNode(Node(arrow.Target(), Node::EType::eSCategory));
  }

  slv.AddNode(A);
  slv.AddNode(B);

  Node C(Target(), Node::EType::eSCategory);
  for (const auto &arrow : QueryArrows(Arrow("*", "*").AsQuery())) {
    C.AddNode(Node(arrow.Target(), Node::EType::eSCategory));
  }

  slv.AddNode(C);

  slv.AddArrow(*this);
  slv.AddArrow(arrow_);

  slv.SolveCompositions();

  Arrow::List ret =
      slv.QueryArrows(Arrow(arrow_.Source(), Target(), "*").AsQuery());

  return ret.empty() ? std::optional<Arrow>() : ret.front();
}
