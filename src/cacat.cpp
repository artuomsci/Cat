#include "cacat.h"

#include <algorithm>

#include "str_utils.h"
#include "common.h"
#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CatName& source_, const Cat::CatName& target_, const FuncName& name_) :
   Arrow(source_, target_, name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CatName& source_, const Cat::CatName& target_) :
   Arrow(source_, target_, default_arrow_name(source_, target_))
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_, const FuncName& name_) :
   Arrow(source_.GetName(), target_.GetName(), name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_) :
   Arrow(source_.GetName(), target_.GetName(), default_arrow_name(source_.GetName(), target_.GetName()))
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
std::optional<Obj> cat::MapObject(const std::optional<Func>& func_, const std::optional<Obj>& obj_)
{
   if (!func_ || !obj_)
      return std::optional<Obj>();

   for (const Morph& morph : func_.value().morphisms)
   {
      if (morph.source == obj_->GetName())
      {
         return std::optional<Obj>(morph.target);
      }
   }

   return std::optional<Obj>();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool CACat::AddCategory(const Cat& cat_)
{
   if (cat_.GetName().empty())
      return false;

   if (m_cats.find(cat_) == m_cats.end())
      m_cats[cat_];
   else
   {
      print_error("Category redefinition: " + cat_.GetName());
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::EraseCategory(const Cat& cat_)
{
   auto it = m_cats.find(cat_);
   if (it != m_cats.end())
   {
      m_cats.erase(it);

      std::vector<FuncVec::iterator> funcs; funcs.reserve(m_funcs.size());

      for (FuncVec::iterator it = m_funcs.begin(); it != m_funcs.end(); ++it)
      {
         if (((*it).source == cat_.GetName()) || ((*it).target == cat_.GetName()))
            funcs.push_back(it);
      }

      while (!funcs.empty())
      {
         m_funcs.erase(funcs.back());
         funcs.pop_back();
      }

      return true;
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------
bool CACat::AddFunctor(Func func_, EExpType type_)
{
   for (const Func& func : m_funcs)
   {
      if (func_.name == func.name && func_.target != func.target)
      {
         print_error("Functor redefinition: " + func_.name);
         return false;
      }
   }

   if (type_ == EExpType::eProof)
   {
      if (!Proof(func_))
         return false;
   }
   else if (type_ == EExpType::eStatement)
   {
      if (func_.morphisms.empty())
      {
         auto itSourceCat = Categories().find(Cat(func_.source));
         if (itSourceCat == Categories().end())
         {
            print_error("Missing source category: " + func_.source);
            return false;
         }

         const auto& [source_cat, _] = *itSourceCat;

         for (const auto& [obj, _] : source_cat.GetObjects())
            func_.morphisms.emplace_back(obj.GetName(), obj.GetName());
      }

      if (!Statement(func_))
         return false;
   }

   auto it = std::find_if(m_funcs.begin(), m_funcs.end(), [&](const FuncVec::value_type& element_)
   {
      return element_.name == func_.name && element_.source == func_.source && element_.target == func_.target;
   });

   if (it == m_funcs.end())
      m_funcs.push_back(func_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::EraseFunctor(const Func& func_)
{
   auto it = std::find_if(m_funcs.begin(), m_funcs.end(), [&](const std::set<Func>::value_type& elem_) {
      return elem_.name == func_.name;
   });

   if (it == m_funcs.end())
      return false;

   m_funcs.erase(it);

   return true;
}

//-----------------------------------------------------------------------------------------
void CACat::EraseFunctors()
{
   m_funcs.clear();

   for (auto& [cat, catset] : m_cats)
      catset.clear();
}

//-----------------------------------------------------------------------------------------
const CatUMap& CACat::Categories() const
{
   return m_cats;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
const FuncVec& CACat::Functors() const
{
   return m_funcs;
}

//-----------------------------------------------------------------------------------------
bool CACat::Proof(const Func& func_) const
{
   auto itSourceCat = m_cats.find(Cat(func_.source));
   auto itTargetCat = m_cats.find(Cat(func_.target));

   if (itSourceCat == m_cats.end() || itTargetCat == m_cats.end())
      return false;

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   // Checking mapping of objects
   for (const auto& [obj, _] : source_cat.GetObjects())
   {
      if (!MapObject(func_, obj).has_value())
      {
         print_error("Failure to map object: " + obj.GetName());
         return false;
      }
   }

   for (const Morph& morph : source_cat.GetMorphisms())
   {
      auto objs = MapObject(func_, Obj(morph.source));
      auto objt = MapObject(func_, Obj(morph.target));

      if (!objs)
      {
         print_error("Failure to map object: " + morph.source);
         return false;
      }

      if (source_cat.GetObjects().find(Obj(morph.source)) == source_cat.GetObjects().end())
      {
         print_error("No such object '" + morph.source + "' in category '" + source_cat.GetName() + "'");
         return false;
      }

      if (target_cat.GetObjects().find(objs.value()) == target_cat.GetObjects().end())
      {
         print_error("No such object '" + objs.value().GetName() + "' in category '" + target_cat.GetName() + "'");
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map object: " + morph.target);
         return false;
      }

      if (source_cat.GetObjects().find(Obj(morph.target)) == source_cat.GetObjects().end())
      {
         print_error("No such object '" + morph.target + "' in category '" + source_cat.GetName() + "'");
         return false;
      }

      if (target_cat.GetObjects().find(objt.value()) == target_cat.GetObjects().end())
      {
         print_error("No such object '" + objt.value().GetName() + "' in category '" + target_cat.GetName() + "'");
         return false;
      }

      // Checking mapping of morphisms
      if (!target_cat.MatchMorphism(objs.value(), objt.value()))
      {
         print_error("Failure to match morphism: " + objs.value().GetName() + "->" + objt.value().GetName());
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::Statement(const Func& func_)
{
   auto itSourceCat = m_cats.find(Cat(func_.source));
   if (itSourceCat == m_cats.end())
   {
      print_error("Missing source category: " + func_.source);
      return false;
   }

   const auto& [source_cat, _] = *itSourceCat;

   auto itTargetCat = m_cats.find(Cat(func_.target));

   Cat target_cat("");

   CatNameVec backup_targets;
   CatNameVec backup_sources;

   if (itTargetCat == m_cats.end())
      target_cat = Cat(func_.target);
   else
   {
      target_cat = (*itTargetCat).first;

      // Backup relations
      backup_targets = FindTargets(target_cat.GetName());
      backup_sources = FindSources(target_cat.GetName());
   }

   eraseInstances(target_cat);

   // Mapping objects
   for (const auto& [obj, _] : source_cat.GetObjects())
   {
      std::optional<Obj> tobj = MapObject(func_, obj);
      if (!tobj.has_value())
      {
         print_error("Missing functor morphism for object: " + obj.GetName());
         return false;
      }

      if (target_cat.GetObjects().find(tobj.value()) == target_cat.GetObjects().end())
         target_cat.AddObject(tobj.value());
   }

   // Mapping morphisms
   for (const Morph& morph : source_cat.GetMorphisms())
   {
      auto objs = MapObject(func_, Obj(morph.source));
      auto objt = MapObject(func_, Obj(morph.target));

      if (!target_cat.MatchMorphism(objs.value(), objt.value()))
         target_cat.AddMorphism(Morph(objs->GetName(), objt->GetName()));
   }

   if (!AddCategory(target_cat))
      return false;

   // Restoring relations
   for (const auto& scat : backup_sources)
      m_cats[Cat(scat)].insert(target_cat);

   auto crt_category_targets = m_cats[Cat(target_cat)];
   for (const auto& tcat : backup_targets)
   {
      const auto& [cat, _] = *m_cats.find(Cat(tcat));
      crt_category_targets.insert(cat);
   }

   return true;
}

//-----------------------------------------------------------------------------------------
std::optional<Func> CACat::FindFunctor(const Cat::CatName& source_, const Cat::CatName& target_) const
{
   for (const Func& func : m_funcs)
   {
      if (func.source == source_ && func.target == target_)
         return std::optional<Func>(func);
   }

   return std::optional<Func>();
}

//-----------------------------------------------------------------------------------------
CatNameVec CACat::FindSources(const Cat::CatName& target_) const
{
   CatNameVec ret; ret.reserve(m_funcs.size());

   for (const Func& func : m_funcs)
   {
      if (func.target == target_)
         ret.push_back(func.source);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
CatNameVec CACat::FindTargets(const Cat::CatName& source_) const
{
   CatNameVec ret; ret.reserve(m_funcs.size());

   for (const Func& func : m_funcs)
   {
      if (func.source == source_)
         ret.push_back(func.target);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
CatVec CACat::FindByTargets(const CatNameVec& targets_) const
{
   CatVec ret; ret.reserve(targets_.size());

   for (const auto& [domain, _] : m_cats)
   {
      for (int i = 0; i < (int)targets_.size(); ++i)
      {
         if (!FindFunctor(domain.GetName(), targets_[i]))
            continue;

         if (i == targets_.size() - 1)
            ret.push_back(domain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
void CACat::eraseInstances(const Cat& cat_)
{
   for (auto& [cat, cat_set] : m_cats)
      cat_set.erase(cat_);

   m_cats.erase(cat_);
}
