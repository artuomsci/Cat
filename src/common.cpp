#include "common.h"

#include <fstream>
#include <sstream>

#include "str_utils.h"
#include "log.h"

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
static std::string trim_sp(const std::string& string_)
{
   return trim_right(trim_left(string_, ' '), ' ');
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
   StringVec subsections = split(string_, "::");
   for (auto& str : subsections)
      str = trim_sp(str);
   return subsections.size() == 2 ? StringPair(subsections[0], subsections[1]) : std::optional<StringPair>();
}

//-----------------------------------------------------------------------------------------
static std::optional<StringVec> get_morphism_objects(const StringPair& pair_)
{
   StringVec ret = split(pair_.second, "->", false);
   for (auto& str : ret)
      str = trim_sp(str);
   return ret.size() < 2 ? std::optional<StringVec>() : ret;
}

//-----------------------------------------------------------------------------------------
static std::vector<Morph> get_morphisms(const std::string& string_)
{
   StringVec subsections = split(string_, "::");
   if (subsections.size() != 2)
      return std::vector<Morph>();

   for (auto& str : subsections)
      str = trim_sp(str);

   StringVec args = split(subsections[1], "->", false);
   if (args.size() < 2)
      return std::vector<Morph>();

   for (auto& str : args)
      str = trim_sp(str);

   std::vector<Morph> ret; ret.reserve(args.size() - 1);
   for (int i = 0; i < (int)args.size() - 1; ++i)
   {
      Obj source(args[i + 0]);
      Obj target(args[i + 1]);
      ret.push_back(Morph(source, target));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
static std::optional<Func> get_functor(const std::string& string_)
{
   StringVec subsections = split(string_, "::");
   if (subsections.size() != 2)
      return std::optional<Func>();

   for (auto& str : subsections)
      str = trim_sp(str);

   StringVec args = split(subsections[1], "=>", false);
   if (args.size() != 2)
      return std::optional<Func>();

   for (auto& str : args)
      str = trim_sp(str);

   return Func(args[0], args[1], subsections[0]);
}

//-----------------------------------------------------------------------------------------
static std::string conform(std::string string_)
{
   // Get rid of tabs
   std::replace(string_.begin(), string_.end(), '\t', ' ');

   // Get rid of win eol's
   size_t eol_ind {};
   do
   {
      eol_ind = string_.find("\r\n", eol_ind);
      if (eol_ind != -1)
      {
         string_.replace(eol_ind, sizeof("\r\n") - 1, "\n");
      }
   }
   while (eol_ind != -1);

   return string_;
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

   // Finalize functor creation
   auto fnCreateFunctor = [](CACat& ccat_, Func& func_, EExpType type_)
   {
      if (func_.morphisms.empty())
      {
         print_error("Morphisms are not defined in functor: " + func_.name);
         return false;
      }

      if (type_ == EExpType::eProof)
      {
         if (!ccat_.Proof(func_))
            return false;
      }
      else if (type_ == EExpType::eStatement)
      {
         if (!ccat_.Statement(func_))
            return false;
      }

      ccat_.AddFunctor(func_);

      return true;
   };

   std::optional<Cat > crt_cat;
   std::optional<Func> crt_func;

   auto fnStateSwitch = [&]()
   {
      if (crt_cat)
         ccat_.AddCategory(crt_cat.value());
      crt_cat.reset();

      if (crt_func)
      {
         if (!fnCreateFunctor(ccat_, crt_func.value(), expr_type))
            return false;

         crt_func.reset();
      }

      return true;
   };

   for (const std::string& iline : split(conform(source_), '\n'))
   {
      if (iline.empty())
         continue;

      std::string line = trim_sp(iline);

      StringVec sections;

      if (line == sStatement)
      {
         if (!fnStateSwitch())
            return false;

         expr_type = EExpType::eStatement;
         continue;
      }
      else if (line == sProof)
      {
         if (!fnStateSwitch())
            return false;

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
         if (!fnStateSwitch())
            return false;

         crt_cat.emplace(Cat(tail));

         process_entity = ECurrentEntity::eCategory;
      }
      // Object
      else if (process_entity == ECurrentEntity::eCategory && head == sObj)
      {
         if (!crt_cat)
         {
            print_error("No category to add object: " + tail);
            return false;
         }

         StringVec arr_objs = split(tail, ',', false);

         for (auto& itObjName : arr_objs)
         {
            auto objName = trim_sp(itObjName);

            if (!crt_cat.value().AddObject(Obj(objName)))
            {
               print_error("Failure to add object: " + objName);
               return false;
            }
         }
      }
      // Morphism
      else if (process_entity == ECurrentEntity::eCategory && is_morphism(line))
      {
         if (!crt_cat)
         {
            print_error("No category to add morphism: " + line);
            return false;
         }

         std::vector<Morph> morphs = get_morphisms(line);
         if (morphs.empty())
         {
            print_error("Error in morphism definition: " + line);
            return false;
         }

         if (morphs.front().name == sAny && morphs.front().source.GetName() == sAny && (morphs.front().target.GetName() == sAny))
         {
            const ObjUMap& objs = crt_cat.value().GetObjects();

            for (const auto& [idomain, icodomain] : objs)
            {
               for (const auto& [jdomain, jcodomain] : objs)
               {
                  if (idomain == jdomain)
                     continue;

                  if (!crt_cat.value().AddMorphism(Morph(idomain, jdomain)))
                     return false;
               }
            }
         }
         else
         {
            for (const Morph& morph : morphs)
            {
               if (!crt_cat.value().AddMorphism(morph))
                  return false;
            }
         }
      }
      // Functor
      else if (is_functor(line))
      {
         process_entity = ECurrentEntity::eFunctor;

         std::optional<Func> oFn = get_functor(line);
         if (!oFn)
         {
            print_error("Error in functor definition: " + line);
            return false;
         }

         Func& fn = oFn.value();

         if (!fnStateSwitch())
            return false;

         if (expr_type == EExpType::eProof)
         {
            if (ccat_.Categories().find(Cat(fn.target)) == ccat_.Categories().end())
            {
               print_error("No such category: " + fn.target);
               return false;
            }
         }
         else if (expr_type == EExpType::eStatement)
         {
            if (ccat_.Categories().find(Cat(fn.target)) == ccat_.Categories().end())
            {
               ccat_.AddCategory(Cat(fn.target));
            }
         }

         std::string name = fn.name == sAny ? default_functor_name(fn.source, fn.target) : fn.name;
         crt_func.emplace(Func(fn.source, fn.target, name));
      }
      // Functor morphism
      else if (process_entity == ECurrentEntity::eFunctor && is_morphism(line))
      {
         auto subsections_opt = get_morphism_sections(line);
         if (!subsections_opt)
         {
            print_error("Error in morphism definition: " + line);
            return false;
         }

         StringPair& subsections = subsections_opt.value();

         auto mr_objects_opt = get_morphism_objects(subsections);
         if (!mr_objects_opt)
         {
            print_error("Error in morphism definition: " + line);
            return false;
         }

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

            if (itSourceCat == cats.end() || itTargetCat == cats.end())
               return false;

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

               crt_func.value().morphisms.insert(Morph(obj, obj));
            }
         }
         else if (source.GetName() == sAny && subsections.first == sAny)
         {
            const std::set<Cat>& cats = ccat_.Categories();

            auto itSourceCat = cats.find(Cat(crt_func.value().source));
            auto itTargetCat = cats.find(Cat(crt_func.value().target));

            if (itSourceCat == cats.end() || itTargetCat == cats.end())
               return false;

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

               crt_func.value().morphisms.insert(Morph(obj, Obj(target.GetName())));
            }
         }
         else
         {
            for (const Morph& it : crt_func.value().morphisms)
            {
               if (it.source == source)
               {
                  print_error("Morphism with the same source already defined");
                  return false;
               }
            }

            std::string name = subsections.first == sAny ? default_morph_name(source, target) : subsections.first;
            crt_func.value().morphisms.insert(Morph(source, target, name));
         }
      }
      else
      {
         print_error(line);
         return false;
      }
   }

   if (!fnStateSwitch())
      return false;

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
std::vector<Morph> map_obj2morphism(const ObjVec& objs_, const Cat& cat_)
{
   std::vector<Morph> ret;

   const MorphSet& morphisms = cat_.GetMorphisms();

   for (int i = 0; i < (int)objs_.size() - 1; ++i)
   {
      auto it = std::find_if(morphisms.begin(), morphisms.end(), [&](const MorphSet::value_type& elem_)
      {
         return objs_[i + 0] == elem_.source && objs_[i + 1] == elem_.target;
      });

      ret.push_back(it != morphisms.end() ? *it : Morph(Obj(""), Obj("")));
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
         cat_.AddMorphism(Morph(domain, codomain_obj));
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

   for (const Morph& morph : morphs)
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
