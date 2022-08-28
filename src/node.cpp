#include "node.h"

#include <algorithm>
#include <assert.h>
#include <cstring>

#include "str_utils.h"

using namespace cat;

// Any entity
static const char* const sAny = "*";
// Arrow name guards
static const char* const sArrowNameBegin = "-[";
static const char* const sArrowNameEnd   = "]->";

//-----------------------------------------------------------------------------------------
static std::string trim_sp(const std::string& string_)
{
   return trim_right(trim_left(string_, ' '), ' ');
}

//-----------------------------------------------------------------------------------------
std::string cat::default_arrow_name(const std::string& source_, const std::string& target_)
{
   return source_ + "-" + target_;
}

//-----------------------------------------------------------------------------------------
std::string cat::id_arrow_name(const std::string& name_)
{
   return default_arrow_name(name_, name_);
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string& source_, const std::string& target_, const std::string& arrow_name_) :
      m_source   (source_)
   ,  m_target   (target_)
   ,  m_name     (arrow_name_)
{};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string& source_, const std::string& target_) :
      m_source   (source_)
   ,  m_target   (target_)
   ,  m_name     (default_arrow_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool Arrow::operator<(const Arrow& arrow_) const
{
   return std::tie(m_source, m_target, m_name) < std::tie(arrow_.m_source, arrow_.m_target, arrow_.m_name);
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator==(const Arrow& arrow_) const
{
   return
         m_source      == arrow_.m_source
      && m_target      == arrow_.m_target
      && m_name        == arrow_.m_name
      && m_arrows      == arrow_.m_arrows;
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator!=(const Arrow& arrow_) const
{
   return
         m_source      != arrow_.m_source
      || m_target      != arrow_.m_target
      || m_name        != arrow_.m_name
      || m_arrows      != arrow_.m_arrows;
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::operator()(const std::optional<Node>& node_) const
{
   if (node_->Name() != m_source)
      return std::optional<Node>();

   Node ret(m_target);

   // Mapping of nodes
   for (const auto& node : node_->QueryNodes("*"))
   {
      Node mapped_node = SingleMap(node).value();
      if (ret.QueryNodes(mapped_node.Name()).empty())
         ret.AddNode(mapped_node);
   }

   // Mapping of arrows
   for (const Arrow& arrow : node_->QueryArrows("* -> *"))
   {
      auto source = SingleMap(Node(arrow.m_source));
      auto target = SingleMap(Node(arrow.m_target));

      Arrow mapped_arrow(source->Name(), target->Name());
      if (ret.QueryArrows(mapped_arrow.Source() + "-[" + mapped_arrow.Name() + "]->" + mapped_arrow.Target()).empty())
         ret.AddArrow(mapped_arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
const std::string& Arrow::Source() const
{
   return m_source;
}

//-----------------------------------------------------------------------------------------
const std::string& Arrow::Target() const
{
   return m_target;
}

//-----------------------------------------------------------------------------------------
const Arrow::AName& Arrow::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
bool Arrow::AddArrow(const Arrow& arrow_)
{
   auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const List::value_type& element_)
   {
      return element_ == arrow_;
   });

   if (it != m_arrows.end())
      return false;

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Arrow::EraseArrow(const Arrow::AName& arrow_)
{
   auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const List::value_type& element_)
   {
      return element_.Name() == arrow_;
   });

   if (it != m_arrows.end())
      m_arrows.erase(it);
   else
      return false;

   return true;
}

//-----------------------------------------------------------------------------------------
void Arrow::EraseArrows()
{
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
static std::optional<Arrow> extract_arrow_from_query(const std::string& query_)
{
   auto query = trim_sp(query_);

   // Arrow name is optional, checking for it
   if (query.find(sArrowNameBegin) != -1 && query.find(sArrowNameEnd) != -1)
   {
      auto head = split(query, sArrowNameBegin, false);
      for (auto& it : head)
         it = trim_sp(it);

      if (head.size() != 2)
         return std::optional<Arrow>();

      auto tail = split(head[1], sArrowNameEnd, false);
      for (auto& it : tail)
         it = trim_sp(it);

      if (tail.size() != 2)
         return std::optional<Arrow>();

      return Arrow(head[0], tail[1], tail[0]);
   }
   else
   {
      auto parts = split(query, "->", false);
      for (auto& it : parts)
         it = trim_sp(it);

      if (parts.size() != 2)
         return std::optional<Arrow>();

      return Arrow(parts[0], parts[1], sAny);
   }
}

//-----------------------------------------------------------------------------------------
Arrow::List query_arrows(const std::string& query_, const Arrow::List& arrows_)
{
   auto qarrow = extract_arrow_from_query(query_);
   if (!qarrow)
      return Arrow::List();

   Arrow::List ret;

   if       (qarrow->Source() == sAny && qarrow->Target() == sAny)
   {
      if (qarrow->Name() == sAny)
      {
         return arrows_;
      }
      else
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Name() == qarrow->Name())
               ret.push_back(arrow);
         }
      }
   }
   else if  (qarrow->Source() != sAny && qarrow->Target() == sAny)
   {
      if (qarrow->Name() == sAny)
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Source() == qarrow->Source())
               ret.push_back(arrow);
         }
      }
      else
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Source() == qarrow->Source() && arrow.Name() == qarrow->Name())
               ret.push_back(arrow);
         }
      }
   }
   else if  (qarrow->Source() == sAny && qarrow->Target() != sAny)
   {
      if (qarrow->Name() == sAny)
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Target() == qarrow->Target())
               ret.push_back(arrow);
         }
      }
      else
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
               ret.push_back(arrow);
         }
      }
   }
   else if  (qarrow->Source() != sAny && qarrow->Target() != sAny)
   {
      if (qarrow->Name() == sAny)
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target())
            {
               ret.push_back(arrow);
            }
         }
      }
      else
      {
         for (const auto& arrow : arrows_)
         {
            if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
            {
               ret.push_back(arrow);
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Arrow::QueryArrows(const std::string& query_) const
{
   return query_arrows(query_, m_arrows);
}

//-----------------------------------------------------------------------------------------
bool Arrow::IsEmpty() const
{
   return m_arrows.empty();
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::SingleMap(const std::optional<Node>& node_) const
{
   if (!node_)
      return std::optional<Node>();

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Source() == node_->Name())
      {
         return std::optional<Node>(arrow.Target());
      }
   }

   return std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
void Arrow::inverse()
{
   m_name = default_arrow_name(m_source, m_target) == m_name ? default_arrow_name(m_target, m_source) : m_name;

   std::swap(m_source, m_target);

   for (auto& arrow : m_arrows)
      arrow.inverse();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Node::Node(const NName& name_) : m_name(name_)
{
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrow(const Arrow& arrow_)
{
   if (!QueryArrows(arrow_.Source() + "-[" + arrow_.Name() + "]->" + arrow_.Target()).empty())
   {
      print_error("Arrow redefinition: " + arrow_.Name());
      return false;
   }

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow_.Name() == arrow.Name() && arrow_.Target() != arrow.Target())
      {
         print_error("Arrow redefinition: " + arrow_.Name());
         return false;
      }
   }

   if (!Verify(arrow_))
      return false;

   auto it = m_nodes.find(Node(arrow_.Target()));

   const auto& [target, _] = *it;

   m_nodes.at(Node(arrow_.Source())).insert(target);

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrows(const Arrow::Vec& arrows_)
{
   for (const Arrow& arrow : arrows_)
   {
      if (!AddArrow(arrow))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseArrow(const Arrow::AName& arrow_)
{
   for (Arrow& arrow : m_arrows)
   {
      if (arrow.Name() == arrow_)
      {
         auto it_node = m_nodes.find(Node(arrow.Source()));
         if (it_node != m_nodes.end())
         {
            if (arrow.Source() == arrow.Target())
               return false;

            auto& [_, node_set] = *it_node;

            node_set.erase(Node(arrow.Target()));
         }
      }
   }

   auto it_delete = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
   {
      return element_.Name() == arrow_;
   });

   if (it_delete == m_arrows.end())
      return false;

   m_arrows.erase(it_delete);

   return true;
}

//-----------------------------------------------------------------------------------------
void Node::EraseArrows()
{
   m_arrows.clear();

   for (auto& [_, nodeset] : m_nodes)
      nodeset.clear();
}

//-----------------------------------------------------------------------------------------
bool Node::IsArrowsEmpty() const
{
   return m_arrows.empty();
}

//-----------------------------------------------------------------------------------------
bool Node::AddNode(const Node& node_)
{
   if (node_.Name().empty())
      return false;

   if (m_nodes.find(node_) == m_nodes.end())
   {
      m_nodes[node_];

      Arrow func(node_.Name(), node_.Name(), id_arrow_name(node_.Name()));

      for (const auto& id : node_.QueryNodes("*"))
         func.AddArrow(Arrow(id.Name(), id.Name()));

      if (!AddArrow(func))
         return false;
   }
   else
   {
      print_error("Node redefinition: " + node_.Name());
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddNodes(const Vec& nodes_)
{
   for (const Node& node : nodes_)
   {
      if (!AddNode(node))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseNode(const NName& node_)
{
   NName name_copy = node_;

   auto it = m_nodes.find(Node(name_copy));
   if (it != m_nodes.end())
   {
      m_nodes.erase(it);

      for (auto& [_, codomain] : m_nodes)
         codomain.erase(Node(name_copy));

      auto it_end = std::remove_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
      {
         return element_.Source() == name_copy || element_.Target() == name_copy;
      });

      m_arrows.erase(it_end, m_arrows.end());

      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
void Node::EraseNodes()
{
   m_nodes .clear();
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
Node::List Node::FindByTargets(const std::list<Node::NName>& targets_) const
{
   Node::List ret;

   for (const auto& [domain, codomain] : m_nodes)
   {
      auto it_last = std::prev(targets_.end());

      for (auto it = targets_.begin(); it != targets_.end(); ++it)
      {
         // skipping identities
         if (domain.Name() == *it)
            continue;

         if (codomain.find(Node(*it)) == codomain.end())
            break;

         if (it == it_last)
            ret.push_back(domain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::IsNodesEmpty() const
{
   return m_nodes.empty();
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::QueryArrows(const std::string& query_) const
{
   return query_arrows(query_, m_arrows);
}

//-----------------------------------------------------------------------------------------
Node::List Node::QueryNodes(const std::string& query_) const
{
   auto query = trim_sp(query_);

   Node::List ret;

   if (query == sAny)
   {
      for (const auto& [node, _] : m_nodes)
      {
         ret.push_back(node);
      }
   }
   else
   {
      auto it = m_nodes.find(Node(query));
      if (it != m_nodes.end())
         ret.push_back(it->first);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::Verify(const Arrow& arrow_) const
{
   auto itSourceCat = m_nodes.find(Node(arrow_.Source()));
   auto itTargetCat = m_nodes.find(Node(arrow_.Target()));

   if (itSourceCat == m_nodes.end())
   {
      print_error("No such source node: " + arrow_.Source());
      return false;
   }

   if (itTargetCat == m_nodes.end())
   {
      print_error("No such target node: " + arrow_.Target());
      return false;
   }

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   for (const Arrow& arrow : arrow_.QueryArrows("*->*"))
   {
      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("Missing node for " + arrow.Source() + "->" + arrow.Target());
         return false;
      }
   }

   // Checking mapping of objects
   for (const auto& obj : source_cat.QueryNodes("*"))
   {
      if (!arrow_.SingleMap(obj))
      {
         print_error("Failure to map node: " + obj.Name());
         return false;
      }
   }

   for (const Arrow& arrow : source_cat.QueryArrows("* -> *"))
   {
      auto objs = arrow_.SingleMap(Node(arrow.Source()));
      auto objt = arrow_.SingleMap(Node(arrow.Target()));

      if (!objs)
      {
         print_error("Failure to map node: " + arrow.Source());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("No such node " + arrow.Source() + " in node " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objs->Name()).empty())
      {
         print_error("No such node " + objs->Name() + " in node " + target_cat.Name());
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map node: " + arrow.Target());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Target()).empty())
      {
         print_error("No such node " + arrow.Target() + " in node " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objt->Name()).empty())
      {
         print_error("No such node " + objt->Name() + " in node " + target_cat.Name());
         return false;
      }

      // Checking mapping of arrows
      if (target_cat.QueryArrows(objs->Name() + "->" + objt->Name()).empty())
      {
         print_error("Failure to match arrow: " + objs->Name() + "->" + objt->Name());
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
const Node::NName& Node::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
bool Node::Statement(const Arrow& arrow_)
{
   Node target(arrow_.Target());

   auto it = m_nodes.find(Node(arrow_.Target()));
   if (it != m_nodes.end())
      target = it->first;

   auto source = QueryNodes(arrow_.Source());
   if (source.empty())
      return false;

   // Mapping nodes
   for (const auto& node : source.front().QueryNodes("*"))
   {
      std::optional<Node> mnode = arrow_.SingleMap(node);
      if (!mnode)
      {
         print_error("Missing arrow for node " + node.Name() + " in functor " + arrow_.Name());
         return false;
      }

      if (target.QueryNodes(mnode->Name()).empty())
         target.AddNode(mnode.value());
   }

   // Mapping arrows
   for (const Arrow& arrow : source.front().QueryArrows("* -> *"))
   {
      auto nodes = arrow_.SingleMap(Node(arrow.Source()));
      auto nodet = arrow_.SingleMap(Node(arrow.Target()));

      if (target.QueryArrows(nodes->Name() + "->" + nodet->Name()).empty())
         target.AddArrow(Arrow(nodes->Name(), nodet->Name()));
   }

   for (auto& [domain, codomain] : m_nodes)
   {
      auto itTarget = codomain.find(target);
      if (itTarget != codomain.end())
      {
         codomain.erase(itTarget);
         codomain.insert(target);
      }
   }

   auto itTarget = m_nodes.find(target);
   if (itTarget != m_nodes.end())
   {
      auto back_up = itTarget->second;

      m_nodes.erase(itTarget);

      m_nodes[target] = back_up;
   }

   if (!QueryArrows(arrow_.Source() + "-[" + arrow_.Name() + "]->" + arrow_.Target()).empty())
   {
      print_error("Arrow already defined: " + arrow_.Name());
      return false;
   }

   m_nodes[Node(arrow_.Source())].insert(Node(arrow_.Target()));

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::operator<(const Node& cat_) const
{
   return m_name < cat_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Node::operator==(const Node& cat_) const
{
   return m_name == cat_.Name();
}

//-----------------------------------------------------------------------------------------
bool Node::operator!=(const Node& cat_) const
{
   return m_name != cat_.Name();
}

//-----------------------------------------------------------------------------------------
void Node::SolveCompositions()
{
   for (const auto & [domain, codomain] : m_nodes)
   {
      Node::Set traverse = codomain;

      Node::Set new_codomain;
      while (!traverse.empty())
      {
         new_codomain.insert(traverse.begin(), traverse.end());

         Node::Set new_traverse;
         for (const Node& node : traverse)
         {
            if (node == domain)
               continue;

            const auto& sub_codomain = m_nodes.at(node);

            for (const Node& sub_node : sub_codomain)
            {
               if (new_codomain.find(sub_node) != new_codomain.end())
                  continue;

               new_traverse.insert(sub_node);
            }
         }

         traverse = new_traverse;
      }

      Node::Set domain_diff;
      std::set_difference(new_codomain.begin(), new_codomain.end(), codomain.begin(), codomain.end(), std::inserter(domain_diff, domain_diff.begin()));

      for (const auto& codomain_node : domain_diff)
         AddArrow(Arrow(domain.Name(), codomain_node.Name()));
   }
}

//-----------------------------------------------------------------------------------------
Node::List Node::Initial() const
{
   Node::List ret;

   for (const auto& [domain, codomain] : m_nodes)
   {
      if (m_nodes.size() == codomain.size())
         ret.push_back(domain);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Node::List Node::Terminal() const
{
   Node::List ret;

   for (const auto& [domain, _] : m_nodes)
   {
      bool is_terminal { true };

      for (const auto& [domain_int, codomain_int] : m_nodes)
      {
         if (std::find_if(codomain_int.begin(), codomain_int.end(), [&](const Node& node_){ return domain == node_; }) == codomain_int.end())
         {
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
Node::List Node::SolveSequence(const Node& from_, const Node& to_) const
{
   Node::List ret;

   std::vector<Node::PairSet> stack;

   std::optional<Node> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         for (auto & [nodei, _] : stack)
            ret.push_back(nodei);

         ret.push_back(current_node.value());

         return ret;
      }

      stack.emplace_back(current_node.value(), m_nodes.at(current_node.value()));

      // Remove identity morphism
      stack.back().second.erase(current_node.value());

      current_node.reset();

      while (!current_node.has_value())
      {
         // Trying new set of nodes
         Node::Set& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one node forward
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node == current_node.value())
            {
               current_node.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::vector<Node::List> Node::SolveSequences(const Node& from_, const Node& to_) const
{
   std::vector<Node::List> ret;

   std::vector<Node::PairSet> stack;

   std::optional<Node> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         Node::List seq;

         for (auto & [nodei, _] : stack)
            seq.push_back(nodei);

         seq.push_back(current_node.value());

         ret.push_back(seq);
      }
      else
      {
         // Stacking forward movements
         stack.emplace_back(current_node.value(), m_nodes.at(current_node.value()));

         // Removing identity morphism
         stack.back().second.erase(current_node.value());
      }

      current_node.reset();

      while (!current_node.has_value())
      {
         // Trying new sets of nodes
         Node::Set& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one node forward
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node == current_node.value())
            {
               current_node.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::MapNodes2Arrows(const Node::List& nodes_) const
{
   Arrow::List ret;

   auto it_last = std::prev(nodes_.end());

   for (auto itn = nodes_.begin(); itn != it_last; ++itn)
   {
      auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::List::value_type& elem_)
      {
         return itn->Name() == elem_.Source() && std::next(itn)->Name() == elem_.Target();
      });

      ret.push_back(it != m_arrows.end() ? *it : Arrow("", ""));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
void Node::Inverse()
{
   Arrow::List tmp = m_arrows;

   EraseArrows();

   for (Arrow& arrow : tmp)
   {
      arrow.inverse();
      AddArrow(arrow);
   }
}

//-----------------------------------------------------------------------------------------
bool Node::validate_node_data() const
{
   size_t sz {};
   for (const auto& [_, codomain] : m_nodes)
      sz += codomain.size();

   return sz == m_arrows.size();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
