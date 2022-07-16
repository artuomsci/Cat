#include "cacat.h"

#include "str_utils.h"
#include "common.h"
#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CatName& source_, const Cat::CatName& target_, const FuncName& name_) :
      source(source_)
   ,  target(target_)
   ,  name  (name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat::CatName& source_, const Cat::CatName& target_) :
      source(source_)
   ,  target(target_)
   ,  name  (default_functor_name(source, target))
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_, const FuncName& name_) :
      source(source_.GetName())
   ,  target(target_.GetName())
   ,  name  (name_)
{}

//-----------------------------------------------------------------------------------------
Func::Func(const Cat& source_, const Cat& target_) :
   source(source_.GetName())
,  target(target_.GetName())
,  name  (default_functor_name(source, target))
{}

//-----------------------------------------------------------------------------------------
bool Func::operator < (const Func& func_) const
{
   return std::tuple(source, target, name) < std::tuple(func_.source, func_.target, name);
}

//-----------------------------------------------------------------------------------------
std::optional<Obj> cat::MapObject(const Func& func_, const Obj& obj_)
{
   for (const Morph& morph : func_.morphisms)
   {
      if (morph.source == obj_)
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
      m_cats.insert(cat_);
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

      std::vector<FuncSet::iterator> funcs; funcs.reserve(m_funcs.size());

      for (FuncSet::iterator it = m_funcs.begin(); it != m_funcs.end(); ++it)
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
      if (func_.name == func.name && (func_.source != func.source || func_.target != func.target))
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

         const Cat& source_cat = *itSourceCat;

         for (const auto& [obj, _] : source_cat.GetObjects())
            func_.morphisms.emplace(obj, obj);
      }

      if (!Statement(func_))
         return false;
   }

   if (m_funcs.find(func_) == m_funcs.end())
      m_funcs.insert(func_);

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

   m_funcs.erase(func_);

   return true;
}

//-----------------------------------------------------------------------------------------
const CatSet& CACat::Categories() const
{
   return m_cats;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
const FuncSet& CACat::Functors() const
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

   const Cat& source_cat = *itSourceCat;
   const Cat& target_cat = *itTargetCat;

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
      auto objs = MapObject(func_, morph.source);
      auto objt = MapObject(func_, morph.target);

      if (!objs)
      {
         print_error("Failure to map object: " + morph.source.GetName());
         return false;
      }

      if (source_cat.GetObjects().find(morph.source) == source_cat.GetObjects().end())
      {
         print_error("No such object '" + morph.source.GetName() + "' in category '" + source_cat.GetName() + "'");
         return false;
      }

      if (target_cat.GetObjects().find(objs.value()) == target_cat.GetObjects().end())
      {
         print_error("No such object '" + objs.value().GetName() + "' in category '" + target_cat.GetName() + "'");
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map object: " + morph.target.GetName());
         return false;
      }

      if (source_cat.GetObjects().find(morph.target) == source_cat.GetObjects().end())
      {
         print_error("No such object '" + morph.target.GetName() + "' in category '" + source_cat.GetName() + "'");
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

   const Cat& source_cat = *itSourceCat;

   Cat target_cat = Categories().find(Cat(func_.target)) == Categories().end() ? Cat(func_.target) : *Categories().find(Cat(func_.target));

   EraseCategory(target_cat);

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
      auto objs = MapObject(func_, morph.source);
      auto objt = MapObject(func_, morph.target);

      if (!target_cat.MatchMorphism(objs.value(), objt.value()))
      {
         target_cat.AddMorphism(Morph(objs.value(), objt.value()));
      }
   }

   AddCategory(target_cat);

   return true;
}

//-----------------------------------------------------------------------------------------
std::optional<Func> CACat::MatchFunctor(const Cat::CatName& source_, const Cat::CatName& target_) const
{
   for (const Func& func : m_funcs)
   {
      if (func.source == source_ && func.target == target_)
         return std::optional<Func>(func);
   }

   return std::optional<Func>();
}
