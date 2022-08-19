#pragma once

#include <map>
#include <vector>
#include <set>
#include <optional>
#include <string>
#include <list>

#include "cat_export.h"

#include "types.h"
#include "log.h"

namespace cat
{
   /**
    * @brief Return default arrow name
    * @param source_ - source name
    * @param target_ - target name
    * @return Arrow name
    */
   CAT_EXPORT std::string default_arrow_name(const std::string& source_, const std::string& target_);

   /**
    * @brief Return identity arrow name
    * @param name_ - source/target name
    * @return Arrow name
    */
   CAT_EXPORT std::string id_arrow_name(const std::string& name_);

   class Node;

   /**
    * @brief The Arrow struct represents morphisms and functors
    */
   struct CAT_EXPORT Arrow
   {
      Arrow(const std::string& source_, const std::string& target_, const std::string& arrow_name_);
      Arrow(const std::string& source_, const std::string& target_);

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

      std::string source;
      std::string target;
      AName       name;
      List        arrows;
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

      /**
      * @brief Node constructor
      * @param name_ - node name
      */
      explicit Node(const NName& name_);

      /**
       * @brief Add arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool AddArrow(const Arrow& arrow_);

      /**
       * @brief Add arrows
       * @param arrows_ - arrows
       * @return True if successful
       */
      bool AddArrows(const Arrow::Vec& arrows_);

      /**
       * @brief Erase arrow
       * @param arrow_ - arrow name
       * @return True if successfull
       */
      bool EraseArrow(const Arrow::AName& arrow_);

      /**
       * @brief Erase all arrows
       */
      void EraseArrows();

      /**
      * @brief Add node
      * @param node_ - node
      * @return True if successful
      */
      bool AddNode(const Node& node_);

      /**
       * @brief Add nodes
       * @param nodes_ - nodes
       * @return True if successful
       */
      bool AddNodes(const Vec& nodes_);

      /**
       * @brief Erase node
       * @param node_ - node name
       * @return True if successfull
       */
      bool EraseNode(const NName& node_);

      /**
       * @brief Erase all nodes
       */
      void EraseNodes();

      /**
       * @brief Find source nodes for the target node
       * @param target_ - target name
       * @return Source nodes
       */
      std::list<NName> FindSources(const NName& target_) const;

      /**
       * @brief Find target nodes for the source node
       * @param source_ - source name
       * @return Target nodes
       */
      std::list<NName> FindTargets(const NName& source_) const;

      /**
      * @brief Find nodes as sources to the identified targets
      * @param targets_ - target node names
      * @return Nodes
      */
      List FindByTargets(const std::list<NName>& targets_) const;

      /**
       * @brief Find node
       * @param name_ - node name
       * @return Node
       */
      std::optional<Node> FindNode(const NName& name_) const;

      /**
       * @brief Return nodes
       * @return Nodes
       */
      const Map& Nodes() const;

      /**
       * @brief Find arrow by source and target
       * @param source_ - source name
       * @param target_ - target name
       * @return Arrow
       */
      std::optional<Arrow> FindArrow(const NName& source_, const NName& target_) const;

      /**
       * @brief Find arrows by source and target
       * @param source_ - source name
       * @param target_ - target name
       * @return Arrows
       */
      Arrow::List FindArrows(const NName& source_, const NName& target_) const;

      /**
       * @brief Find arrow
       * @param name_ - arrow name
       * @return Arrow
       */
      std::optional<Arrow> FindArrow(const Arrow::AName& name_) const;

      /**
       * @brief Return arrows
       * @return Arrows
       */
      const Arrow::List& Arrows() const;

      /**
       * @brief Proof node
       * @param node_ - node
       * @return True if successful
       */
      bool Proof(const Node& node_) const;

      /**
       * @brief Proof arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool Proof(const Arrow& arrow_) const;

      /**
       * @brief Proof arrow
       * @param source_ - source
       * @param target_ - target
       * @return True if successful
       */
      bool Proof(const Node& source_, const Node& target_) const;

      /**
       * @brief Verifying arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool Verify(const Arrow& arrow_) const;

      /**
       * @brief Return node name
       * @return Name
       */
      const NName& Name() const;

      bool Statement(const Arrow& arrow_);

      bool operator < (const Node& cat_) const;
      bool operator ==(const Node& cat_) const;
      bool operator !=(const Node& cat_) const;

      private:

      Map            m_nodes;
      Arrow::List    m_arrows;
      NName          m_name;
   };

   struct CAT_EXPORT NodeKeyHasher
   {
      std::size_t operator()(const Node& n_) const;
   };

   /**
   * @brief Map node with arrow
   * @param arrow_ - arrow to map with
   * @param node_ - node for mapping
   * @return Mapped node
   */
   CAT_EXPORT std::optional<Node> SingleMap(const std::optional<cat::Arrow>& arrow_, const std::optional<Node>& node_);
}
