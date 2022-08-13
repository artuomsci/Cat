#include "node.h"

#include <algorithm>

#include "str_utils.h"

using namespace cat;

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
      source   (source_)
   ,  target   (target_)
   ,  name     (arrow_name_)
{};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string& source_, const std::string& target_) :
      source   (source_)
   ,  target   (target_)
   ,  name     (default_arrow_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool Arrow::operator<(const Arrow& arrow_) const
{
   return std::tie(source, target, name) < std::tie(arrow_.source, arrow_.target, arrow_.name);
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator==(const Arrow& arrow_) const
{
   return
         source      == arrow_.source
      && target      == arrow_.target
      && name        == arrow_.name
      && morphisms   == arrow_.morphisms;
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator!=(const Arrow& arrow_) const
{
   return
         source      != arrow_.source
      || target      != arrow_.target
      || name        != arrow_.name
      || morphisms   != arrow_.morphisms;
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::operator()(const std::optional<Node>& node_) const
{
   if (node_->Name() != source)
      return std::optional<Node>();

   Node ret(target);

   // Mapping of nodes
   for (const auto& [node, _] : node_->Nodes())
   {
      Node mapped_node = SingleMap(*this, node).value();
      if (!ret.Proof(mapped_node))
         ret.AddNode(mapped_node);
   }

   // Mapping of arrows
   for (const Arrow& arrow : node_->Arrows())
   {
      auto source = SingleMap(*this, Node(arrow.source));
      auto target = SingleMap(*this, Node(arrow.target));

      ret.AddArrow(Arrow(source->Name(), target->Name()));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Node::Node(const std::string& name_) : m_name(name_)
{
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
bool Node::EraseArrow(const std::string& arrow_)
{
   for (Arrow& morph : m_arrows)
   {
      if (morph.name == arrow_)
      {
         auto it_node = m_nodes.find(Node(morph.source));
         if (it_node != m_nodes.end())
         {
            if (morph.source == morph.target)
               return false;

            auto& [_, node_set] = *it_node;

            node_set.erase(Node(morph.target));
         }
      }
   }

   auto it_begin = std::remove_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
      {
         return element_.name == arrow_;
      });

   if (it_begin == m_arrows.end())
      return false;

   m_arrows.erase(it_begin, m_arrows.end());

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
bool Node::EraseNode(const std::string& node_)
{
   auto it = m_nodes.find(Node(node_));
   if (it != m_nodes.end())
   {
      m_nodes.erase(it);

      std::vector<Arrow::Vec::iterator> arrows; arrows.reserve(m_arrows.size());

      const std::string& node_name = node_;

      for (Arrow::Vec::iterator it = m_arrows.begin(); it != m_arrows.end(); ++it)
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

//-----------------------------------------------------------------------------------------
void Node::EraseNodes()
{
   m_nodes .clear();
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
StringVec Node::FindSources(const std::string& target_) const
{
   StringVec ret; ret.reserve(m_arrows.size());

   for (const Arrow& func : m_arrows)
   {
      if (func.target == target_)
         ret.push_back(func.source);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
StringVec Node::FindTargets(const std::string& source_) const
{
   StringVec ret; ret.reserve(m_arrows.size());

   for (const Arrow& func : m_arrows)
   {
      if (func.source == source_)
         ret.push_back(func.target);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Node::Vec Node::FindByTargets(const StringVec& targets_) const
{
   std::vector<Node> ret; ret.reserve(targets_.size());

   for (const auto& [domain, codomain] : m_nodes)
   {
      for (int i = 0; i < (int)targets_.size(); ++i)
      {
         // skipping identities
         if (domain.Name() == targets_[i])
            continue;

         if (codomain.find(Node(targets_[i])) == codomain.end())
            break;

         if (i == targets_.size() - 1)
            ret.push_back(domain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Node::FindNode(const std::string& name_) const
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
std::optional<Arrow> Node::FindArrow(const std::string& source_, const std::string& target_) const
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.source == source_ && arrow.target == target_)
         return arrow;
   }

   return std::optional<Arrow>();
}

//-----------------------------------------------------------------------------------------
Arrow::Vec Node::FindArrows(const std::string& source_, const std::string& target_) const
{
   Arrow::Vec ret;

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.source == source_ && arrow.target == target_)
         ret.push_back(arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::optional<Arrow> Node::FindArrow(const std::string& name_) const
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.name == name_)
         return std::optional<Arrow>(arrow);
   }

   return std::optional<Arrow>();
}

//-----------------------------------------------------------------------------------------
const Arrow::Vec& Node::Arrows() const
{
   return m_arrows;
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
         return element_.source == arrow_.source && element_.target == arrow_.target && element_.name == arrow_.name;
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
   auto itSourceCat = m_nodes.find(Node(arrow_.source));
   auto itTargetCat = m_nodes.find(Node(arrow_.target));

   if (itSourceCat == m_nodes.end())
   {
      print_error("No such source node: " + arrow_.source);
      return false;
   }

   if (itTargetCat == m_nodes.end())
   {
      print_error("No such target node: " + arrow_.target);
      return false;
   }

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   // Checking mapping of objects
   for (const auto& [obj, _] : source_cat.Nodes())
   {
      if (!SingleMap(arrow_, obj))
      {
         print_error("Failure to map node: " + obj.Name());
         return false;
      }
   }

   for (const Arrow& morph : source_cat.Arrows())
   {
      auto objs = SingleMap(arrow_, Node(morph.source));
      auto objt = SingleMap(arrow_, Node(morph.target));

      if (!objs)
      {
         print_error("Failure to map node: " + morph.source);
         return false;
      }

      if (!source_cat.Node::Proof(Node(morph.source)))
      {
         print_error("No such node '" + morph.source + "' in node '" + source_cat.Name() + "'");
         return false;
      }

      if (!target_cat.Node::Proof(objs.value()))
      {
         print_error("No such node '" + objs.value().Name() + "' in node '" + target_cat.Name() + "'");
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map node: " + morph.target);
         return false;
      }

      if (!source_cat.Node::Proof(Node(morph.target)))
      {
         print_error("No such node '" + morph.target + "' in node '" + source_cat.Name() + "'");
         return false;
      }

      if (!target_cat.Node::Proof(objt.value()))
      {
         print_error("No such node '" + objt.value().Name() + "' in node '" + target_cat.Name() + "'");
         return false;
      }

      // Checking mapping of morphisms
      if (!target_cat.Proof(objs.value(), objt.value()))
      {
         print_error("Failure to match arrow: " + objs.value().Name() + "->" + objt.value().Name());
         return false;
      }
   }

   return true;
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
         func.morphisms.emplace_back(id.Name(), id.Name());

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
bool Node::AddArrow(const Arrow& arrow_)
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow_.name == arrow.name && arrow_.target != arrow.target)
      {
         print_error("Arrow redefinition: " + arrow_.name);
         return false;
      }
   }

   if (!Verify(arrow_))
      return false;

   auto it = m_nodes.find(Node(arrow_.target));

   const auto& [target, _] = *it;

   m_nodes.at(Node(arrow_.source)).insert(target);

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
const std::string& Node::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
bool Node::Statement(const Arrow& arrow_)
{
   Node target(arrow_.target);

   auto it = m_nodes.find(Node(arrow_.target));
   if (it != m_nodes.end())
      target = it->first;

   auto source = FindNode(arrow_.source);
   if (!source)
      return false;

   // Mapping nodes
   for (const auto& [node, _] : source->Nodes())
   {
      std::optional<Node> mnode = SingleMap(arrow_, node);
      if (!mnode)
      {
         print_error("Missing morphism for node " + node.Name() + " in functor " + arrow_.name);
         return false;
      }

      if (!target.Proof(mnode.value()))
         target.AddNode(mnode.value());
   }

   // Mapping arrows
   for (const Arrow& arrow : source->Arrows())
   {
      auto nodes = SingleMap(arrow_, Node(arrow.source));
      auto nodet = SingleMap(arrow_, Node(arrow.target));

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
      print_error("Arrow already defined: " + arrow_.name);
      return false;
   }

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
std::optional<Node> cat::SingleMap(const std::optional<Arrow>& arrow_, const std::optional<Node>& node_)
{
   if (!arrow_ || !node_)
      return std::optional<Node>();

   for (const Arrow& morph : arrow_->morphisms)
   {
      if (morph.source == node_->Name())
      {
         return std::optional<Node>(morph.target);
      }
   }

   return std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
