#include "cat.h"

#include <fstream>
#include <sstream>

#include "str_utils.h"
#include "common.h"
#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Obj::Obj(const std::string& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
Obj::Obj(const Obj& obj_) : m_name(obj_.m_name) {};

//-----------------------------------------------------------------------------------------
void Obj::operator=(const Obj& obj_)
{
   m_name = obj_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Obj::operator==(const Obj& obj_) const {
   return m_name == obj_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Obj::operator!=(const Obj& obj_) const {
   return m_name != obj_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Obj::operator<(const Obj& obj_) const {
   return m_name < obj_.m_name;
}

//-----------------------------------------------------------------------------------------
const std::string& Obj::GetName() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t ObjKeyHasher::operator()(const Obj& k_) const
{
 return std::hash<std::string>{}(k_.GetName());
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
MorphDef::MorphDef(const Obj& source_, const Obj& target_, const std::string& morph_name_) :
   source      (source_)
 , target      (target_)
 , name        (morph_name_)
{};

//-----------------------------------------------------------------------------------------
MorphDef::MorphDef(const Obj& source_, const Obj& target_) :
   source      (source_)
 , target      (target_)
 , name        (default_morph_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool MorphDef::operator<(const MorphDef& morph_) const
{
   return std::tie(source, target, name) < std::tie(morph_.source, morph_.target, morph_.name);
}

//-----------------------------------------------------------------------------------------
bool MorphDef::operator==(const MorphDef& morph_) const
{
   return source == morph_.source && target == morph_.target && name == morph_.name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Cat::Cat(const CatName& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
bool Cat::operator < (const Cat& cat_) const
{
   return m_name < cat_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Cat::AddMorphism(const Obj& source_, const Obj& target_, const std::string& morph_name_)
{
   if (m_objects.find(source_) == m_objects.end())
   {
      print_error("No such object: " + source_.GetName());
      return false;
   }

   if (m_objects.find(target_) == m_objects.end())
   {
      print_error("No such object: " + target_.GetName());
      return false;
   }

   for (auto & [obj_source, obj_target, morph_name_value] : m_morphisms)
   {
      if (morph_name_ == morph_name_value && (obj_source != source_ || obj_target != target_))
      {
         print_error("Morphism redefinition: " + morph_name_);
         return false;
      }
   }

   m_objects[source_].insert(target_);

   m_morphisms.insert(MorphDef(source_, target_, morph_name_));

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::AddMorphism(const MorphDef& morph_)
{
   if (m_objects.find(morph_.source) == m_objects.end())
   {
      print_error("No such object: " + morph_.source.GetName());
      return false;
   }

   if (m_objects.find(morph_.target) == m_objects.end())
   {
      print_error("No such object: " + morph_.target.GetName());
      return false;
   }

   for (auto & [obj_source, obj_target, morph_name_value] : m_morphisms)
   {
      if (morph_.name == morph_name_value && (obj_source != morph_.source || obj_target != morph_.target))
      {
         print_error("Morphism redefinition: " + morph_.name);
         return false;
      }
   }

   m_objects[morph_.source].insert(morph_.target);

   m_morphisms.insert(morph_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::EraseMorphism(const std::string& morph_name_)
{
   auto it = std::find_if(m_morphisms.begin(), m_morphisms.end(), [&](const MorphSet::value_type& elem_) {
      return elem_.name == morph_name_;
   });

   if (it == m_morphisms.end())
      return false;

   bool isIdentity = (*it).source == (*it).target;

   if (!isIdentity)
      m_morphisms.erase(it);
   else
   {
      // removing identity morphism for missing object only
      bool isObjectPresent = m_objects.find((*it).source) != m_objects.end();
      if (!isObjectPresent)
         m_morphisms.erase(it);
      else
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
void Cat::EraseMorphisms()
{
   m_morphisms.clear();

   for (auto& [obj, objset] : m_objects)
      objset.clear();
}

//-----------------------------------------------------------------------------------------
bool Cat::AddObject(const Obj& obj_)
{
   if (obj_.GetName().empty())
      return false;

   if (m_objects.find(obj_) == m_objects.end())
   {
      m_objects[obj_];
      if (!AddMorphism(obj_, obj_, id_morph_name(obj_)))
         return false;
   }
   else
   {
      print_error("Object redefinition: " + obj_.GetName());
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::EraseObject(const Obj& obj_)
{
   auto it = m_objects.find(obj_);
   if (it != m_objects.end())
   {
      m_objects.erase(it);

      std::vector<MorphSet::iterator> morphs; morphs.reserve(m_morphisms.size());

      for (MorphSet::iterator it = m_morphisms.begin(); it != m_morphisms.end(); ++it)
      {
         if (((*it).source == obj_) || ((*it).target == obj_))
            morphs.push_back(it);
      }

      while (!morphs.empty())
      {
         m_morphisms.erase(morphs.back());
         morphs.pop_back();
      }

      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
void Cat::EraseObjects()
{
   m_objects   .clear();
   m_morphisms .clear();
}

//-----------------------------------------------------------------------------------------
const Cat::CatName& Cat::GetName() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
const MorphSet& Cat::GetMorphisms() const
{
   return m_morphisms;
}

//-----------------------------------------------------------------------------------------
const ObjUMap& Cat::GetObjects() const
{
   return m_objects;
}

//-----------------------------------------------------------------------------------------
bool Cat::MatchMorphism(const Obj& source_, const Obj& target_) const
{
   auto it = m_objects.find(source_);
   if (it == m_objects.end())
      return false;

   const auto& [_, codomain] = *it;

   return codomain.find(target_) != codomain.end();
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool Func::operator < (const Func& func_) const
{
   return std::tuple(source, target, name) < std::tuple(func_.source, func_.target, name);
}

//-----------------------------------------------------------------------------------------
std::optional<Obj> cat::MapObject(const Func& func_, const Obj& obj_)
{
   for (const MorphDef& morph : func_.morphisms)
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
void CACat::AddCategory(const Cat& cat_)
{
   m_cats.insert(cat_);
}

//-----------------------------------------------------------------------------------------
void CACat::AddFunctor(const Func& func_)
{
   m_funcs.insert(func_);
}

//-----------------------------------------------------------------------------------------
const std::set<Cat>& CACat::Categories() const
{
   return m_cats;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
const std::set<Func>& CACat::Functors() const
{
   return m_funcs;
}

//-----------------------------------------------------------------------------------------
bool CACat::Proof(Func& func_) const
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

   for (const MorphDef& morph : source_cat.GetMorphisms())
   {
      auto objs = MapObject(func_, morph.source);
      auto objt = MapObject(func_, morph.target);

      if (!objs)
      {
         print_error("Failure to map object: " + morph.source.GetName());
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map object: " + morph.target.GetName());
         return false;
      }

      // Checking mapping of morphisms
      if (!target_cat.MatchMorphism(objs.value(), objt.value()))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool CACat::Statement(Func& func_)
{
   auto itSourceCat = m_cats.find(Cat(func_.source));
   if (itSourceCat == m_cats.end())
   {
      print_error("Missing source category: " + func_.source);
      return false;
   }

   const Cat& source_cat = *itSourceCat;

   std::optional<Cat> ocat;
   auto itTargetCat = m_cats.find(Cat(func_.target));
   if (itTargetCat != m_cats.end())
      ocat.emplace(*itTargetCat);
   else
      ocat.emplace(Cat(func_.target));

   Cat& target_cat = ocat.value();

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
   for (const MorphDef& morph : source_cat.GetMorphisms())
   {
      auto objs = MapObject(func_, morph.source);
      auto objt = MapObject(func_, morph.target);

      if (!target_cat.MatchMorphism(objs.value(), objt.value()))
      {
         target_cat.AddMorphism(MorphDef(objs.value(), objt.value()));
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
std::optional<Func> CACat::MatchFunctor(const Cat::CatName& source_, const Cat::CatName& target_)
{
   for (const Func& func : m_funcs)
   {
      if (func.source == source_ && func.target == target_)
         return std::optional<Func>(func);
   }

   return std::optional<Func>();
}
