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
   for (const auto& [node, _] : node_->Nodes())
   {
      Node mapped_node = SingleMap(node).value();
      if (!ret.Proof(mapped_node))
         ret.AddNode(mapped_node);
   }

   // Mapping of arrows
   for (const Arrow& arrow : node_->QueryArrows("* -> *"))
   {
      auto source = SingleMap(Node(arrow.m_source));
      auto target = SingleMap(Node(arrow.m_target));

      Arrow mapped_arrow(source->Name(), target->Name());
      if (!ret.Proof(mapped_arrow))
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
//-----------------------------------------------------------------------------------------
static bool validate_node_data(const Node& node_)
{
   int sz {};
   for (const auto& [_, codomain] : node_.Nodes())
   {
      sz += codomain.size();
   }
   return sz == (int)node_.QueryArrows("* -> *").size();
}

//-----------------------------------------------------------------------------------------
Node::Node(const NName& name_) : m_name(name_)
{
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrow(const Arrow& arrow_)
{
   if (Proof(arrow_))
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
bool Node::AddNode(const Node& node_)
{
   if (node_.Name().empty())
      return false;

   if (m_nodes.find(node_) == m_nodes.end())
   {
      m_nodes[node_];

      Arrow func(node_.Name(), node_.Name(), id_arrow_name(node_.Name()));

      for (const auto& [id, _] : node_.Nodes())
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
std::list<Node::NName> Node::FindSources(const NName& target_) const
{
   std::list<NName> ret;

   for (const Arrow& func : m_arrows)
   {
      if (func.Target() == target_)
         ret.push_back(func.Source());
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::list<Node::NName> Node::FindTargets(const NName& source_) const
{
   std::list<NName> ret;

   for (const Arrow& func : m_arrows)
   {
      if (func.Source() == source_)
         ret.push_back(func.Target());
   }

   return ret;
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
std::optional<Node> Node::FindNode(const NName& name_) const
{
   auto it = m_nodes.find(Node(name_));
   if (it != m_nodes.end())
      return it->first;

   return std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
const Node::Map& Node::Nodes() const
{
   return m_nodes;
}

//-----------------------------------------------------------------------------------------
std::optional<Arrow> Node::FindArrow(const NName& source_, const NName& target_) const
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Source() == source_ && arrow.Target() == target_)
         return arrow;
   }

   return std::optional<Arrow>();
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::FindArrows(const NName& source_, const NName& target_) const
{
   Arrow::List ret;

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Source() == source_ && arrow.Target() == target_)
         ret.push_back(arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::optional<Arrow> Node::FindArrow(const Arrow::AName& name_) const
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Name() == name_)
         return std::optional<Arrow>(arrow);
   }

   return std::optional<Arrow>();
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::QueryArrows(const std::string& query_) const
{
   return query_arrows(query_, m_arrows);
}

//-----------------------------------------------------------------------------------------
bool Node::Proof(const Node& node_) const
{
   return m_nodes.find(node_) != m_nodes.end();
}

//-----------------------------------------------------------------------------------------
bool Node::Proof(const Arrow& arrow_) const
{
   auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
      {
         return element_.Source() == arrow_.Source() && element_.Target() == arrow_.Target() && element_.Name() == arrow_.Name();
      });

   return it != m_arrows.end();
}

//-----------------------------------------------------------------------------------------
bool Node::Proof(const Node& source_, const Node& target_) const
{
   bool result{};

   auto its = m_nodes.find(Node(source_));
   if (its != m_nodes.end())
   {
      const auto& [_, codomain] = *its;

      auto itt = codomain.find(Node(target_));
      if (itt != codomain.end())
      {
         result = true;
      }
   }

   return result;
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
      if (!source_cat.FindNode(arrow.Source()))
      {
         print_error("Missing node for " + arrow.Source() + "->" + arrow.Target());
         return false;
      }
   }

   // Checking mapping of objects
   for (const auto& [obj, _] : source_cat.Nodes())
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

      if (!source_cat.Node::Proof(Node(arrow.Source())))
      {
         print_error("No such node " + arrow.Source() + " in node " + source_cat.Name());
         return false;
      }

      if (!target_cat.Node::Proof(objs.value()))
      {
         print_error("No such node " + objs.value().Name() + " in node " + target_cat.Name());
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map node: " + arrow.Target());
         return false;
      }

      if (!source_cat.Node::Proof(Node(arrow.Target())))
      {
         print_error("No such node " + arrow.Target() + " in node " + source_cat.Name());
         return false;
      }

      if (!target_cat.Node::Proof(objt.value()))
      {
         print_error("No such node " + objt.value().Name() + " in node " + target_cat.Name());
         return false;
      }

      // Checking mapping of arrows
      if (!target_cat.Proof(objs.value(), objt.value()))
      {
         print_error("Failure to match arrow: " + objs.value().Name() + "->" + objt.value().Name());
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

   auto source = FindNode(arrow_.Source());
   if (!source)
      return false;

   // Mapping nodes
   for (const auto& [node, _] : source->Nodes())
   {
      std::optional<Node> mnode = arrow_.SingleMap(node);
      if (!mnode)
      {
         print_error("Missing arrow for node " + node.Name() + " in functor " + arrow_.Name());
         return false;
      }

      if (!target.Proof(mnode.value()))
         target.AddNode(mnode.value());
   }

   // Mapping arrows
   for (const Arrow& arrow : source->QueryArrows("* -> *"))
   {
      auto nodes = arrow_.SingleMap(Node(arrow.Source()));
      auto nodet = arrow_.SingleMap(Node(arrow.Target()));

      if (!target.Proof(nodes.value(), nodet.value()))
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

   if (Proof(arrow_))
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
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
