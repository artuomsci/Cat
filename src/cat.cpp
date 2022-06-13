#include "cat.h"

#include <fstream>
#include <sstream>

#include "str_utils.h"

using namespace cat;

// Keywords
static const char* const sObj = "obj";
static const char* const sCat = "cat";

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
MorphDef::MorphDef(const Obj& start_, const Obj& end_, const std::string& morph_name_) :
   start       (start_)
 , end         (end_)
 , morph_name  (morph_name_)
{};

//-----------------------------------------------------------------------------------------
MorphDef::MorphDef(const Obj& start_, const Obj& end_) :
   start       (start_)
 , end         (end_)
 , morph_name  (default_morph_name(start, end))
{};

//-----------------------------------------------------------------------------------------
bool MorphDef::operator<(const MorphDef& morph_) const
{
   return std::tie(start, end, morph_name) < std::tie(morph_.start, morph_.end, morph_.morph_name);
}

//-----------------------------------------------------------------------------------------
bool MorphDef::operator==(const MorphDef& morph_) const
{
   return start == morph_.start && end == morph_.end && morph_name == morph_.morph_name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Cat::Cat(const std::string& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
bool Cat::AddMorphism(const Obj& start_, const Obj& end_, const std::string& morph_name_)
{
   if (m_objects.find(start_) == m_objects.end())
   {
      print_error("No such object: " + start_.GetName());
      return false;
   }

   if (m_objects.find(end_) == m_objects.end())
   {
      print_error("No such object: " + end_.GetName());
      return false;
   }

   for (auto & [obj_start, obj_end, morph_name_value] : m_morphisms)
   {
      if (morph_name_ == morph_name_value && (obj_start != start_ || obj_end != end_))
      {
         print_error("Morphism redefinition: " + morph_name_);
         return false;
      }
   }

   m_objects[start_].insert(end_);

   m_morphisms.insert(MorphDef(start_, end_, morph_name_));

   return true;
}

//-----------------------------------------------------------------------------------------
bool Cat::AddMorphism(const MorphDef& morph_)
{
   if (m_objects.find(morph_.start) == m_objects.end())
   {
      print_error("No such object: " + morph_.start.GetName());
      return false;
   }

   if (m_objects.find(morph_.end) == m_objects.end())
   {
      print_error("No such object: " + morph_.end.GetName());
      return false;
   }

   for (auto & [obj_start, obj_end, morph_name_value] : m_morphisms)
   {
      if (morph_.morph_name == morph_name_value && (obj_start != morph_.start || obj_end != morph_.end))
      {
         print_error("Morphism redefinition: " + morph_.morph_name);
         return false;
      }
   }

   m_objects[morph_.start].insert(morph_.end);

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

   bool isIdentity = (*it).start == (*it).end;

   if (!isIdentity)
      m_morphisms.erase(it);
   else
   {
      // removing identity morphism for missing object only
      bool isObjectPresent = m_objects.find((*it).start) != m_objects.end();
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
         if (((*it).start == obj_) || ((*it).end == obj_))
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
const std::string& Cat::GetName() const
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
static std::optional<StringPair> get_morphism_sections(const std::string& string_)
{
   StringVec subsections = split(remove(string_, ' '), "::");
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_morphism_objects(const std::string& string_)
{
   StringVec ret = split(string_, "->", false);
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
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
bool cat::parse_source(const std::string& source_, std::vector<Cat>& cats_)
{
   ObjUMap objMap;

   auto lines = split(source_, '\n');
   for (const std::string& line : lines)
   {
      auto sections = split(line, ' ', false);
      if (sections.size() < 2)
         continue;

      const std::string& col0 = remove(sections[0], ' ');
      const std::string& col1 = remove(sections[1], ' ');

      // Comment
      if (is_comment(col0))
      {
      }
      // Category
      else if (col0 == sCat)
      {
         cats_.push_back(Cat(col1));
      }
      // Object
      else if (col0 == sObj)
      {
         if (!cats_.empty())
         {
            for (int i = 1; i < (int)sections.size(); ++i)
            {
               StringVec arr_objs = split(remove(sections[i], ' '), ',', false);

               for (auto& itObjName : arr_objs)
                  cats_.back().AddObject(Obj(itObjName));
            }
         }
         else
         {
            print_error("No category to add object: " + col1);
            return false;
         }
      }
      // Morphism
      else if (is_morphism(line))
      {
         if (!cats_.empty())
         {
            auto subsections_opt = get_morphism_sections(line);
            if (subsections_opt)
            {
               StringPair& subsections = subsections_opt.value();

               if (auto mr_objects_opt = get_morphism_objects(subsections.second))
               {
                  StringVec& mr_objects = mr_objects_opt.value();

                  if (!cats_.back().AddMorphism(Obj(mr_objects.front()), Obj(mr_objects.back()), subsections.first))
                     return false;

                  if (mr_objects.size() > 2)
                  {
                     for (int i = 0; i < mr_objects.size() - 1; ++i)
                     {
                        Obj start(mr_objects[i + 0]);
                        Obj end  (mr_objects[i + 1]);

                        if (!cats_.back().AddMorphism(start, end, default_morph_name(start, end)))
                           return false;
                     }
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
            print_error("No category to add morphism: " + line);
            return false;
         }
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool cat::load_source(const std::string& path_, std::vector<Cat>& cats_)
{
   std::ifstream input(path_);
   if (input.is_open())
   {
      std::stringstream descr;
      descr << input.rdbuf();

      input.close();

      return parse_source(descr.str(), cats_);
   }

   return false;
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
         return objs_[i + 0] == elem_.start && objs_[i + 1] == elem_.end;
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
   return obj_.GetName() + obj_.GetName();
}

//-----------------------------------------------------------------------------------------
std::string cat::default_morph_name(const Obj& start_, const Obj& end_)
{
   return start_.GetName() + end_.GetName();
}

//-----------------------------------------------------------------------------------------
void cat::inverse(Cat& cat_)
{
   MorphSet morphs = cat_.GetMorphisms();

   cat_.EraseMorphisms();

   for (const MorphDef& morph : morphs)
   {
      std::string name = default_morph_name(morph.start, morph.end) == morph.morph_name ? default_morph_name(morph.end, morph.start) : morph.morph_name;
      cat_.AddMorphism(morph.end, morph.start, name);
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
