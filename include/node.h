#pragma once

#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "arrow.h"
#include "cat_export.h"
#include "log.h"
#include "tokenizer.h"

namespace cat {
class Node;

enum class ESetTypes : unsigned char { eDouble = 0, eFloat, eInt, eString };
using TSetValue = std::variant<double, float, int, std::string>;
using FunctionName = std::string;
using Function = std::pair<FunctionName, TSetValue>;

/**
 * @brief The Node class
 */
class CAT_EXPORT Node {
public:
  Node(const Node &) = default;
  Node(Node &&) = default;
  ~Node() = default;

  Node &operator=(Node &&) = default;
  Node &operator=(const Node &) = default;

  using Set = std::set<Node>;
  using Map = std::map<Node, Set>;
  using Vec = std::vector<Node>;
  using List = std::list<Node>;
  using PairSet = std::pair<Node, Set>;
  using NName = std::string;

  enum class EType : unsigned char {
    eSet // Set
    ,
    eObject // Object
    ,
    eSCategory // Small category
    ,
    eLCategory // Large category
    ,
    eUndefined
  };

  /**
   * @brief Converts type to string
   * @return Type name
   */
  static std::string Type2Name(EType type_);

  /**
   * @brief Converts enum type to string
   * @return Type
   */
  static std::string Type2Str(EType type_);

  /**
   * @brief Converts string to enum type
   * @return Type
   */
  static EType Str2Type(const std::string &type_);

  bool operator<(const Node &cat_) const;
  bool operator==(const Node &cat_) const;
  bool operator!=(const Node &cat_) const;

  /**
   * @brief Node constructor
   * @param name_ - node name
   */
  explicit Node(const NName &name_, EType type_);

  /**
   * @brief Adds arrow
   * @param arrow_ - arrow
   * @return True if successful
   */
  bool AddArrow(const Arrow &arrow_);

  /**
   * @brief Adds arrow
   * @param args_ - arrow arguments
   * @return True if successful
   */
  template <typename... Args> bool EmplaceArrow(Args &&...args_) {
    return AddArrow(Arrow(std::forward<Args>(args_)...));
  }

  /**
   * @brief Adds arrows
   * @param arrows_ - arrows
   * @return True if successful
   */
  bool AddArrows(const Arrow::Vec &arrows_);

  /**
   * @brief Erases arrow
   * @param arrow_ - arrow name
   * @return True if successfull
   */
  bool EraseArrow(const Arrow::AName &name_);

  /**
   * @brief Erases all arrows
   */
  void EraseArrows();

  /**
   * @brief Checks whether the node contains any arrows
   * @return True if there are no arrows
   */
  bool IsArrowsEmpty() const;

  /**
   * @brief Counts the number of arrows
   * @return Number of arrows
   */
  size_t CountArrows() const;

  /**
   * @brief Adds node
   * @param node_ - node
   * @return True if successful
   */
  bool AddNode(const Node &node_);

  /**
   * @brief Adds node
   * @param args - node arguments
   * @return True if successful
   */
  template <typename... Args> bool EmplaceNode(Args &&...args_) {
    return AddNode(Node(std::forward<Args>(args_)...));
  }

  /**
   * @brief Adds nodes
   * @param nodes_ - nodes
   * @return True if successful
   */
  bool AddNodes(const Vec &nodes_);

  /**
   * @brief Erases node
   * @param node_ - node name
   * @return True if successfull
   */
  bool EraseNode(const NName &node_);

  /**
   * @brief Replaces node
   * @param node_ - node
   */
  void ReplaceNode(const Node &node_);

  /**
   * @brief Erases all nodes
   */
  void EraseNodes();

  /**
   * @brief Checks whether the node contains any nodes
   * @return True if there are no nodes
   */
  bool IsNodesEmpty() const;

  /**
   * @brief Counts the number of nodes
   * @return Number of nodes
   */
  size_t CountNodes() const;

  /**
   * @brief Clones node
   * @param old_ - name of the node to clone
   * @param new_ - new node name
   */
  void CloneNode(const NName &old_, const NName &new_);

  /**
   * @brief Queries for arrows
   * @param query_ - query
   * @param matchCount_ - match count limit
   * @return Arrows
   */
  Arrow::List QueryArrows(
      const std::string &query_,
      std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

  /**
   * @brief Queries for nodes
   * @param query_ - query
   * @return Nodes
   */
  Node::List QueryNodes(const std::string &query_) const;

  /**
   * @brief Queries for patterns
   * @param query_ - query
   * @param matchCount_ - match count limit
   * @return Pattern
   */
  Node Query(const std::string &query_,
             std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

  /**
   * @brief Verifying arrow
   * @param arrow_ - arrow
   * @return True if successful
   */
  bool Verify(const Arrow &arrow_) const;

  /**
   * @brief Sets node name
   * @param name_ - name
   */
  void SetName(const NName &name_);

  /**
   * @brief Returns node name
   * @return Name
   */
  const NName &Name() const;

  /**
   * @brief Creates compositions
   */
  void SolveCompositions();

  /**
   * @brief Finds initial nodes. All arrow compositions
   * must be resolved before calling this method i.e. call "SolveCompositions"
   * first
   * @return Initial nodes
   */
  Node::List Initial() const;

  /**
   * @brief Finds terminal nodes. All arrow compositions
   * must be resolved before calling this method i.e. call "SolveCompositions"
   * first
   * @return Terminal nodes
   */
  Node::List Terminal() const;

  /**
   * @brief Finds any sequence of nodes between two given nodes
   * @param from_ - source node of the sequence
   * @param to_ - target node of the sequence
   * @param length_ - match for length
   * @return Sequence of nodes
   */
  std::list<Node::NName>
  SolveSequence(const Node::NName &from_, const Node::NName &to_,
                std::optional<size_t> length_ = std::optional<size_t>()) const;

  /**
   * @brief Finds all sequences of nodes between two given nodes
   * @param from_ - source node of the sequences
   * @param to_ - target node of the sequences
   * @param length_ - match for length
   * @return Sequences of nodes
   */
  std::list<std::list<Node::NName>>
  SolveSequences(const Node::NName &from_, const Node::NName &to_,
                 std::optional<size_t> length_ = std::optional<size_t>()) const;

  /**
   * @brief Maps sequence of nodes onto sequence of arrows
   * @param nodes_ - sequence of nodes
   * @return Sequence of arrows
   */
  Arrow::List MapNodes2Arrows(const std::list<Node::NName> &nodes_) const;

  /**
   * @brief Inverses arrows
   */
  void Inverse();

  /**
   * @brief Creates a set of all possible arrows
   * @param from_ - source node for arrows
   * @param to_ - target node for arrows
   * @return Set of arrows corresponding to different mappings between nodes
   */
  Arrow::List ProposeArrows(const Node::NName &from_, const Node::NName &to_);

  /**
   * @brief Solves determination problem
   * @param AB - arrow from A to B
   * @param AC - arrow from A to C
   * @return Arrow from B to C
   */
  Arrow::List SolveDetermination(const Arrow::AName &AB,
                                 const Arrow::AName &AC);

  /**
   * @brief Returns node type
   * @return Node type
   */
  EType Type() const;

  /**
   * @brief Returns type of internal nodes
   * @return Node type
   */
  Node::EType InternalNode() const;

  /**
   * @brief Sets value
   */
  void SetValue(const TSetValue &value_);

  /**
   * @brief Returns value
   * @return Value
   */
  const TSetValue &GetValue() const;

private:
  /**
   * @brief Node structure validation
   * @return True if valid
   */
  bool validate_node_data() const;

  /**
   * @brief Expression (Reverse Polish Notation) evaluation
   * @param tks_ - sequence of tokens
   * @return Evaluated value
   */
  Node::List evaluateRPN(const std::list<TToken> &tks_) const;

  Map m_nodes;
  Arrow::List m_arrows;
  NName m_name;
  EType m_type;
  TSetValue m_value;
};

struct CAT_EXPORT NodeKeyHasher {
  std::size_t operator()(const Node &n_) const;
};

} // namespace cat
