#pragma once

#include <map>
#include <vector>
#include <set>
#include <optional>
#include <string>
#include <list>
#include <filesystem>
#include <variant>

#include "cat_export.h"

#include "log.h"

namespace cat
{
   class Node;

   enum class ESetTypes
   {
         eDouble
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

      enum class EType {
            eMorphism   // Morphism
         ,  eFunctor    // Functor
         ,  eUndefined
      };

      /**
      * @brief Converts type to string
      * @return Type name
      */
      static std::string Type2Name(EType type_);

      Arrow(EType type_, const std::string& source_, const std::string& target_, const std::string& arrow_name_);
      Arrow(EType type_, const std::string& source_, const std::string& target_);

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
       * @brief Returns arrow target name
       * @return Name
       */
      const std::string& Target() const;

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

      //========================================= Set category =========================================
      /**
      * @brief Adds const function value
      * @param aname_ - function name
      * @param value_ - function value
      */
      void AddFunctionValue(const Arrow::AName& aname_, const TSetValue& value_);

      /**
      * @brief Returns const function value
      * @param aname_ - function name
      * @return Function value
      */
      std::optional<TSetValue> GetFunctionValue(const Arrow::AName& aname_) const;

      /**
      * @brief Removes const function value
      * @param aname_ - function name
      */
      void RemoveFunctionValue(const Arrow::AName& aname_);

      /**
      * @brief Returns const functions values
      * @return Functions
      */
      std::list<Function> GetFunctions() const;

      /**
      * @brief Removes functions
      */
      void RemoveFunctions();

   private:

      std::string m_source;
      std::string m_target;
      AName       m_name;
      List        m_arrows;
      EType       m_type;

      // Set data
      // const functions values
      std::map<AName, TSetValue> m_fn2value;
   };

   /**
    * @brief The Node class represents objects and categories
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

      enum class EType {
            eObject     // Object
         ,  eSCategory  // Small category
         ,  eLCategory  // Large category
         ,  eUndefined
      };

      /**
      * @brief Converts type to string
      * @return Type name
      */
      static std::string Type2Name(EType type_);

      bool operator < (const Node& cat_) const;
      bool operator ==(const Node& cat_) const;
      bool operator !=(const Node& cat_) const;

      /**
      * @brief Node constructor
      * @param name_ - node name
      */
      explicit Node(const NName& name_, EType type_);

      /**
      * @brief Parses file
      * @return True if file was successfully parsed
      */
      bool Parse(const std::string& path_);

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
      Node Query(const std::string& query_, std::optional<size_t> matchCount_ = std::optional<size_t>());

      /**
       * @brief Verifying arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool Verify(const Arrow& arrow_) const;

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

      private:

      bool validate_node_data() const;

      /**
       * @brief Parse the file contents
       * @param path_ - file path
       * @return True if file was successfully parsed
       */
      bool parse_source(const std::string& path_);

      Map            m_nodes;
      Arrow::List    m_arrows;
      NName          m_name;
      EType          m_type;
   };

   struct CAT_EXPORT NodeKeyHasher
   {
      std::size_t operator()(const Node& n_) const;
   };
}
