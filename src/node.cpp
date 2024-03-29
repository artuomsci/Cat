#include "node.h"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stack>

#include "parser.h"
#include "register.h"

using namespace cat;

// Keywords

// entity names
static const char *sNObject = "object";
static const char *sNSCategory = "category";
static const char *sNLCategory = "large category";

//-----------------------------------------------------------------------------------------
std::string Node::Type2Name(Node::EType type_) {
  if (type_ == EType::eObject)
    return sNObject;
  else if (type_ == EType::eSCategory)
    return sNSCategory;
  else if (type_ == EType::eLCategory)
    return sNLCategory;
  else
    return "";
}

//-----------------------------------------------------------------------------------------
std::string Node::Type2Str(EType type_) {
  if (type_ == EType::eObject)
    return "eObject";
  else if (type_ == EType::eSCategory)
    return "eSCategory";
  else if (type_ == EType::eLCategory)
    return "eLCategory";
  else if (type_ == EType::eUndefined)
    return "eUndefined";

  return "";
}

//-----------------------------------------------------------------------------------------
Node::EType Node::Str2Type(const std::string &type_) {
  if (type_ == "eObject")
    return EType::eObject;
  else if (type_ == "eSCategory")
    return EType::eSCategory;
  else if (type_ == "eLCategory")
    return EType::eLCategory;
  else if (type_ == "eUndefined")
    return EType::eUndefined;

  return EType::eUndefined;
}

//-----------------------------------------------------------------------------------------
bool Node::operator<(const Node &cat_) const { return m_name < cat_.m_name; }

//-----------------------------------------------------------------------------------------
bool Node::operator==(const Node &cat_) const {
  return m_name == cat_.m_name && m_type == cat_.m_type;
}

//-----------------------------------------------------------------------------------------
bool Node::operator!=(const Node &cat_) const {
  return m_name != cat_.m_name || m_type != cat_.m_type;
}

//-----------------------------------------------------------------------------------------
Node::Node(const NName &name_, EType type_) : m_name(name_), m_type(type_) {
  if (type_ == EType::eUndefined)
    throw std::runtime_error("EType::eUndefined is not allowed as a node type");
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrow(const Arrow &arrow_) {
  if (!QueryArrows(arrow_.AsQuery()).empty()) {
    print_error("Arrow redefinition: " + arrow_.Name());
    return false;
  }

  for (const Arrow &arrow : m_arrows) {
    if (arrow_.Name() == arrow.Name() && arrow_.Target() != arrow.Target()) {
      print_error("Arrow redefinition: " + arrow_.Name());
      return false;
    }
  }

  if (!Verify(arrow_))
    return false;

  auto it = m_nodes.find(Node(arrow_.Target(), InternalNode()));

  const auto &[target, _] = *it;

  m_nodes.at(Node(arrow_.Source(), InternalNode())).insert(target);

  m_arrows.push_back(arrow_);

  return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrows(const Arrow::Vec &arrows_) {
  for (const Arrow &arrow : arrows_) {
    if (!AddArrow(arrow))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseArrow(const Arrow::AName &name_) {
  for (Arrow &arrow : m_arrows) {
    if (arrow.Name() == name_) {
      if (arrow.Source() == arrow.Target()) {
        print_error("Deleting identity arrow " + name_);
        return false;
      }

      auto it_node = m_nodes.find(Node(arrow.Source(), InternalNode()));
      if (it_node != m_nodes.end()) {
        if (arrow.Source() == arrow.Target())
          return false;

        auto &[_, node_set] = *it_node;

        node_set.erase(Node(arrow.Target(), InternalNode()));
      }
    }
  }

  auto it_delete = std::find_if(m_arrows.begin(), m_arrows.end(),
                                [&](const Arrow::Vec::value_type &element_) {
                                  return element_.Name() == name_;
                                });

  if (it_delete == m_arrows.end())
    return false;

  m_arrows.erase(it_delete);

  return true;
}

//-----------------------------------------------------------------------------------------
void Node::EraseArrows() {
  for (auto it = m_arrows.begin(); it != m_arrows.end();) {
    if (it->Source() != it->Target())
      it = m_arrows.erase(it);
    else
      ++it;
  }

  for (auto &[node, nodeset] : m_nodes) {
    nodeset.clear();
    nodeset.insert(node);
  }
}

//-----------------------------------------------------------------------------------------
bool Node::IsArrowsEmpty() const { return m_arrows.empty(); }

//-----------------------------------------------------------------------------------------
size_t Node::CountArrows() const { return m_arrows.size(); }

//-----------------------------------------------------------------------------------------
bool Node::AddNode(const Node &node_) {
  if (node_.Name().empty())
    return false;

  if (m_nodes.find(node_) == m_nodes.end()) {
    m_nodes[node_];

    Arrow func(node_, node_, Arrow::IdArrowName(node_.Name()));

    for (const auto &id : node_.QueryNodes("*"))
      func.AddArrow(Arrow(id, id));

    if (!AddArrow(func))
      return false;
  } else {
    print_error("Redefinition of " + Node::Type2Name(node_.Type()) + ": " +
                node_.Name());
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddNodes(const Vec &nodes_) {
  for (const Node &node : nodes_) {
    if (!AddNode(node))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseNode(const NName &node_) {
  NName name_copy = node_;

  auto it = m_nodes.find(Node(name_copy, InternalNode()));
  if (it != m_nodes.end()) {
    m_nodes.erase(it);

    for (auto &[_, codomain] : m_nodes)
      codomain.erase(Node(name_copy, InternalNode()));

    auto it_end = std::remove_if(m_arrows.begin(), m_arrows.end(),
                                 [&](const Arrow::Vec::value_type &element_) {
                                   return element_.Source() == name_copy ||
                                          element_.Target() == name_copy;
                                 });

    m_arrows.erase(it_end, m_arrows.end());

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------------------
void Node::ReplaceNode(const Node &node_) {
  auto it = m_nodes.find(node_);
  if (it != m_nodes.end()) {
    auto codomain = it->second;

    codomain.erase(node_);

    codomain.insert(node_);

    m_nodes.erase(it);

    m_nodes[node_] = codomain;
  } else
    AddNode(node_);
}

//-----------------------------------------------------------------------------------------
void Node::EraseNodes() {
  m_nodes.clear();
  m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
bool Node::IsNodesEmpty() const { return m_nodes.empty(); }

//-----------------------------------------------------------------------------------------
size_t Node::CountNodes() const { return m_nodes.size(); }

//-----------------------------------------------------------------------------------------
void Node::CloneNode(const NName &old_, const NName &new_) {
  Node::List nodes = QueryNodes(old_);
  if (nodes.size() != 1)
    return;

  Node node = nodes.front();
  node.SetName(new_);

  AddNode(node);

  std::string sAny(1, ASTERISK::id);

  // outward
  {
    Arrow::List arrows = QueryArrows(Arrow(old_, sAny, sAny).AsQuery());

    for (Arrow &arrow : arrows) {
      if (arrow.Source() == arrow.Target())
        continue;

      arrow.SetSource(new_);

      AddArrow(arrow);
    }
  }

  // inward
  {
    Arrow::List arrows = QueryArrows(Arrow(sAny, old_, sAny).AsQuery());

    for (Arrow &arrow : arrows) {
      if (arrow.Source() == arrow.Target())
        continue;

      arrow.SetTarget(new_);

      AddArrow(arrow);
    }
  }
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::QueryArrows(const std::string &query_,
                              std::optional<size_t> matchCount_) const {
  return Parser::QueryArrows(query_, m_arrows, matchCount_);
}

//-----------------------------------------------------------------------------------------
Node::List Node::evaluateRPN(const std::list<TToken> &tks_) const {
  // Evaluation stack
  std::list<Node::List> stack;

  for (auto tk = tks_.begin(); tk != tks_.end(); ++tk) {
    if (Tokenizer::IsOperand(*tk)) {
      std::string name;

      if (std::holds_alternative<std::string>(*tk)) {
        name = std::get<std::string>(*tk);
      } else if (std::holds_alternative<int>(*tk)) {
        name = std::to_string(std::get<int>(*tk));
      }

      // Empty container evaluates to False
      stack.push_back(Node::List());

      // "Resolving" tokens
      auto it = m_nodes.find(Node(name, InternalNode()));
      if (it != m_nodes.end())
        // Not empty container evaluates to True
        stack.back().push_back(it->first);
    } else if (std::holds_alternative<AND>(*tk)) {
      Node::List &right = *(stack.rbegin());
      Node::List &left = *(++stack.rbegin());

      // Summarizing results by AND
      if (!left.empty() && !right.empty()) {
        left.insert(left.end(), right.begin(), right.end());
        stack.pop_back();
      } else {
        stack.pop_back();
        stack.back().clear();
      }
    } else if (std::holds_alternative<OR>(*tk)) {
      Node::List &right = *(stack.rbegin());
      Node::List &left = *(++stack.rbegin());

      // Summarizing results by OR
      left.insert(left.end(), right.begin(), right.end());

      stack.pop_back();
    } else if (std::holds_alternative<NEG>(*tk)) {
      std::vector<std::string> exclude_names;

      if (stack.back().empty()) {
        --tk;

        if (std::holds_alternative<std::string>(*tk)) {
          exclude_names = {std::get<std::string>(*tk)};
        } else if (std::holds_alternative<int>(*tk)) {
          exclude_names = {std::to_string(std::get<int>(*tk))};
        }

        ++tk;
      } else {
        for (const auto &exclude : stack.back())
          exclude_names.push_back(exclude.Name());
      }

      stack.back().clear();

      // Adding everyting except indicated nodes
      for (const auto &[key, _] : m_nodes) {
        bool isExclude{};

        for (const auto &exclude : exclude_names) {
          if (key.Name() == exclude)
            isExclude = true;
        }

        if (!isExclude)
          stack.back().push_back(key);
      }
    }
  }

  if (!stack.empty()) {
    // Removing duplicates
    std::set<Node> tmp(stack.front().begin(), stack.front().end());

    return Node::List(tmp.begin(), tmp.end());
  }

  return Node::List();
}

//-----------------------------------------------------------------------------------------
Node::List Node::QueryNodes(const std::string &query_) const {
  std::list<TToken> tks = Tokenizer::Process(query_);

  Node::List ret;

  if (tks.size() == 1 && std::holds_alternative<ASTERISK>(tks.front())) {
    for (const auto &[node, _] : m_nodes) {
      ret.push_back(node);
    }
  } else
    return evaluateRPN(Tokenizer::Expr2RPN(tks));

  return ret;
}

//-----------------------------------------------------------------------------------------
Node Node::Query(const std::string &query_,
                 std::optional<size_t> matchCount_) const {
  if (matchCount_ && matchCount_ == 0)
    return Node("", Node::EType::eUndefined);

  auto qarrow = Parser::GetArrows(query_, Node::List(), Node::List(), false);
  if (qarrow.size() != 1)
    return Node("", Node::EType::eUndefined);

  Node ret(m_name, m_type);
  size_t counter{};

  const auto &source = qarrow[0].Source();
  const auto &target = qarrow[0].Target();
  const auto &name = qarrow[0].Name();

  std::string sAny(1, ASTERISK::id);

  bool name_check = name != sAny;

  if (source == sAny && target == sAny) {
    if (!name_check && !matchCount_)
      return *this;

    for (const Arrow &arrow : m_arrows) {
      if (name_check && arrow.Name() != name)
        continue;

      if (ret.QueryNodes(arrow.Source()).empty()) {
        auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
        ret.AddNode(it->first);
      }

      if (ret.QueryNodes(arrow.Target()).empty()) {
        auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
        ret.AddNode(it->first);
      }

      ret.AddArrow(arrow);

      if (matchCount_ && ++counter == matchCount_)
        break;
    }
  } else if (source != sAny && target == sAny) {
    for (const Arrow &arrow : m_arrows) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Source() == source) {
        if (ret.QueryNodes(arrow.Source()).empty()) {
          auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
          ret.AddNode(it->first);
        }

        if (ret.QueryNodes(arrow.Target()).empty()) {
          auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
          ret.AddNode(it->first);
        }

        ret.AddArrow(arrow);

        if (matchCount_ && ++counter == matchCount_)
          break;
      }
    }
  } else if (source == sAny && target != sAny) {
    for (const auto &arrow : m_arrows) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Target() == target) {
        if (ret.QueryNodes(arrow.Source()).empty()) {
          auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
          ret.AddNode(it->first);
        }

        if (ret.QueryNodes(arrow.Target()).empty()) {
          auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
          ret.AddNode(it->first);
        }

        ret.AddArrow(arrow);

        if (matchCount_ && ++counter == matchCount_)
          break;
      }
    }
  } else if (source != sAny && target != sAny) {
    for (const auto &arrow : m_arrows) {
      if (name_check && arrow.Name() != name)
        continue;

      if (arrow.Source() == source && arrow.Target() == target) {
        if (ret.QueryNodes(arrow.Source()).empty()) {
          auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
          ret.AddNode(it->first);
        }

        if (ret.QueryNodes(arrow.Target()).empty()) {
          auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
          ret.AddNode(it->first);
        }

        ret.AddArrow(arrow);

        if (matchCount_ && ++counter == matchCount_)
          break;
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::Verify(const Arrow &arrow_) const {
  auto source_node = Node(arrow_.Source(), InternalNode());
  auto target_node = Node(arrow_.Target(), InternalNode());

  auto itSourceCat = m_nodes.find(source_node);
  auto itTargetCat = m_nodes.find(target_node);

  if (itSourceCat == m_nodes.end()) {
    print_error("No such source " + Node::Type2Name(source_node.Type()) + ": " +
                arrow_.Source());
    return false;
  }

  if (itTargetCat == m_nodes.end()) {
    print_error("No such target " + Node::Type2Name(target_node.Type()) + ": " +
                arrow_.Target());
    return false;
  }

  if (Type() == Node::EType::eSet || Type() == Node::EType::eObject)
    return true;

  const auto &[source_cat, _s] = *itSourceCat;
  const auto &[target_cat, _t] = *itTargetCat;

  using TSource2Arrow = std::set<std::pair<Node::NName, Arrow::AName>>;
  TSource2Arrow visited;

  for (const Arrow &arrow : arrow_.QueryArrows(Arrow("*", "*").AsQuery())) {
    auto head = TSource2Arrow::value_type(arrow.Source(), arrow.Name());

    auto itv = visited.find(head);
    if (itv != visited.end()) {
      auto msg = "Arrow: " + arrow_.Name() + " : ";
      msg += "Mapping the same source " + arrow.Source() +
             " multiple times with arrow " + arrow.Name();
      print_error(msg);
      return false;
    }

    visited.insert(head);

    if (InternalNode() != EType::eObject) {
      if (source_cat.QueryNodes(arrow.Source()).empty()) {
        print_error("Missing source for " + arrow.Source() + " to " +
                    arrow.Target());
        return false;
      }
    }
  }

  std::string sAny(1, ASTERISK::id);

  // Checking mapping
  for (const auto &obj : source_cat.QueryNodes(sAny)) {
    if (!arrow_.SingleMap(obj)) {
      print_error("Failure to map " + Node::Type2Name(obj.Type()) + ": " +
                  obj.Name());
      return false;
    }
  }

  for (const Arrow &arrow : source_cat.QueryArrows(Arrow("*", "*").AsQuery())) {
    auto mapped_source = Node(arrow.Source(), source_cat.InternalNode());
    auto mapped_target = Node(arrow.Target(), source_cat.InternalNode());

    auto objs = arrow_.SingleMap(mapped_source);
    auto objt = arrow_.SingleMap(mapped_target);

    if (!objs) {
      print_error("Failure to map " + Node::Type2Name(mapped_source.Type()) +
                  " " + mapped_source.Name());
      return false;
    }

    if (source_cat.QueryNodes(arrow.Source()).empty()) {
      print_error("No such " + Node::Type2Name(mapped_source.Type()) + " " +
                  mapped_source.Name() + " in " +
                  Node::Type2Name(source_cat.Type()) + " " + source_cat.Name());
      return false;
    }

    if (target_cat.QueryNodes(objs->Name()).empty()) {
      print_error("No such " + Node::Type2Name(objs->Type()) + " " +
                  objs->Name() + " in " + Node::Type2Name(target_cat.Type()) +
                  " " + target_cat.Name());
      return false;
    }

    if (!objt) {
      print_error("Failure to map " + Node::Type2Name(mapped_target.Type()) +
                  " " + mapped_target.Name());
      return false;
    }

    if (source_cat.QueryNodes(arrow.Target()).empty()) {
      print_error("No such " + Node::Type2Name(mapped_target.Type()) + " " +
                  mapped_target.Name() + " in " +
                  Node::Type2Name(source_cat.Type()) + " " + source_cat.Name());
      return false;
    }

    if (target_cat.QueryNodes(objt->Name()).empty()) {
      print_error("No such " + Node::Type2Name(objt->Type()) + " " +
                  objt->Name() + " in " + Node::Type2Name(target_cat.Type()) +
                  " " + target_cat.Name());
      return false;
    }

    // Checking mapping of arrows
    if (target_cat.QueryArrows(Arrow(objs->Name(), objt->Name(), "*").AsQuery())
            .empty()) {
      print_error("Failure to match morphism: " + objs->Name() + " to " +
                  objt->Name());
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------------------
void Node::SetName(const NName &name_) { m_name = name_; }

//-----------------------------------------------------------------------------------------
const Node::NName &Node::Name() const { return m_name; }

//-----------------------------------------------------------------------------------------
void Node::SolveCompositions() {
  Arrow::List initial_arrows = QueryArrows(Arrow("*", "*").AsQuery());

  for (const Arrow &init_arrow : initial_arrows) {
    if (init_arrow.Source() == init_arrow.Target()) {
      continue;
    }

    Arrow::List compositions;

    Arrow::List traverse =
        QueryArrows(Arrow(init_arrow.Target(), "*").AsQuery());

    Arrow::List new_codomain;
    while (!traverse.empty()) {
      new_codomain.insert(new_codomain.end(), traverse.begin(), traverse.end());

      Arrow::List new_traverse;
      for (const Arrow &arrow : traverse) {
        if (arrow.Source() == arrow.Target()) {
          continue;
        }

        Arrow composition(init_arrow.Source(), arrow.Target(),
                          init_arrow.Source() + init_arrow.Target() +
                              arrow.Target());

        for (const Arrow &internal_arrow :
             init_arrow.QueryArrows(Arrow("*", "*").AsQuery())) {
          auto internal_target = arrow.SingleMap(internal_arrow.Target());

          composition.EmplaceArrow(internal_arrow.Source(),
                                   internal_target->Name());
        }

        compositions.push_back(composition);
      }

      traverse = new_traverse;
    }

    for (const auto &composition : compositions) {
      AddArrow(composition);
    }
  }
}

//-----------------------------------------------------------------------------------------
Node::List Node::Initial() const {
  Node::List ret;

  for (const auto &[domain, codomain] : m_nodes) {
    if (m_nodes.size() == codomain.size())
      ret.push_back(domain);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Node::List Node::Terminal() const {
  Node::List ret;

  for (const auto &[domain, _] : m_nodes) {
    bool is_terminal{true};

    for (const auto &[domain_int, codomain_int] : m_nodes) {
      if (std::find_if(codomain_int.begin(), codomain_int.end(),
                       [&](const Node &node_) { return domain == node_; }) ==
          codomain_int.end()) {
        is_terminal = false;
        break;
      }
    }

    if (is_terminal)
      ret.push_back(domain);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
std::list<Node::NName>
Node::SolveSequence(const Node::NName &from_, const Node::NName &to_,
                    std::optional<size_t> length_) const {
  std::list<Node::NName> ret;

  std::list<Node::PairSet> stack;

  std::optional<Node::NName> current_node(from_);

  while (true) {
    // Checking for destination
    if (current_node.value() == to_) {
      bool pass = !length_ || (length_ && length_ == stack.size() + 1);

      if (pass) {
        for (auto &[nodei, _] : stack)
          ret.push_back(nodei.Name());

        ret.push_back(current_node.value());

        return ret;
      }
    }

    stack.emplace_back(Node(current_node.value(), InternalNode()),
                       m_nodes.at(Node(current_node.value(), InternalNode())));

    // Remove identity morphism
    stack.back().second.erase(Node(current_node.value(), InternalNode()));

    current_node.reset();

    while (!current_node.has_value()) {
      // Trying new set of nodes
      Node::Set &forward_codomain = stack.back().second;

      if (forward_codomain.empty()) {
        stack.pop_back();

        if (stack.empty())
          return ret;

        continue;
      }

      // Moving one node forward
      current_node.emplace(
          forward_codomain.extract(forward_codomain.begin()).value().Name());

      // Checking for loops
      for (const auto &[node, _] : stack) {
        // Is already visited
        if (node.Name() == current_node.value()) {
          current_node.reset();
          break;
        }
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
std::list<std::list<Node::NName>>
Node::SolveSequences(const Node::NName &from_, const Node::NName &to_,
                     std::optional<size_t> length_) const {
  std::list<std::list<Node::NName>> ret;

  std::list<Node::PairSet> stack;

  std::optional<Node::NName> current_node(from_);

  while (true) {
    // Checking for destination
    if (current_node.value() == to_) {
      std::list<Node::NName> seq;

      bool pass = !length_ || (length_ && length_ == stack.size() + 1);

      if (pass) {
        for (auto &[nodei, _] : stack)
          seq.push_back(nodei.Name());

        seq.push_back(current_node.value());

        ret.push_back(seq);
      }
    } else {
      // Stacking forward movements
      stack.emplace_back(
          Node(current_node.value(), InternalNode()),
          m_nodes.at(Node(current_node.value(), InternalNode())));

      // Removing identity morphism
      stack.back().second.erase(Node(current_node.value(), InternalNode()));
    }

    current_node.reset();

    while (!current_node.has_value()) {
      // Trying new sets of nodes
      Node::Set &forward_codomain = stack.back().second;

      if (forward_codomain.empty()) {
        stack.pop_back();

        if (stack.empty())
          return ret;

        continue;
      }

      // Moving one node forward
      current_node.emplace(
          forward_codomain.extract(forward_codomain.begin()).value().Name());

      // Checking for loops
      for (const auto &[node, _] : stack) {
        // Is already visited
        if (node.Name() == current_node.value()) {
          current_node.reset();
          break;
        }
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::MapNodes2Arrows(const std::list<Node::NName> &nodes_) const {
  Arrow::List ret;

  auto it_last = std::prev(nodes_.end());

  for (auto itn = nodes_.begin(); itn != it_last; ++itn) {
    auto it = std::find_if(m_arrows.begin(), m_arrows.end(),
                           [&](const Arrow::List::value_type &elem_) {
                             return *itn == elem_.Source() &&
                                    *std::next(itn) == elem_.Target();
                           });

    if (it != m_arrows.end())
      ret.push_back(*it);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
void Node::Inverse() {
  Arrow::List tmp = m_arrows;

  EraseArrows();

  for (Arrow &arrow : tmp) {
    arrow.Inverse();
    AddArrow(arrow);
  }
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::ProposeArrows(const Node::NName &from_,
                                const Node::NName &to_) {
  Node::List source_list = QueryNodes(from_);
  if (source_list.size() != 1)
    return Arrow::List();
  const Node &source = source_list.front();

  Node::List target_list = QueryNodes(to_);
  if (target_list.size() != 1)
    return Arrow::List();
  const Node &target = target_list.front();

  std::list<std::list<std::pair<Node, Node>>> projections;

  Node::List source_nodes = source.QueryNodes("*");
  Node::List target_nodes = target.QueryNodes("*");

  projections.emplace_back(std::list<std::pair<Node, Node>>());

  for (const Node &src_node : source_nodes) {
    std::list<std::pair<Node, Node>> options;
    for (const Node &trg_node : target_nodes) {
      options.emplace_back(src_node, trg_node);
    }

    std::list<std::list<std::pair<Node, Node>>> update;

    for (auto &option : options) {
      for (auto &projection : projections) {
        update.emplace_back(std::list<std::pair<Node, Node>>());
        update.back().insert(update.back().begin(), projection.begin(),
                             projection.end());
        update.back().push_back(option);
      }
    }

    projections = std::move(update);
  }

  Arrow::List ret;
  for (const auto &projection : projections) {
    Arrow arrow(source, target);

    for (const auto &[from, to] : projection) {
      arrow.AddArrow(Arrow(from.Name(), to.Name()));
    }

    ret.push_back(std::move(arrow));
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::SolveDetermination(const Arrow::AName &AB,
                                     const Arrow::AName &AC) {

  Arrow::List ABlist = QueryArrows(Arrow("*", "*", AB).AsQuery());
  if (ABlist.empty()) {
    return {};
  }

  Arrow &abArrow = ABlist.front();

  Arrow::List AClist = QueryArrows(Arrow("*", "*", AC).AsQuery());
  if (AClist.empty()) {
    return {};
  }

  Arrow &acArrow = AClist.front();

  Arrow::List bcList = ProposeArrows(abArrow.Target(), acArrow.Target());

  Arrow::List ret;
  for (const auto &BC : bcList) {
    auto detComposeArrow = BC.Compose(abArrow);
    if (detComposeArrow.has_value()) {

      if (detComposeArrow->IsAssociative(acArrow)) {
        ret.push_back(BC);
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::SolveChoice(const Arrow::AName &BC, const Arrow::AName &AC) {

  Arrow::List BClist = QueryArrows(Arrow("*", "*", BC).AsQuery());
  if (BClist.empty()) {
    return {};
  }

  Arrow &bcArrow = BClist.front();

  Arrow::List AClist = QueryArrows(Arrow("*", "*", AC).AsQuery());
  if (AClist.empty()) {
    return {};
  }

  Arrow &acArrow = AClist.front();

  Arrow::List abList = ProposeArrows(acArrow.Source(), bcArrow.Source());

  Arrow::List ret;
  for (const auto &AB : abList) {
    auto choiceComposeArrow = bcArrow.Compose(AB);
    if (choiceComposeArrow.has_value()) {

      if (choiceComposeArrow->IsAssociative(acArrow)) {
        ret.push_back(AB);
      }
    }
  }

  return ret;
}

//-----------------------------------------------------------------------------------------
Node::EType Node::Type() const { return m_type; }

//-----------------------------------------------------------------------------------------
Node::EType Node::InternalNode() const {
  if (m_type == EType::eLCategory)
    return EType::eSCategory;
  else if (m_type == EType::eSCategory)
    return EType::eObject;
  else if (m_type == EType::eObject)
    return EType::eSet;
  else
    return EType::eUndefined;
}

//-----------------------------------------------------------------------------------------
void Node::SetValue(const TSetValue &value_) { m_value = value_; }

//-----------------------------------------------------------------------------------------
const TSetValue &Node::GetValue() const { return m_value; }

//-----------------------------------------------------------------------------------------
bool Node::validate_node_data() const {
  size_t sz{};
  for (const auto &[_, codomain] : m_nodes)
    sz += codomain.size();

  return sz == m_arrows.size();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node &k_) const {
  return std::hash<std::string>{}(k_.Name());
}
