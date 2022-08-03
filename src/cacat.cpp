#include "cacat.h"

#include <algorithm>

#include "str_utils.h"
#include "common.h"
#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CName& source_, const Cat::CName& target_, const FuncName& name_) :
   Arrow(source_, target_, name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CName& source_, const Cat::CName& target_) :
   Arrow(source_, target_, default_arrow_name(source_, target_))
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_, const FuncName& name_) :
   Arrow(source_.Name(), target_.Name(), name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_) :
   Arrow(source_.Name(), target_.Name(), default_arrow_name(source_.Name(), target_.Name()))
{}

//-----------------------------------------------------------------------------------------
bool Func::operator<(const Func& func_) const
{
   return std::tuple(source, target, name) < std::tuple(func_.source, func_.target, name);
}

//-----------------------------------------------------------------------------------------
bool Func::operator==(const Func& func_) const
{
   return
         (source     == func_.source)
      && (target     == func_.target)
      && (name       == func_.name)
      && (morphisms  == func_.morphisms);
}

//-----------------------------------------------------------------------------------------
bool Func::operator!=(const Func& func_) const
{
   return !(*this == func_);
}

//-----------------------------------------------------------------------------------------
std::optional<Obj> Func::operator()(const std::optional<Obj>& obj_) const
{
   return MapObject(*this, obj_);
}

//-----------------------------------------------------------------------------------------
std::optional<Obj> cat::MapObject(const std::optional<Func>& func_, const std::optional<Obj>& obj_)
{
   if (!func_ || !obj_)
      return std::optional<Obj>();

   for (const Morph& morph : func_.value().morphisms)
   {
      if (morph.source == obj_->Name())
      {
         return std::optional<Obj>(morph.target);
      }
   }

   return std::optional<Obj>();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool CACat::AddNode(const Cat& node_)
{
   if (node_.Name().empty())
      return false;

   if (m_nodes.find(node_) == m_nodes.end())
   {
      m_nodes[node_];

      Func func(node_, node_, id_arrow_name(node_.Name()));

      for (const auto& [id, _] : node_.Nodes())
         func.morphisms.emplace_back(Obj(id), Obj(id));

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
bool CACat::AddArrow(const Func& arrow_)
{
   for (const Func& func : m_arrows)
   {
      if (arrow_.name == func.name && arrow_.target != func.target)
      {
         print_error("Arrow redefinition: " + arrow_.name);
         return false;
      }
   }

   if (!Verify(arrow_))
      return false;

   auto it = m_nodes.find(Cat(arrow_.target));

   const auto& [cat, _] = *it;

   m_nodes.at(Cat(arrow_.source)).insert(cat);

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::Verify(const Func& func_) const
{
   auto itSourceCat = m_nodes.find(Cat(func_.source));
   auto itTargetCat = m_nodes.find(Cat(func_.target));

   if (itSourceCat == m_nodes.end())
   {
      print_error("No such source category: " + func_.source);
      return false;
   }

   if (itTargetCat == m_nodes.end())
   {
      print_error("No such target category: " + func_.target);
      return false;
   }

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   // Checking mapping of objects
   for (const auto& [obj, _] : source_cat.Nodes())
   {
      if (!MapObject(func_, obj))
      {
         print_error("Failure to map object: " + obj.Name());
         return false;
      }
   }

   for (const Morph& morph : source_cat.Arrows())
   {
      auto objs = MapObject(func_, Obj(morph.source));
      auto objt = MapObject(func_, Obj(morph.target));

      if (!objs)
      {
         print_error("Failure to map object: " + morph.source);
         return false;
      }

      if (!source_cat.CatFrame::Proof(Obj(morph.source)))
      {
         print_error("No such object '" + morph.source + "' in category '" + source_cat.Name() + "'");
         return false;
      }

      if (!target_cat.CatFrame::Proof(objs.value()))
      {
         print_error("No such object '" + objs.value().Name() + "' in category '" + target_cat.Name() + "'");
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map object: " + morph.target);
         return false;
      }

      if (!source_cat.CatFrame::Proof(Obj(morph.target)))
      {
         print_error("No such object '" + morph.target + "' in category '" + source_cat.Name() + "'");
         return false;
      }

      if (!target_cat.CatFrame::Proof(objt.value()))
      {
         print_error("No such object '" + objt.value().Name() + "' in category '" + target_cat.Name() + "'");
         return false;
      }

      // Checking mapping of morphisms
      if (!target_cat.Proof(objs.value(), objt.value()))
      {
         print_error("Failure to match morphism: " + objs.value().Name() + "->" + objt.value().Name());
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::Statement(const Func& func_)
{
   auto itSourceCat = m_nodes.find(Cat(func_.source));
   auto itTargetCat = m_nodes.find(Cat(func_.target));

   Cat source_cat("");
   Cat target_cat("");

   NameVec backup_targets[2];
   NameVec backup_sources[2];

   // source
   if (itSourceCat == m_nodes.end())
      source_cat = Cat(func_.source);
   else
   {
      source_cat = (*itSourceCat).first;

      // Backup relations
      backup_targets[0] = FindTargets(source_cat.Name());
      backup_sources[0] = FindSources(source_cat.Name());
   }

   // target
   if (itTargetCat == m_nodes.end())
      target_cat = Cat(func_.target);
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
      std::optional<Obj> tobj = MapObject(func_, obj);
      if (!tobj)
      {
         print_error("Missing functor morphism for object: " + obj.Name());
         return false;
      }

      if (!target_cat.CatFrame::Proof(tobj.value()))
         target_cat.AddNode(tobj.value());
   }

   // Mapping morphisms
   for (const Morph& morph : source_cat.Arrows())
   {
      auto objs = MapObject(func_, Obj(morph.source));
      auto objt = MapObject(func_, Obj(morph.target));

      if (!target_cat.Proof(objs.value(), objt.value()))
         target_cat.AddArrow(Morph(objs->Name(), objt->Name()));
   }

   if (!AddNode(source_cat))
      return false;

   if (!AddNode(target_cat))
      return false;

   // Restoring relations
   {
      for (const auto& scat : backup_sources[0])
         m_nodes[Cat(scat)].insert(source_cat);

      auto crt_category_targets = m_nodes[Cat(source_cat)];
      for (const auto& tcat : backup_targets[0])
      {
         const auto& [cat, _] = *m_nodes.find(Cat(tcat));
         crt_category_targets.insert(cat);
      }
   }

   // Restoring relations
   {
      for (const auto& scat : backup_sources[1])
         m_nodes[Cat(scat)].insert(target_cat);

      auto crt_category_targets = m_nodes[Cat(target_cat)];
      for (const auto& tcat : backup_targets[1])
      {
         const auto& [cat, _] = *m_nodes.find(Cat(tcat));
         crt_category_targets.insert(cat);
      }
   }

   m_arrows.push_back(func_);

   return true;
}
