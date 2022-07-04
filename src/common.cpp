#include "common.h"

#include <fstream>
#include <sstream>

#include "str_utils.h"

// Keywords
// Category
static const char* const sCat = "cat";
// Object
static const char* const sObj = "obj";
// Any
static const char* const sAny = "*";
// Comment
static const char* const sComment = "--";
// Expressions type
static const char* const sStatement = "statement";
static const char* const sProof     = "proof";

using namespace cat;

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
   StringVec subsections = split(string_, "::");
   for (auto& str : subsections)
      str = trim_right(trim_left(str, ' '), ' ');
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_morphism_objects(const StringPair& pair_)
{
   StringVec ret = split(pair_.second, "->", false);
   for (auto& str : ret)
      str = trim_right(trim_left(str, ' '), ' ');
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
}

//-----------------------------------------------------------------------------------------
static std::optional<StringPair> get_functor_sections(const std::string& string_)
{
   StringVec subsections = split(string_, "::");
   for (auto& str : subsections)
      str = trim_right(trim_left(str, ' '), ' ');
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_functor_categories(const StringPair& pair_)
{
   StringVec ret = split(pair_.second, "=>", false);
   for (auto& str : ret)
      str = trim_right(trim_left(str, ' '), ' ');
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
}

//-----------------------------------------------------------------------------------------
bool parse_source(const std::string& source_, CACat& ccat_)
{
   enum class ECurrentEntity
   {
         eCategory
      ,  eFunctor
      ,  eNone
   }
   process_entity { ECurrentEntity::eNone };

   enum class EExpType
   {
         eStatement
      ,  eProof
   }
   expr_type { EExpType::eProof };

   std::optional<Cat > crt_cat;
   std::optional<Func> crt_func;

   std::string preprocessed = source_;

   // Get rid of tabs
   std::replace(preprocessed.begin(), preprocessed.end(), '\t', ' ');

   // Get rid of win eol's
   size_t eol_ind {};
   do
   {
      eol_ind = preprocessed.find("\r\n", eol_ind);
      if (eol_ind != -1)
      {
         preprocessed.replace(eol_ind, sizeof("\r\n") - 1, "\n");
      }
   }
   while (eol_ind != -1);

   auto lines = split(preprocessed, '\n');

   for (const std::string& iline : lines)
   {
      if (iline.empty())
         continue;

      std::string line = trim_left(trim_right(iline, ' '), ' ');

      StringVec sections;

      if (line == sStatement)
      {
         if (crt_cat)
            ccat_.AddCategory(crt_cat.value());
         crt_cat.reset();

         if (crt_func)
         {
            Func& func = crt_func.value();

            if (func.morphisms.empty())
            {
               print_error("Morphisms are not defined in functor: " + func.name);
               return false;
            }

            if (expr_type == EExpType::eProof)
            {
               if (!ccat_.Proof(func))
               {
                  print_error("Incorrect functor: " + func.name);
                  return false;
               }
            }
            else if (expr_type == EExpType::eStatement)
               if (!ccat_.Statement(func))
                  return false;

            ccat_.AddFunctor(func);

            crt_func.reset();
         }

         expr_type = EExpType::eStatement;
         continue;
      }
      else if (line == sProof)
      {
         if (crt_cat)
            ccat_.AddCategory(crt_cat.value());
         crt_cat.reset();

         if (crt_func)
         {
            Func& func = crt_func.value();

            if (func.morphisms.empty())
            {
               print_error("Morphisms are not defined in functor: " + func.name);
               return false;
            }

            if (expr_type == EExpType::eProof)
            {
               if (!ccat_.Proof(func))
               {
                  print_error("Incorrect functor: " + func.name);
                  return false;
               }
            }
            else if (expr_type == EExpType::eStatement)
               if (!ccat_.Statement(func))
                  return false;

            ccat_.AddFunctor(func);

            crt_func.reset();
         }

         expr_type = EExpType::eProof;
         continue;
      }
      else
      {
         sections = split(line, ' ', false);
         if (sections.size() < 2)
         {
            print_error("Invalid record: " + line);
            return false;
         }
      }

      const std::string& head = sections[0];

      size_t tail_start = head.length();
      while (true)
      {
         if (line.at(tail_start) != ' ')
            break;

         tail_start++;
      }

      std::string tail = line.substr(tail_start, line.length());

      // Comment
      if (head == sComment)
      {
         continue;
      }
      // Category
      else if (head == sCat)
      {
         if (crt_cat)
            ccat_.AddCategory(crt_cat.value());
         crt_cat.reset();

         crt_cat.emplace(Cat(tail));

         if (crt_func)
         {
            Func& func = crt_func.value();

            if (func.morphisms.empty())
            {
               print_error("Morphisms are not defined in functor: " + func.name);
               return false;
            }

            if (expr_type == EExpType::eProof)
            {
               if (!ccat_.Proof(func))
               {
                  print_error("Incorrect functor: " + func.name);
                  return false;
               }
            }
            else if (expr_type == EExpType::eStatement)
               if (!ccat_.Statement(func))
                  return false;

            ccat_.AddFunctor(func);

            crt_func.reset();
         }

         process_entity = ECurrentEntity::eCategory;
      }
      // Object
      else if (process_entity == ECurrentEntity::eCategory && head == sObj)
      {
         if (crt_cat)
         {
            StringVec arr_objs = split(tail, ',', false);

            for (auto& itObjName : arr_objs)
            {
               auto objName = trim_left(trim_right(itObjName, ' '), ' ');

               if (!crt_cat.value().AddObject(Obj(objName)))
               {
                  print_error("Failure to add object: " + objName);
                  return false;
               }
            }
         }
         else
         {
            print_error("No category to add object: " + tail);
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
                  std::string morph_name = subsections.first;

                  StringVec& mr_objects = mr_objects_opt.value();

                  Obj source(mr_objects.front());
                  Obj target(mr_objects.back ());

                  const ObjUMap& objs = crt_cat.value().GetObjects();

                  if (morph_name == sAny && source.GetName() == sAny && (target.GetName() == sAny))
                  {
                     for (const auto& [idomain, icodomain] : objs)
                     {
                        for (const auto& [jdomain, jcodomain] : objs)
                        {
                           if (idomain == jdomain)
                              continue;

                           if (!crt_cat.value().AddMorphism(MorphDef(idomain, jdomain)))
                              return false;
                        }
                     }
                  }
                  else
                  {
                     std::string name = morph_name == sAny ? default_morph_name(source, target) : subsections.first;
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

               if (crt_func)
               {
                  Func& func = crt_func.value();

                  if (func.morphisms.empty())
                  {
                     print_error("Morphisms are not defined in functor: " + func.name);
                     return false;
                  }

                  if (expr_type == EExpType::eProof)
                  {
                     if (!ccat_.Proof(func))
                     {
                        print_error("Incorrect functor: " + func.name);
                        return false;
                     }
                  }
                  else if (expr_type == EExpType::eStatement)
                     if (!ccat_.Statement(func))
                        return false;

                  ccat_.AddFunctor(func);

                  crt_func.reset();
               }

               Cat::CatName& source = func_categories.front();
               Cat::CatName& target = func_categories.back ();

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
               else if (mr_objects.size() < 2)
               {
                  print_error("Incorrect functor morphism definition: " + line);
                  return false;
               }

               Obj source(mr_objects.front());
               Obj target(mr_objects.back ());

               if (source.GetName() == sAny && target.GetName() == sAny && subsections.first != sAny)
               {
                  print_error("Incorrect functor morphism definition: " + line);
                  return false;
               }

               if (source.GetName() == sAny && target.GetName() == sAny && subsections.first == sAny)
               {
                  const std::set<Cat>& cats = ccat_.Categories();

                  auto itSourceCat = cats.find(Cat(crt_func.value().source));
                  auto itTargetCat = cats.find(Cat(crt_func.value().target));

                  if (itSourceCat != cats.end() && itTargetCat != cats.end())
                  {
                     const Cat& source_cat = *itSourceCat;
                     const Cat& target_cat = *itTargetCat;

                     const ObjUMap& source_cat_objects = source_cat.GetObjects();
                     const ObjUMap& target_cat_objects = target_cat.GetObjects();

                     for (const auto& [obj, _] : source_cat_objects)
                     {
                        if (target_cat_objects.find(obj) == target_cat_objects.end())
                        {
                           print_error("No such object '" + obj.GetName() + "' in target category '" + target_cat.GetName() + "'");
                           return false;
                        }

                        crt_func.value().morphisms.insert(MorphDef(obj, obj));
                     }
                  }
                  else
                     return false;
               }
               else if (source.GetName() == sAny && subsections.first == sAny)
               {
                  const std::set<Cat>& cats = ccat_.Categories();

                  auto itSourceCat = cats.find(Cat(crt_func.value().source));
                  auto itTargetCat = cats.find(Cat(crt_func.value().target));

                  if (itSourceCat != cats.end() && itTargetCat != cats.end())
                  {
                     const Cat& source_cat = *itSourceCat;
                     const Cat& target_cat = *itTargetCat;

                     const ObjUMap& source_cat_objects = source_cat.GetObjects();
                     const ObjUMap& target_cat_objects = target_cat.GetObjects();

                     for (const auto& [obj, _] : source_cat_objects)
                     {
                        if (target_cat_objects.find(Obj(target.GetName())) == target_cat_objects.end())
                        {
                           print_error("No such object '" + obj.GetName() + "' in target category '" + target_cat.GetName() + "'");
                           return false;
                        }

                        crt_func.value().morphisms.insert(MorphDef(obj, Obj(target.GetName())));
                     }
                  }
                  else
                     return false;
               }
               else
               {
                  std::string name = subsections.first == sAny ? default_morph_name(source, target) : subsections.first;

                  for (const MorphDef& it : crt_func.value().morphisms)
                  {
                     if (it.source == source)
                     {
                        print_error("Morphism with the same source already defined");
                        return false;
                     }
                  }

                  crt_func.value().morphisms.insert(MorphDef(source, target, name));
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
   }

   if (crt_cat)
      ccat_.AddCategory(crt_cat.value());
   crt_cat.reset();

   if (crt_func)
   {
      Func& func = crt_func.value();

      if (func.morphisms.empty())
      {
         print_error("Morphisms are not defined in functor: " + func.name);
         return false;
      }

      if (expr_type == EExpType::eProof)
      {
         if (!ccat_.Proof(func))
         {
            print_error("Incorrect functor: " + func.name);
            return false;
         }
      }
      else if (expr_type == EExpType::eStatement)
         if (!ccat_.Statement(func))
            return false;

      ccat_.AddFunctor(func);

      crt_func.reset();
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool load_source(const std::string& path_, CACat& ccat_)
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
std::optional<std::string> get_description(const std::string& filename_)
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
ObjVec solve_sequence(const Cat& cat_, const Obj& from_, const Obj& to_)
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
std::vector<ObjVec> solve_sequences(const Cat& cat_, const Obj& from_, const Obj& to_)
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
std::vector<MorphDef> map_obj2morphism(const ObjVec& objs_, const Cat& cat_)
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
void solve_compositions(Cat& cat_)
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
std::string id_morph_name(const Obj& obj_)
{
   return default_morph_name(obj_, obj_);
}

//-----------------------------------------------------------------------------------------
std::string default_morph_name(const Obj& source_, const Obj& target_)
{
   return source_.GetName() + "-" + target_.GetName();
}

//-----------------------------------------------------------------------------------------
Func::FuncName default_functor_name(const Cat::CatName& source_, const Cat::CatName& target_)
{
   return source_ + "-" + target_;
}

//-----------------------------------------------------------------------------------------
void inverse(Cat& cat_)
{
   MorphSet morphs = cat_.GetMorphisms();

   cat_.EraseMorphisms();

   for (const MorphDef& morph : morphs)
   {
      std::string name = default_morph_name(morph.source, morph.target) == morph.name ? default_morph_name(morph.target, morph.source) : morph.name;
      cat_.AddMorphism(morph.target, morph.source, name);
   }
}

//-----------------------------------------------------------------------------------------
ObjVec initial(Cat& cat_)
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
CAT_EXPORT ObjVec terminal(Cat& cat_)
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
