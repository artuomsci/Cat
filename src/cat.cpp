#include "cat.h"

#include <fstream>
#include <sstream>

#include "str_utils.h"

using namespace cat;

// Keywords
// Category
static const char* const sCat = "cat";
// Object
static const char* const sObj = "obj";
// Any
static const char* const sAny = "*";

ELogMode g_log_mode { ELogMode::eConsole };

//-----------------------------------------------------------------------------------------
void cat::set_log_mode(ELogMode mode_)
{
   g_log_mode = mode_;
}

//-----------------------------------------------------------------------------------------
ELogMode cat::get_log_mode()
{
   return g_log_mode;
}

//-----------------------------------------------------------------------------------------
void cat::print_error(const std::string& msg_)
{
   if (g_log_mode == ELogMode::eConsole)
   {
      printf("Error: %s\n", msg_.c_str());
   }
}

//-----------------------------------------------------------------------------------------
void cat::print_info(const std::string& msg_)
{
   if (g_log_mode == ELogMode::eConsole)
   {
      printf("Info: %s\n", msg_.c_str());
   }
}

//-----------------------------------------------------------------------------------------
Obj::Obj(const std::string& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
Obj::Obj(const Obj& obj_) : m_name(obj_.m_name) {};

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
 , morph_name  (morph_name_)
{};

//-----------------------------------------------------------------------------------------
MorphDef::MorphDef(const Obj& source_, const Obj& target_) :
   source      (source_)
 , target      (target_)
 , morph_name  (default_morph_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool MorphDef::operator<(const MorphDef& morph_) const
{
   return std::tie(source, target, morph_name) < std::tie(morph_.source, morph_.target, morph_.morph_name);
}

//-----------------------------------------------------------------------------------------
bool MorphDef::operator==(const MorphDef& morph_) const
{
   return source == morph_.source && target == morph_.target && morph_name == morph_.morph_name;
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
      if (morph_.morph_name == morph_name_value && (obj_source != morph_.source || obj_target != morph_.target))
      {
         print_error("Morphism redefinition: " + morph_.morph_name);
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
      return elem_.morph_name == morph_name_;
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
static bool is_comment(const std::string& string_)
{
   return string_.find("--") == 0;
}

//-----------------------------------------------------------------------------------------
static bool is_morphism(const std::string& string_)
{
   return (string_.find("::") != -1) && (string_.find("->") != -1);
}

//-----------------------------------------------------------------------------------------
static bool is_functor(const std::string& string_)
{
   return (string_.find("::") != -1) && (string_.find("=>") != -1);
}

//-----------------------------------------------------------------------------------------
static std::optional<StringPair> get_morphism_sections(const std::string& string_)
{
   StringVec subsections = split(remove(string_, ' '), "::");
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_morphism_objects(const StringPair& pair_)
{
   StringVec ret = split(pair_.second, "->", false);
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
}

//-----------------------------------------------------------------------------------------
static std::optional<StringPair> get_functor_sections(const std::string& string_)
{
   StringVec subsections = split(remove(string_, ' '), "::");
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_functor_categories(const StringPair& pair_)
{
   StringVec ret = split(pair_.second, "=>", false);
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool Func::operator < (const Func& func_) const
{
   return std::tuple(source, target) < std::tuple(func_.source, func_.target);
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
std::optional<Obj> CACat::MapObject(const Func& func_, const Obj& obj_) const
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
bool CACat::Validate(Func& func_) const
{
   auto itSourceCat = m_cats.find(Cat(func_.source));
   auto itTargetCat = m_cats.find(Cat(func_.target));

   if (itSourceCat != m_cats.end() && itTargetCat != m_cats.end())
   {
      const Cat& source_cat = *itSourceCat;
      const Cat& target_cat = *itTargetCat;

      for (const MorphDef& morph : source_cat.GetMorphisms())
      {
         auto objs = MapObject(func_, morph.source);
         auto objt = MapObject(func_, morph.target);

         if (objs && objt)
         {
            if (!target_cat.MatchMorphism(objs.value(), objt.value()))
               return false;
         }
         else
            return false;
      }
   }
   else
      return false;

   return true;
}

//-----------------------------------------------------------------------------------------
static bool fill_functor_morphisms(Func& func_, CACat& ccat_)
{
   const std::set<Cat>& cats = ccat_.Categories();

   auto itSourceCat = cats.find(Cat(func_.source));
   auto itTargetCat = cats.find(Cat(func_.target));

   if (itSourceCat != cats.end() && itTargetCat != cats.end())
   {
      const Cat& source_cat = *itSourceCat;
      const Cat& target_cat = *itTargetCat;

      for (const auto& [obj, _] : source_cat.GetObjects())
      {
         if (target_cat.GetObjects().find(obj) == target_cat.GetObjects().end())
         {
            print_error("No such object '" + obj.GetName() + "' in target category '" + target_cat.GetName() + "'");
            return false;
         }

         func_.morphisms.insert(MorphDef(obj, obj));
      }
   }
   else
      return false;

   return true;
}

//-----------------------------------------------------------------------------------------
bool cat::parse_source(const std::string& source_, CACat& ccat_)
{
   enum class ECurrentEntity
   {
         eCategory
      ,  eFunctor
      ,  eNone
   };

   ECurrentEntity process_entity { ECurrentEntity::eNone };

   ObjUMap objMap;

   std::optional<Cat> crt_cat;
   std::optional<Func> crt_func;

   // Determine end of line type
   auto eol = source_.find("\r\n") == -1 ? "\n" : "\r\n";

   auto lines = split(source_, eol);

   for (std::string& line : lines)
      std::replace(line.begin(), line.end(), '\t', ' ');

   for (const std::string& line : lines)
   {
      if (line.empty())
         continue;

      auto sections = split(line, ' ', false);
      if (sections.size() < 2)
      {
         print_error("Invalid record: " + line);
         return false;
      }

      std::string col0 = remove(sections[0], ' ');
      std::string col1 = remove(sections[1], ' ');

      // Comment
      if (is_comment(col0))
      {
         continue;
      }
      // Category
      else if (col0 == sCat)
      {
         if (crt_cat)
            ccat_.AddCategory(crt_cat.value());
         crt_cat.reset();

         crt_cat.emplace(Cat(col1));

         if (crt_func)
         {
            if (!fill_functor_morphisms(crt_func.value(), ccat_))
               return false;

            if (!ccat_.Validate(crt_func.value()))
            {
               print_error("Incorrect functor: " + crt_func.value().name);
               return false;
            }

            ccat_.AddFunctor(crt_func.value());

            crt_func.reset();
         }

         process_entity = ECurrentEntity::eCategory;
      }
      // Object
      else if (process_entity == ECurrentEntity::eCategory && col0 == sObj)
      {
         if (crt_cat)
         {
            for (int i = 1; i < (int)sections.size(); ++i)
            {
               StringVec arr_objs = split(remove(sections[i], ' '), ',', false);

               for (auto& itObjName : arr_objs)
               {
                  if (!crt_cat.value().AddObject(Obj(itObjName)))
                  {
                     print_error("Failure to add object: " + itObjName);
                     return false;
                  }
               }
            }
         }
         else
         {
            print_error("No category to add object: " + col1);
            return false;
         }
      }
      // Morphism
      else if (process_entity == ECurrentEntity::eCategory && is_morphism(line))
      {
         if (crt_cat)
         {
            if (auto subsections_opt = get_morphism_sections(line))
            {
               StringPair& subsections = subsections_opt.value();

               if (auto mr_objects_opt = get_morphism_objects(subsections))
               {
                  StringVec& mr_objects = mr_objects_opt.value();

                  Obj source(mr_objects.front());
                  Obj target(mr_objects.back ());

                  std::string name = subsections.first == sAny ? default_morph_name(source, target) : subsections.first;
                  if (!crt_cat.value().AddMorphism(Obj(source), Obj(target), name))
                     return false;

                  // In case of a chain of morphisms
                  if (mr_objects.size() > 2)
                  {
                     for (int i = 0; i < mr_objects.size() - 1; ++i)
                     {
                        Obj source(mr_objects[i + 0]);
                        Obj target(mr_objects[i + 1]);

                        if (!crt_cat.value().AddMorphism(source, target, default_morph_name(source, target)))
                           return false;
                     }
                  }
               }
               else
               {
                  print_error("Error in morphism definition: " + line);
                  return false;
               }
            }
            else
            {
               print_error("Error in morphism definition: " + line);
               return false;
            }
         }
         else
         {
            print_error("No category to add morphism: " + line);
            return false;
         }
      }
      // Functor
      else if (is_functor(line))
      {
         process_entity = ECurrentEntity::eFunctor;

         if (crt_cat)
            ccat_.AddCategory(crt_cat.value());
         crt_cat.reset();

         if (auto subsections_opt = get_functor_sections(line))
         {
            StringPair& subsections = subsections_opt.value();

            if (auto func_categories_opt = get_functor_categories(subsections))
            {
               StringVec& func_categories = func_categories_opt.value();
               if (func_categories.size() > 2)
               {
                  print_error("Functor definition can only have two end points: " + line);
                  return false;
               }

               Cat::CatName& source = func_categories.front();
               Cat::CatName& target = func_categories.back ();

               if (ccat_.Categories().find(Cat(source)) == ccat_.Categories().end())
               {
                  print_error("No such source category: " + source);
                  return false;
               }

               if (ccat_.Categories().find(Cat(target)) == ccat_.Categories().end())
               {
                  print_error("No such target category: " + target);
                  return false;
               }

               if (crt_func)
               {
                  if (!fill_functor_morphisms(crt_func.value(), ccat_))
                     return false;

                  if (!ccat_.Validate(crt_func.value()))
                  {
                     print_error("Incorrect functor: " + crt_func.value().name);
                     return false;
                  }

                  ccat_.AddFunctor(crt_func.value());

                  crt_func.reset();
               }

               std::string name = subsections.first == sAny ? default_functor_name(source, target) : subsections.first;
               crt_func.emplace(Func(source, target, name));
            }
            else
            {
               print_error("Error in functor definition: " + line);
               return false;
            }
         }
         else
         {
            print_error("Error in functor definition: " + line);
            return false;
         }
      }
      // Functor morphism
      else if (process_entity == ECurrentEntity::eFunctor && is_morphism(line))
      {
         if (auto subsections_opt = get_morphism_sections(line))
         {
            StringPair& subsections = subsections_opt.value();

            if (auto mr_objects_opt = get_morphism_objects(subsections))
            {
               StringVec& mr_objects = mr_objects_opt.value();
               if (mr_objects.size() > 2)
               {
                  print_error("Functor morphism definition can only have two end points: " + line);
                  return false;
               }

               Obj source(mr_objects.front());
               Obj target(mr_objects.back ());

               if (target.GetName() == sAny)
               {
                  auto it = ccat_.Categories().find(Cat(crt_func.value().target));
                  if (it != ccat_.Categories().end())
                  {
                     const Cat& cat = *it;

                  }
                  else
                  {
                     print_error("No such target category: " + crt_func.value().target);
                     return false;
                  }
               }

               std::string name = subsections.first == sAny ? default_morph_name(source, target) : subsections.first;
               crt_func.value().morphisms.insert(MorphDef(source, target, name));
            }
            else
            {
               print_error("Error in morphism definition: " + line);
               return false;
            }
         }
         else
         {
            print_error("Error in morphism definition: " + line);
            return false;
         }
      }
   }

   if (crt_cat)
      ccat_.AddCategory(crt_cat.value());
   crt_cat.reset();

   if (crt_func)
   {
      if (!fill_functor_morphisms(crt_func.value(), ccat_))
         return false;

      if (!ccat_.Validate(crt_func.value()))
      {
         print_error("Incorrect functor: " + crt_func.value().name);
         return false;
      }

      ccat_.AddFunctor(crt_func.value());

      crt_func.reset();
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool cat::load_source(const std::string& path_, CACat& ccat_)
{
   std::ifstream input(path_);
   if (input.is_open())
   {
      std::stringstream descr;
      descr << input.rdbuf();

      input.close();

      return parse_source(descr.str(), ccat_);
   }

   return false;
}

//-----------------------------------------------------------------------------------------
std::optional<std::string> cat::get_description(const std::string& filename_)
{
   std::stringstream description;

   std::ifstream fdescr(filename_);
   if (fdescr.is_open())
   {
      description << fdescr.rdbuf();
      fdescr.close();

      return description.str();
   }

   return std::optional<std::string>();
}

//-----------------------------------------------------------------------------------------
ObjVec cat::solve_sequence(const Cat& cat_, const Obj& from_, const Obj& to_)
{
   ObjVec ret;

   std::vector<ObjSetPair> stack;

   std::optional<Obj> current_obj(from_);

   while (true)
   {
      // Checking for destination
      if (current_obj.value() == to_)
      {
         ret.reserve(stack.size() + 1);

         for (auto & [obji, objset] : stack)
            ret.push_back(obji);

         ret.push_back(current_obj.value());

         return ret;
      }

      stack.emplace_back(current_obj.value(), cat_.GetObjects().at(current_obj.value()));

      // Remove identity morphism
      stack.back().second.erase(current_obj.value());

      current_obj.reset();

      while (!current_obj.has_value())
      {
         // Trying new set of objects
         ObjSet& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one object forward
         current_obj.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [obj, objset] : stack)
         {
            // Is already visited
            if (obj == current_obj.value())
            {
               current_obj.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::vector<ObjVec> cat::solve_sequences(const Cat& cat_, const Obj& from_, const Obj& to_)
{
   std::vector<ObjVec> ret;

   std::vector<ObjSetPair> stack;

   std::optional<Obj> current_obj(from_);

   while (true)
   {
      // Checking for destination
      if (current_obj.value() == to_)
      {
         ObjVec seq; seq.reserve(stack.size() + 1);

         for (auto & [obji, objset] : stack)
            seq.push_back(obji);

         seq.push_back(current_obj.value());

         ret.push_back(seq);
      }
      else
      {
         // Stacking forward movements
         stack.emplace_back(current_obj.value(), cat_.GetObjects().at(current_obj.value()));

         // Removing identity morphism
         stack.back().second.erase(current_obj.value());
      }

      current_obj.reset();

      while (!current_obj.has_value())
      {
         // Trying new sets of objects
         ObjSet& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one object forward
         current_obj.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [obj, objset] : stack)
         {
            // Is already visited
            if (obj == current_obj.value())
            {
               current_obj.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::vector<MorphDef> cat::map_obj2morphism(const ObjVec& objs_, const Cat& cat_)
{
   std::vector<MorphDef> ret;

   const MorphSet& morphisms = cat_.GetMorphisms();

   for (int i = 0; i < (int)objs_.size() - 1; ++i)
   {
      auto it = std::find_if(morphisms.begin(), morphisms.end(), [&](const MorphSet::value_type& elem_)
      {
         return objs_[i + 0] == elem_.source && objs_[i + 1] == elem_.target;
      });

      ret.push_back(it != morphisms.end() ? *it : MorphDef(Obj(""), Obj("")));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
void cat::solve_compositions(Cat& cat_)
{
   for (const auto & [domain, codomain] : cat_.GetObjects())
   {
      ObjSet traverse = codomain;

      ObjSet new_codomain;
      while (!traverse.empty())
      {
         for (const Obj& it : traverse)
            new_codomain.insert(it);

         ObjSet new_traverse;
         for (const Obj& obj : traverse)
         {
            if (obj == domain)
               continue;

            const ObjSet& sub_codomain = cat_.GetObjects().at(obj);

            for (const Obj& sub_obj : sub_codomain)
            {
               if (new_codomain.find(sub_obj) != new_codomain.end())
                  continue;

               new_traverse.insert(sub_obj);
            }
         }

         traverse = new_traverse;
      }

      ObjSet domain_diff;
      std::set_difference(new_codomain.begin(), new_codomain.end(), codomain.begin(), codomain.end(), std::inserter(domain_diff, domain_diff.begin()));

      for (const auto& codomain_obj : domain_diff)
         cat_.AddMorphism(MorphDef(domain, codomain_obj));
   }
}

//-----------------------------------------------------------------------------------------
std::string cat::id_morph_name(const Obj& obj_)
{
   return default_morph_name(obj_, obj_);
}

//-----------------------------------------------------------------------------------------
std::string cat::default_morph_name(const Obj& source_, const Obj& target_)
{
   return source_.GetName() + "-" + target_.GetName();
}

//-----------------------------------------------------------------------------------------
Func::FuncName cat::default_functor_name(const Cat::CatName& source_, const Cat::CatName& target_)
{
   return source_ + "-" + target_;
}

//-----------------------------------------------------------------------------------------
void cat::inverse(Cat& cat_)
{
   MorphSet morphs = cat_.GetMorphisms();

   cat_.EraseMorphisms();

   for (const MorphDef& morph : morphs)
   {
      std::string name = default_morph_name(morph.source, morph.target) == morph.morph_name ? default_morph_name(morph.target, morph.source) : morph.morph_name;
      cat_.AddMorphism(morph.target, morph.source, name);
   }
}

//-----------------------------------------------------------------------------------------
ObjVec cat::initial(Cat& cat_)
{
   ObjVec ret;

   const ObjUMap& objs = cat_.GetObjects();

   for (const auto& [obj, objset] : objs)
   {
      if (objs.size() == objset.size())
         ret.push_back(obj);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
CAT_EXPORT ObjVec cat::terminal(Cat& cat_)
{
   ObjVec ret;

   for (const auto& [obj, objset] : cat_.GetObjects())
   {
      bool is_terminal { true };

      for (const auto& [obj_int, objset_int] : cat_.GetObjects())
      {
         if (std::find_if(objset_int.begin(), objset_int.end(), [&](const Obj& obj_){ return obj == obj_; }) == objset_int.end())
         {
            is_terminal = false;
            break;
         }
      }

      if (is_terminal)
         ret.push_back(obj);
   }

   return ret;
}
