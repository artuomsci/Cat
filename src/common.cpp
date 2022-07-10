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
template <typename TMapType>
std::set<typename TMapType::key_type> umapk2set(const TMapType& map_)
{
   std::set<typename TMapType::key_type> ret;
   for (const auto& it : map_)
      ret.insert(it.first);
   return ret;
}

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
enum class EExpType
{
      eStatement
   ,  eProof
};

//-----------------------------------------------------------------------------------------
template <typename TNode, typename TContainer, typename TLink>
static std::vector<TLink> get_chains(const std::string& name_, const TNode& source_, const TNode& target_, const TContainer& domain_, const TContainer& codomain_, EExpType expr_type_)
{
   std::vector<TLink> ret;

   auto fnCheckSource = [&]()
   {
      if (domain_.find(source_) == domain_.end())
      {
         print_error("No such source: " + source_.GetName());
         return false;
      }
      return true;
   };

   auto fnCheckTarget = [&]()
   {
      if (expr_type_ == EExpType::eStatement)
         return true;

      if (codomain_.find(target_) == codomain_.end())
      {
         print_error("No such target: " + target_.GetName());
         return false;
      }
      return true;
   };

   // f :: a -> b
   if       (name_ != sAny && source_.GetName() != sAny && target_.GetName() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(TLink(source_, target_, name_));
   }
   // * :: a -> b
   else if  (name_ == sAny && source_.GetName() != sAny && target_.GetName() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(TLink(source_, target_));
   }
   // * :: * -> *
   else if (name_ == sAny && source_.GetName() == sAny && target_.GetName() == sAny)
   {
      for (const auto& dnode : domain_)
      {
         for (const auto& cnode : codomain_)
         {
            ret.push_back(TLink(dnode, cnode));
         }
      }
   }
   // * :: * -> b
   else if (name_ == sAny && source_.GetName() == sAny && target_.GetName() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(TLink(dnode, target_));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.GetName() != sAny && target_.GetName() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& cnode : codomain_)
         ret.push_back(TLink(source_, cnode));
   }
   // f :: a -> *
   else if (name_ != sAny && source_.GetName() != sAny && target_.GetName() == sAny)
   {
      print_error("Incorrect definition");
   }
   // f :: * -> b
   else if (name_ != sAny && source_.GetName() == sAny && target_.GetName() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(TLink(dnode, target_, name_));
   }
   // f :: * -> *
   else if (name_ != sAny && source_.GetName() == sAny && target_.GetName() == sAny)
   {
      print_error("Incorrect definition");
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
template <typename T>
std::string chain_symbol();

template <> std::string chain_symbol<Morph>() { return "->"; }
template <> std::string chain_symbol<Func >() { return "=>"; }

//-----------------------------------------------------------------------------------------
template <typename TLink, typename TNode, typename TContainer>
static std::vector<TLink> get_chain(const std::string& line_, const TContainer& domain_, const TContainer& codomain_, EExpType expr_type_)
{
   StringVec subsections = split(line_, "::");
   if (subsections.size() != 2)
      return std::vector<TLink>();

   for (auto& str : subsections)
      str = trim_sp(str);

   const std::string& head = subsections[0];
   const std::string& tail = subsections[1];

   StringVec args = split(tail, chain_symbol<TLink>(), false);
   if (args.size() < 2)
      return std::vector<TLink>();

   for (auto& str : args)
      str = trim_sp(str);

   std::vector<TLink> ret; ret.reserve(args.size() - 1);
   for (int i = 0; i < (int)args.size() - 1; ++i)
   {
      TNode source(args[i + 0]);
      TNode target(args[i + 1]);

      for (const auto& it : get_chains<TNode, TContainer, TLink>(head, source, target, domain_, codomain_, expr_type_))
         ret.push_back(it);
   }

   return ret;
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


   EExpType expr_type { EExpType::eProof };

   // Finalize functor creation
   auto fnCreateFunctor = [](CACat& ccat_, Func& func_, EExpType type_)
   {
      if (type_ == EExpType::eProof)
      {
         if (!ccat_.Proof(func_))
            return false;
      }
      else if (type_ == EExpType::eStatement)
      {
         if (func_.morphisms.empty())
         {
            auto itSourceCat = ccat_.Categories().find(Cat(func_.source));
            if (itSourceCat == ccat_.Categories().end())
            {
               print_error("Missing source category: " + func_.source);
               return false;
            }

            const Cat& source_cat = *itSourceCat;

            for (const auto& [obj, _] : source_cat.GetObjects())
               func_.morphisms.emplace(obj, obj);
         }

         if (!ccat_.Statement(func_))
            return false;
      }

      ccat_.AddFunctor(func_);

      return true;
   };

   std::optional<Cat > crt_cat;
   std::optional<Func> crt_func;

   auto fnBeginCategory = [](const std::string& line_, std::optional<Cat>& crt_cat_)
   {
      crt_cat_.emplace(line_);
   };

   auto fnEndCategory = [](std::optional<Cat>& crt_cat_, CACat& ccat_)
   {
      if (crt_cat_)
         ccat_.AddCategory(crt_cat_.value());
      crt_cat_.reset();

      return true;
   };

   auto fnAddObjects = [](const std::string& line_, std::optional<Cat>& crt_cat_)
   {
      if (!crt_cat_)
      {
         print_error("No category to add object: " + line_);
         return false;
      }

      for (auto& itObjName : split(line_, ',', false))
      {
         auto objName = trim_sp(itObjName);

         if (!crt_cat_.value().AddObject(Obj(objName)))
         {
            print_error("Failure to add object: " + objName);
            return false;
         }
      }

      return true;
   };

   auto fnAddMorphisms = [](const std::string& line_, std::optional<Cat>& crt_cat_, EExpType expr_type_)
   {
      if (!crt_cat_)
      {
         print_error("No category to add morphism: " + line_);
         return false;
      }

      const ObjUMap& objs = crt_cat_.value().GetObjects();

      std::set<Obj> obj_keys = umapk2set<ObjUMap>(objs);

      std::vector<Morph> morphs = get_chain<Morph, Obj>(line_, obj_keys, obj_keys, expr_type_);
      if (morphs.empty())
      {
         print_error("Error in morphism definition: " + line_);
         return false;
      }

      for (const Morph& morph : morphs)
      {
         if (!crt_cat_.value().AddMorphism(morph))
            return false;
      }

      return true;
   };

   auto fnBeginFunctor = [](const std::string& line_, std::optional<Func>& crt_func_, CACat& ccat_, EExpType expr_type_)
   {
      std::vector<Func> funcs = get_chain<Func, Cat>(line_, ccat_.Categories(), ccat_.Categories(), expr_type_);
      if (funcs.size() != 1)
      {
         print_error("Incorrect functor definition: " + line_);
         return false;
      }

      if (expr_type_ == EExpType::eStatement)
      {
         if (ccat_.Categories().find(Cat(funcs.front().target)) == ccat_.Categories().end())
            ccat_.AddCategory(Cat(funcs.front().target));
      }

      crt_func_.emplace(funcs.front());

      return true;
   };

   auto fnEndFunctor = [&]()
   {
      if (crt_func)
      {
         if (!fnCreateFunctor(ccat_, crt_func.value(), expr_type))
            return false;

         crt_func.reset();
      }

      return true;
   };

   auto fnAddFMorphisms = [](const std::string& line_, std::optional<Func>& crt_func_, CACat& ccat_, EExpType expr_type_)
   {
      const std::set<Cat>& cats = ccat_.Categories();

      auto itSourceCat = cats.find(Cat(crt_func_.value().source));
      auto itTargetCat = cats.find(Cat(crt_func_.value().target));

      std::set<Obj> source_keys = umapk2set<ObjUMap>((*itSourceCat).GetObjects());
      std::set<Obj> target_keys = umapk2set<ObjUMap>((*itTargetCat).GetObjects());

      std::vector<Morph> morphs = get_chain<Morph, Obj>(line_, source_keys, target_keys, expr_type_);
      if (morphs.empty())
      {
         print_error("Error in morphism definition: " + line_);
         return false;
      }

      for (const Morph& morph : morphs)
         crt_func_.value().morphisms.insert(morph);

     return true;
   };

   auto fnStateSwitch = [&]()
   {
      return fnEndCategory(crt_cat, ccat_) && fnEndFunctor();
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

         fnBeginCategory(tail, crt_cat);

         process_entity = ECurrentEntity::eCategory;
      }
      // Object
      else if (process_entity == ECurrentEntity::eCategory && head == sObj)
      {
         if (!fnAddObjects(tail, crt_cat))
            return false;
      }
      // Morphism
      else if (process_entity == ECurrentEntity::eCategory && is_morphism(line))
      {
         if (!fnAddMorphisms(line, crt_cat, expr_type))
            return false;
      }
      // Functor
      else if (is_functor(line))
      {
         process_entity = ECurrentEntity::eFunctor;

         if (!fnStateSwitch())
            return false;

         if (!fnBeginFunctor(line, crt_func, ccat_, expr_type))
            return false;
      }
      // Functor morphism
      else if (process_entity == ECurrentEntity::eFunctor && is_morphism(line))
      {
         if (!fnAddFMorphisms(line, crt_func, ccat_, expr_type))
            return false;
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
