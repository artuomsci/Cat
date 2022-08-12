#include "node.h"

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
bool Node::EraseNode(const Node& node_)
{
   auto it = m_nodes.find(node_);
   if (it != m_nodes.end())
   {
      m_nodes.erase(it);

      std::vector<Arrow::Vec::iterator> arrows; arrows.reserve(m_arrows.size());

      const std::string& node_name = node_.Name();

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
   m_nodes.clear();
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
std::optional<Arrow> Node::FindArrow(const std::string& source_, const std::string& target_) const
{
   for (const Arrow& func : m_arrows)
   {
      if (func.source == source_ && func.target == target_)
         return std::optional<Arrow>(func);
   }

   return std::optional<Arrow>();
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

   for (const auto& [domain, _] : m_nodes)
   {
      for (int i = 0; i < (int)targets_.size(); ++i)
      {
         // skipping identities
         if (domain.Name() == targets_[i])
            continue;

         if (!FindArrow(domain.Name(), targets_[i]))
            break;

         if (i == targets_.size() - 1)
            ret.push_back(domain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
const Node::Map& Node::Nodes() const
{
   return m_nodes;
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
bool Node::Statement(const Arrow& func_)
{
   auto itSourceCat = m_nodes.find(Node(func_.source));
   auto itTargetCat = m_nodes.find(Node(func_.target));

   Node source_cat("");
   Node target_cat("");

   StringVec backup_targets[2];
   StringVec backup_sources[2];

   // source
   if (itSourceCat == m_nodes.end())
      source_cat = Node(func_.source);
   else
   {
      source_cat = (*itSourceCat).first;

      // Backup relations
      backup_targets[0] = FindTargets(source_cat.Name());
      backup_sources[0] = FindSources(source_cat.Name());
   }

   // target
   if (itTargetCat == m_nodes.end())
      target_cat = Node(func_.target);
   else
   {
      target_cat = (*itTargetCat).first;

      // Backup relations
      backup_targets[1] = FindTargets(target_cat.Name());
      backup_sources[1] = FindSources(target_cat.Name());
   }

   //
   for (auto& [_, nodes_set] : m_nodes)
      nodes_set.erase(source_cat);

   m_nodes.erase(source_cat);

   //
   for (auto& [_, nodes_set] : m_nodes)
      nodes_set.erase(target_cat);

   m_nodes.erase(target_cat);

   // Mapping objects
   for (const auto& [obj, _] : source_cat.Nodes())
   {
      std::optional<Node> tobj = SingleMap(func_, obj);
      if (!tobj)
      {
         print_error("Missing morphism for object " + obj.Name() + " in functor " + func_.name);

         return false;
      }

      if (!target_cat.Node::Proof(tobj.value()))
         target_cat.AddNode(tobj.value());
   }

   // Mapping morphisms
   for (const Arrow& morph : source_cat.Arrows())
   {
      auto objs = SingleMap(func_, Node(morph.source));
      auto objt = SingleMap(func_, Node(morph.target));

      if (!target_cat.Proof(objs.value(), objt.value()))
         target_cat.AddArrow(Arrow(objs->Name(), objt->Name()));
   }

   if (!AddNode(source_cat))
      return false;

   if (!AddNode(target_cat))
      return false;

   // Restoring relations
   {
      for (const auto& scat : backup_sources[0])
         m_nodes[Node(scat)].insert(source_cat);

      auto crt_category_targets = m_nodes[Node(source_cat)];
      for (const auto& tcat : backup_targets[0])
      {
         const auto& [cat, _] = *m_nodes.find(Node(tcat));
         crt_category_targets.insert(cat);
      }
   }

   // Restoring relations
   {
      for (const auto& scat : backup_sources[1])
         m_nodes[Node(scat)].insert(target_cat);

      auto crt_category_targets = m_nodes[Node(target_cat)];
      for (const auto& tcat : backup_targets[1])
      {
         const auto& [cat, _] = *m_nodes.find(Node(tcat));
         crt_category_targets.insert(cat);
      }
   }

   m_arrows.push_back(func_);

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
std::optional<Node> cat::Map(const std::optional<Arrow>& arrow_, const std::optional<Node>& node_)
{
   if (node_->Name() != arrow_->source)
      return std::optional<Node>();

   Node ret(arrow_->target);

   // Mapping of objects
   for (const auto& [obj, _] : node_->Nodes())
   {
      Node mapped_obj = SingleMap(arrow_, obj).value();
      if (!ret.Proof(mapped_obj))
         ret.AddNode(mapped_obj);
   }

   // Mapping of morphisms
   for (const Arrow& morph : node_->Arrows())
   {
      auto source = SingleMap(arrow_, Node(morph.source));
      auto target = SingleMap(arrow_, Node(morph.target));

      ret.AddArrow(Arrow(source->Name(), target->Name()));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
