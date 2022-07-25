#include "cat.h"

#include <algorithm>

#include "common.h"
#include "log.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Morph::Morph(const Obj& source_, const Obj& target_, const std::string& morph_name_) :
   source      (source_)
 , target      (target_)
 , name        (morph_name_)
{};

//-----------------------------------------------------------------------------------------
Morph::Morph(const Obj& source_, const Obj& target_) :
   source      (source_)
 , target      (target_)
 , name        (default_morph_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool Morph::operator<(const Morph& morph_) const
{
   return std::tie(source, target, name) < std::tie(morph_.source, morph_.target, morph_.name);
}

//-----------------------------------------------------------------------------------------
bool Morph::operator==(const Morph& morph_) const
{
   return source == morph_.source && target == morph_.target && name == morph_.name;
}

//-----------------------------------------------------------------------------------------
bool Morph::operator!=(const Morph& morph_) const
{
   return source != morph_.source || target != morph_.target || name != morph_.name;
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
bool Cat::operator ==(const Cat& cat_) const
{
   return m_name == cat_.GetName();
}

//-----------------------------------------------------------------------------------------
bool Cat::operator !=(const Cat& cat_) const
{
   return m_name != cat_.GetName();
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

   m_morphisms.emplace_back(source_, target_, morph_name_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::AddMorphism(const Morph& morph_)
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
      if (morph_.name == morph_name_value && obj_target != morph_.target)
      {
         print_error("Morphism redefinition: " + morph_.name);
         return false;
      }
   }

   m_objects[morph_.source].insert(morph_.target);

   m_morphisms.push_back(morph_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::EraseMorphism(const std::string& morph_name_)
{
   for (Morph& morph : m_morphisms)
   {
      if (morph.name == morph_name_)
      {
         auto it_obj = m_objects.find(morph.source);
         if (it_obj != m_objects.end())
         {
            if (morph.source == morph.target)
               return false;

            auto& [_, obj_set] = *it_obj;

            obj_set.erase(morph.target);
         }
      }
   }

   auto it_begin = std::remove_if(m_morphisms.begin(), m_morphisms.end(), [&](const MorphVec::value_type& element_)
      {
         return element_.name == morph_name_;
      });

   if (it_begin == m_morphisms.end())
      return false;

   m_morphisms.erase(it_begin, m_morphisms.end());

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

      std::vector<MorphVec::iterator> morphs; morphs.reserve(m_morphisms.size());

      for (MorphVec::iterator it = m_morphisms.begin(); it != m_morphisms.end(); ++it)
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
const MorphVec& Cat::GetMorphisms() const
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
std::size_t CatKeyHasher::operator()(const Cat& k_) const
{
   return std::hash<std::string>{}(k_.GetName());
}
