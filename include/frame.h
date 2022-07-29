#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>

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
   inline std::string default_arrow_name(const std::string& source_, const std::string& target_)
   {
      return source_ + "-" + target_;
   }

   /**
    * @brief Return identity arrow name
    * @param name_ - source/target name
    * @return Arrow name
    */
   inline std::string id_arrow_name(const std::string& name_)
   {
      return default_arrow_name(name_, name_);
   }

   /**
    * @brief The Arrow struct represents morphisms/functors
    */
   struct Arrow
   {
      explicit Arrow(const std::string& source_, const std::string& target_, const std::string& arrow_name_);
      explicit Arrow(const std::string& source_, const std::string& target_);

      Arrow(const Arrow&) = default;
      Arrow(Arrow&&) = default;

      Arrow& operator = (Arrow&&) = default;
      Arrow& operator = (const Arrow&) = default;

      bool operator  < (const Arrow& arrow_) const;
      bool operator == (const Arrow& arrow_) const;
      bool operator != (const Arrow& arrow_) const;

      std::string source;
      std::string target;
      std::string name;
   };

   /**
    * @brief The Frame class represents categories and categories of categories
    */
   template <typename TNode, typename TNodeHasher, typename TArrow>
   class Frame
   {
   public:

      using NodeSet  = std::set<TNode>;
      using NodeUMap = std::unordered_map<TNode, NodeSet, TNodeHasher>;

      using ArrowVec = std::vector<TArrow>;

      /**
       * @brief Add arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      template <typename T, typename... TArgs>
      bool AddArrows(const T& arrow_, const TArgs&... arrows_)
      {
         return addArrow(arrow_) ? AddArrows(arrows_...) : false;
      }

      /**
       * @brief Terminal condition
       * @return True
       */
      bool AddArrows() { return true; }

      /**
       * @brief Erase arrow
       * @param arrow_ - arrow
       * @return True if successfull
       */
      bool EraseArrow(const std::string& arrow_)
      {
         for (TArrow& morph : m_arrows)
         {
            if (morph.name == arrow_)
            {
               auto it_node = m_nodes.find(TNode(morph.source));
               if (it_node != m_nodes.end())
               {
                  if (morph.source == morph.target)
                     return false;

                  auto& [_, node_set] = *it_node;

                  node_set.erase(TNode(morph.target));
               }
            }
         }

         auto it_begin = std::remove_if(m_arrows.begin(), m_arrows.end(), [&](const typename ArrowVec::value_type& element_)
            {
               return element_.name == arrow_;
            });

         if (it_begin == m_arrows.end())
            return false;

         m_arrows.erase(it_begin, m_arrows.end());

         return true;
      }

      /**
       * @brief Erase all arrows
       */
      void EraseArrows()
      {
         m_arrows.clear();

         for (auto& [_, nodeset] : m_nodes)
            nodeset.clear();
      }

      /**
       * @brief Add node
       * @param node_ - node
       * @return True if successful
       */
      bool AddNode(const TNode& node_)
      {
         if (node_.GetName().empty())
            return false;

         if (m_nodes.find(node_) == m_nodes.end())
         {
            m_nodes[node_];

            if (!addArrow(TArrow(node_, node_, id_arrow_name(node_.GetName()))))
               return false;
         }
         else
         {
            print_error("Node redefinition: " + node_.GetName());
            return false;
         }

         return true;
      }

      /**
       * @brief Add multiple nodes
       * @param node_ - node
       * @return True if successful
       */
      template <typename T, typename... TArgs>
      bool AddNodes(const T& node_, const TArgs&... nodes_)
      {
         return AddNode(node_) ? AddNodes(nodes_...) : false;
      }

      /**
       * @brief Terminal condition
       */
      bool AddNodes() { return true; }

      /**
       * @brief Erase node
       * @param node_ - node
       * @return True if successfull
       */
      bool EraseNode(const TNode& node_)
      {
         auto it = m_nodes.find(node_);
         if (it != m_nodes.end())
         {
            m_nodes.erase(it);

            std::vector<typename ArrowVec::iterator> arrows; arrows.reserve(m_arrows.size());

            const std::string& node_name = node_.GetName();

            for (typename ArrowVec::iterator it = m_arrows.begin(); it != m_arrows.end(); ++it)
            {
               if (((*it).source == node_name) || ((*it).target == node_name))
                  arrows.push_back(it);
            }

            while (!arrows.empty())
            {
               m_arrows.erase(arrows.back());
               arrows.pop_back();
            }

            return true;
         }

         return false;
      }

      /**
       * @brief Erase all nodes
       */
      void EraseNodes()
      {
         m_nodes .clear();
         m_arrows.clear();
      }

      /**
      * @brief Check for arrow
      * @param source_ - source
      * @param target_ - target
      * @return True if arrow exists
      */
      bool MatchArrow(const TNode& source_, const TNode& target_) const
      {
         auto it = m_nodes.find(source_);
         if (it == m_nodes.end())
            return false;

         const auto& [_, codomain] = *it;

         return codomain.find(target_) != codomain.end();
      }

      /**
       * @brief Find arrow by source and target
       * @param source_ - source
       * @param target_ - target
       * @return Arrow
       */
      std::optional<TArrow> FindArrow(const std::string& source_, const std::string& target_) const
      {
         for (const TArrow& func : m_arrows)
         {
            if (func.source == source_ && func.target == target_)
               return std::optional<TArrow>(func);
         }

         return std::optional<TArrow>();
      }

      /**
       * @brief Find source nodes for the target node
       * @param target_ - target category
       * @return Source categories
       */
      StringVec FindSources(const std::string& target_) const
      {
         StringVec ret; ret.reserve(m_arrows.size());

         for (const TArrow& func : m_arrows)
         {
            if (func.target == target_)
               ret.push_back(func.source);
         }

         return ret;
      }

      /**
       * @brief Find target nodes for the source node
       * @param source_ - source
       * @return Target nodes
       */
      StringVec FindTargets(const std::string& source_) const
      {
         StringVec ret; ret.reserve(m_arrows.size());

         for (const TArrow& func : m_arrows)
         {
            if (func.source == source_)
               ret.push_back(func.target);
         }

         return ret;
      }

      /**
      * @brief Find nodes as sources to the identified targets
      * @param targets_ - target nodes
      * @return Nodes
      */
      std::vector<TNode> FindByTargets(const StringVec& targets_) const
      {
         std::vector<TNode> ret; ret.reserve(targets_.size());

         for (const auto& [domain, _] : m_nodes)
         {
            for (int i = 0; i < (int)targets_.size(); ++i)
            {
               if (!FindArrow(domain.GetName(), targets_[i]))
                  continue;

               if (i == targets_.size() - 1)
                  ret.push_back(domain);
            }
         }

         return ret;
      }

      /**
       * @brief Return nodes
       * @return Nodes
       */
      const NodeUMap& Nodes() const
      {
         return m_nodes;
      }

      /**
       * @brief Return arrows
       * @return Arrows
       */
      const ArrowVec& Arrows() const
      {
         return m_arrows;
      }

      /**
       * @brief Proof node
       * @param node_ - node
       * @return True if successful
       */
      bool Proof(const TNode& node_) const
      {
         return m_nodes.find(node_) != m_nodes.end();
      }

      protected:

      /**
       * @brief Add arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool addArrow(const TArrow& arrow_)
      {
         if (!Proof(TNode(arrow_.source)))
         {
            print_error("No such source node: " + arrow_.source);
            return false;
         }

         if (!Proof(TNode(arrow_.target)))
         {
            print_error("No such target node: " + arrow_.target);
            return false;
         }

         for (auto& arrow : m_arrows)
         {
            if (arrow_.name == arrow.name && arrow_.target != arrow.target)
            {
               print_error("Arrow redefinition: " + arrow_.name);
               return false;
            }
         }

         m_nodes[TNode(arrow_.source)].insert(TNode(arrow_.target));

         m_arrows.push_back(arrow_);

         return true;
      }

      /**
       * @brief Proof arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool proof(const TArrow& arrow_) const
      {
         auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const typename ArrowVec::value_type& element_)
         {
            return element_.source == arrow_.source && element_.target == arrow_.target && element_.name == arrow_.name;
         });

         return it != m_arrows.end();
      }

      /**
       * @brief Proof arrow
       * @param source_ - source
       * @param target_ - target
       * @return True if successful
       */
      bool proof(const TNode& source_, const TNode& target_) const
      {
         bool result {};

         auto its = m_nodes.find(TNode(source_));
         if (its != m_nodes.end())
         {
            const auto& [_, codomain] = *its;

            auto itt = codomain.find(TNode(target_));
            if (itt != codomain.end())
            {
               result = true;
            }
         }

         return result;
      }

      NodeUMap m_nodes;
      ArrowVec m_arrows;
   };    
}
