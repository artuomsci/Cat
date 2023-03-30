#pragma once

#include <map>
#include <vector>
#include <set>
#include <optional>
#include <string>
#include <list>
#include <variant>

#include "cat_export.h"

#include "log.h"

namespace cat
{
   class Node;

   enum class ESetTypes : unsigned char
   {
         eDouble = 0
      ,  eFloat
      ,  eInt
      ,  eString
   };
   using TSetValue      = std::variant<double, float, int, std::string>;
   using FunctionName   = std::string;
   using Function       = std::pair<FunctionName, TSetValue>;

   /**
    * @brief The Arrow class represents morphisms and functors
    */
   class CAT_EXPORT Arrow
   {
   public:

      enum class EType : unsigned char {
            eMorphism   // Morphism
         ,  eFunctor    // Functor
         ,  eFunction   // Function
         ,  eUndefined
      };

      /**
      * @brief Converts type to string
      * @return Type name
      */
      static std::string Type2Name(EType type_);

      Arrow(EType type_, const std::string& source_, const std::string& target_, const std::string& arrow_name_);
      Arrow(EType type_, const std::string& source_, const std::string& target_);

      Arrow(const Node& source_, const Node& target_, const std::string& arrow_name_);
      Arrow(const Node& source_, const Node& target_);

      bool operator <  (const Arrow& arrow_) const;
      bool operator == (const Arrow& arrow_) const;
      bool operator != (const Arrow& arrow_) const;

      Arrow(const Arrow&) = default;
      Arrow(Arrow&&) = default;

      Arrow& operator = (Arrow&&) = default;
      Arrow& operator = (const Arrow&) = default;

      std::optional<Node> operator()(const std::optional<Node>& node_) const;

      using Vec   = std::vector<Arrow>;
      using List  = std::list<Arrow>;
      using AName = std::string;

      /**
       * @brief Returns default arrow name
       * @param source_ - source name
       * @param target_ - target name
       * @return Arrow name
       */
      static std::string DefaultArrowName(const std::string& source_, const std::string& target_);

      /**
       * @brief Returns identity arrow name
       * @param name_ - source/target name
       * @return Arrow name
       */
      static std::string IdArrowName(const std::string& name_);

      /**
       * @brief Returns arrow source name
       * @return Name
       */
      const std::string& Source() const;

      /**
       * @brief Sets source name
       * @param source_ - source name
       */
      void SetSource(const std::string& source_);

      /**
       * @brief Returns arrow target name
       * @return Name
       */
      const std::string& Target() const;

      /**
       * @brief Sets target name
       * @param target_ - target name
       */
      void SetTarget(const std::string& target_);

      /**
       * @brief Returns arrow name
       * @return Name
       */
      const AName& Name() const;

      /**
       * @brief Adds arrow
       * @param arrow_ - arrow
       */
      void AddArrow(const Arrow& arrow_);

      /**
       * @brief Erases arrow
       * @param arrow_ - arrow name
       */
      void EraseArrow(const Arrow::AName& arrow_);

      /**
       * @brief Erases all arrows
       */
      void EraseArrows();

      /**
       * @brief Queries for arrows
       * Syntax: source name -> target name
       * Syntax for named arrows: source name -[ arrow name ]-> target name
       * Use "*" as a replacement for any name
       * Query examples: x -> y, * -> y, x -> *, * -> *
       * Query examples: x -[ * ]-> y, * -[ operation ]-> * ... etc
       * @param query_ - query
       * @return Arrows
       */
      List QueryArrows(const std::string& query_, std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

      /**
       * @brief Checks whether the arrow contains any arrows
       * @return True if there are no arrows
       */
      bool IsEmpty() const;

      /**
      * @brief Maps node with arrow
      * @param node_ - node for mapping
      * @return Mapped node
      */
      std::optional<Node> SingleMap(const std::optional<Node>& node_) const;

      /**
      * @brief Maps node with arrow
      * @param name_ - node name for mapping
      * @return Mapped node
      */
      std::optional<Node> SingleMap(const std::string& name_) const;

      /**
       * @brief Inverses arrow
       */
      void Inverse();

      /**
       * @brief Returns arrow type
       * @return Arrow type
       */
      Arrow::EType Type() const;

      /**
       * @brief Returns arrow as query
       * @return Query representation
       */
      std::string AsQuery() const;

   private:

      std::string m_source;
      std::string m_target;
      AName       m_name;
      List        m_arrows;
      EType       m_type;
   };

   /**
    * @brief The Node class
    */
   class CAT_EXPORT Node
   {
   public:

      Node(const Node&) = default;
      Node(Node&&) = default;
      ~Node() = default;

      Node& operator = (Node&&) = default;
      Node& operator = (const Node&) = default;

      using Set      = std::set<Node>;
      using Map      = std::map<Node, Set>;
      using Vec      = std::vector<Node>;
      using List     = std::list<Node>;
      using PairSet  = std::pair<Node, Set>;
      using NName    = std::string;

      enum class EType : unsigned char {
            eSet        // Set
         ,  eObject     // Object
         ,  eSCategory  // Small category
         ,  eLCategory  // Large category
         ,  eUndefined
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
      static EType Str2Type(const std::string& type_);

      bool operator < (const Node& cat_) const;
      bool operator ==(const Node& cat_) const;
      bool operator !=(const Node& cat_) const;

      /**
      * @brief Node constructor
      * @param name_ - node name
      */
      explicit Node(const NName& name_, EType type_);

      /**
       * @brief Adds arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool AddArrow(const Arrow& arrow_);

      /**
       * @brief Adds arrows
       * @param arrows_ - arrows
       * @return True if successful
       */
      bool AddArrows(const Arrow::Vec& arrows_);

      /**
       * @brief Erases arrow
       * @param arrow_ - arrow name
       * @return True if successfull
       */
      bool EraseArrow(const Arrow::AName& name_);

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
      bool AddNode(const Node& node_);

      /**
       * @brief Adds nodes
       * @param nodes_ - nodes
       * @return True if successful
       */
      bool AddNodes(const Vec& nodes_);

      /**
       * @brief Erases node
       * @param node_ - node name
       * @return True if successfull
       */
      bool EraseNode(const NName& node_);

      /**
       * @brief Replaces node
       * @param node_ - node
       */
      void ReplaceNode(const Node& node_);

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
      void CloneNode(const NName& old_, const NName& new_);

      /**
       * @brief Queries for arrows
       * @brief Syntax: "arrow name :: source name -> target name"
       * @brief Use "*" as a replacement for any name
       * @brief Query examples: "f0 :: x -> y", "* :: x -> y", "* :: * -> y", etc...
       * @param query_ - query
       * @param matchCount_ - match count limit
       * @return Arrows
       */
      Arrow::List QueryArrows(const std::string& query_, std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

      /**
       * @brief Queries for nodes
       * @brief Use "*" as a replacement for any name
       * @brief Use "|" as logical operator OR
       * @brief Query example: "x | y", "x"
       * @param query_ - query
       * @return Nodes
       */
      Node::List QueryNodes(const std::string& query_) const;

      /**
       * @brief Queries for patterns
       * @param query_ - query
       * @param matchCount_ - match count limit
       * @return Pattern
       */
      Node Query(const std::string& query_, std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

      /**
       * @brief Verifying arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool Verify(const Arrow& arrow_) const;

      /**
       * @brief Sets node name
       * @param name_ - name
       */
      void SetName(const NName& name_);

      /**
       * @brief Returns node name
       * @return Name
       */
      const NName& Name() const;

      /**
       * @brief Creates compositions
       */
      void SolveCompositions();

      /**
       * @brief Finds initial nodes. All arrow compositions
       * must be resolved before calling this method i.e. call "SolveCompositions" first
       * @return Initial nodes
       */
      Node::List Initial() const;

      /**
       * @brief Finds terminal nodes. All arrow compositions
       * must be resolved before calling this method i.e. call "SolveCompositions" first
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
      std::list<Node::NName> SolveSequence(const Node::NName& from_, const Node::NName& to_, std::optional<size_t> length_ = std::optional<size_t>()) const;

      /**
       * @brief Finds all sequences of nodes between two given nodes
       * @param from_ - source node of the sequences
       * @param to_ - target node of the sequences
       * @param length_ - match for length
       * @return Sequences of nodes
       */
      std::list<std::list<Node::NName>> SolveSequences(const Node::NName& from_, const Node::NName& to_, std::optional<size_t> length_ = std::optional<size_t>()) const;

      /**
       * @brief Maps sequence of nodes onto sequence of arrows
       * @param nodes_ - sequence of nodes
       * @return Sequence of arrows
       */
      Arrow::List MapNodes2Arrows(const std::list<Node::NName>& nodes_) const;

      /**
       * @brief Inverses arrows
       */
      void Inverse();

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
       * @brief Returns type of internal arrows
       * @return Arrow type
       */
      Arrow::EType InternalArrow() const;

      /**
       * @brief Returns type of external arrows
       * @return Arrow type
       */
      Arrow::EType ExternalArrow() const;

      /**
       * @brief Sets value
       */
      void SetValue(const TSetValue& value_);

      /**
       * @brief Returns value
       * @return Value
       */
      const TSetValue& GetValue() const;

      private:

      bool validate_node_data() const;

      Map            m_nodes;
      Arrow::List    m_arrows;
      NName          m_name;
      EType          m_type;
      TSetValue      m_value;
   };

   struct CAT_EXPORT NodeKeyHasher
   {
      std::size_t operator()(const Node& n_) const;
   };
}
