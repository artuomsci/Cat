#include "common.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <variant>

#include "str_utils.h"
#include "log.h"

// Keywords
// Category
static const char* const sCat = "cat";
// Object
static const char* const sObj = "obj";
// Any entity
static const char* const sAny = "*";
// Comment
static const char* const sComment = "--";
// Import
static const char* const sImport = "import";
// Expressions type
static const char* const sStatement = "statement";
static const char* const sProof     = "proof";

// File extension
static const char* const sExt = ".cat";

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
      for (const auto& [dnode, _d] : domain_)
      {
         for (const auto& [cnode, _c] : codomain_)
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

      for (const auto& [dnode, _] : domain_)
         ret.push_back(TLink(dnode, target_));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.GetName() != sAny && target_.GetName() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& [cnode, _] : codomain_)
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

      for (const auto& [dnode, _] : domain_)
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
bool SParser::parse_source(const std::string& source_, CACat& ccat_)
{
   enum class ECurrentEntity
   {
         eCategory
      ,  eFunctor
      ,  eNone
   }
   process_entity { ECurrentEntity::eNone };


   EExpType expr_type { EExpType::eProof };

   std::optional<Cat > crt_cat;
   std::optional<Func> crt_func;

   auto fnBeginCategory = [](const std::string& line_, std::optional<Cat>& crt_cat_, CACat& ccat_)
   {
      auto it = ccat_.Categories().find(Cat(line_));

      if (it == ccat_.Categories().end())
         crt_cat_.emplace(line_);
      else
      {
         const auto& [cat, _] = *it;

         crt_cat_.emplace(cat);

         ccat_.EraseCategory(cat);
      }
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

      std::vector<Morph> morphs = get_chain<Morph, Obj>(line_, objs, objs, expr_type_);
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
         if (!ccat_.AddFunctor(crt_func.value(), expr_type))
            return false;

         crt_func.reset();
      }

      return true;
   };

   auto fnAddFMorphisms = [](const std::string& line_, std::optional<Func>& crt_func_, CACat& ccat_, EExpType expr_type_)
   {
      const auto& cats = ccat_.Categories();

      auto itSourceCat = cats.find(Cat(crt_func_.value().source));
      auto itTargetCat = cats.find(Cat(crt_func_.value().target));

      std::vector<Morph> morphs = get_chain<Morph, Obj>(line_, (*itSourceCat).first.GetObjects(), (*itTargetCat).first.GetObjects(), expr_type_);
      if (morphs.empty())
      {
         print_error("Error in morphism definition: " + line_);
         return false;
      }

      for (const Morph& morph : morphs)
         crt_func_.value().morphisms.push_back(morph);

     return true;
   };

   auto fnStateSwitch = [&]()
   {
      return fnEndCategory(crt_cat, ccat_) && fnEndFunctor();
   };

   auto fnImport = [](const std::filesystem::path& path_, const std::string& line_, int line_index_, StringVec& lines_)
   {
      auto pt = path_;
      std::ifstream input(pt.remove_filename().string() + line_ + sExt);
      if (input.is_open())
      {
         std::stringstream descr;
         descr << input.rdbuf();

         input.close();

         StringVec import_lines = split(conform(descr.str()), '\n');

         lines_.insert(lines_.begin() + line_index_ + 1, import_lines.begin(), import_lines.end());
      }
      else
      {
         print_error("Failure to import: " + line_);
         return false;
      }

      return true;
   };

   StringVec lines = split(conform(source_), '\n');

   for (int i = 0; i < lines.size(); ++i)
   {
      if (lines[i].empty())
         continue;

      std::string line = trim_sp(lines[i]);

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
      // Import
      if (head == sImport)
      {
         if (!fnImport(m_path, tail, i, lines))
            return false;
      }
      // Category
      else if (head == sCat)
      {
         if (!fnStateSwitch())
            return false;

         fnBeginCategory(tail, crt_cat, ccat_);

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
SParser::SParser(const std::string& file_path_) : m_path(file_path_)
{
}

//-----------------------------------------------------------------------------------------
bool SParser::parse(cat::CACat& ccat_)
{
   return load_source(m_path.string(), ccat_);
}

//-----------------------------------------------------------------------------------------
bool SParser::load_source(const std::string& path_, CACat& ccat_)
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

   const MorphVec& morphisms = cat_.GetMorphisms();

   for (int i = 0; i < (int)objs_.size() - 1; ++i)
   {
      auto it = std::find_if(morphisms.begin(), morphisms.end(), [&](const MorphVec::value_type& elem_)
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
   MorphVec morphs = cat_.GetMorphisms();

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
ObjVec terminal(Cat& cat_)
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

//-----------------------------------------------------------------------------------------
template <typename T>
static std::optional<cat::Obj> t_coproduct(cat::Obj& fst_, cat::Obj& snd_)
{
   auto pfst = std::get_if<T>(&fst_.Value());
   auto psnd = std::get_if<T>(&snd_.Value());

   if (pfst && psnd)
   {
      cat::Obj ret(fst_.GetName() + "+" + snd_.GetName());
      ret.SetValue(*pfst + *psnd);

      return ret;
   }

   return std::optional<cat::Obj>();
}

//-----------------------------------------------------------------------------------------
std::optional<cat::Obj> coproduct(cat::Obj& fst_, cat::Obj& snd_)
{
   if       (auto cp = t_coproduct<int>         (fst_, snd_))
      return cp;
   else if  (auto cp = t_coproduct<double>      (fst_, snd_))
      return cp;
   else if  (auto cp = t_coproduct<std::string> (fst_, snd_))
      return cp;
   else
      return std::optional<cat::Obj>();
}

//-----------------------------------------------------------------------------------------
template <typename T>
static std::optional<cat::Obj> t_product(cat::Obj& fst_, cat::Obj& snd_)
{
   auto pfst = std::get_if<T>(&fst_.Value());
   auto psnd = std::get_if<T>(&snd_.Value());

   if (pfst && psnd)
   {
      cat::Obj ret(fst_.GetName() + "*" + snd_.GetName());
      ret.SetValue(*pfst * *psnd);

      return ret;
   }

   return std::optional<cat::Obj>();
}

//-----------------------------------------------------------------------------------------
static std::optional<cat::Obj> sproduct(cat::Obj& fst_, cat::Obj& snd_)
{
   auto pfst = std::get_if<std::string>(&fst_.Value());
   auto psnd = std::get_if<std::string>(&snd_.Value());

   if (pfst && psnd)
   {
      cat::Obj ret(fst_.GetName() + "*" + snd_.GetName());

      std::string prod;

      for (auto i : *pfst)
      {
         for (auto j : *psnd)
         {
            prod += i;
            prod += j;
         }
      }

      ret.SetValue(prod);

      return ret;
   }

   return std::optional<cat::Obj>();
}

//-----------------------------------------------------------------------------------------
std::optional<cat::Obj> product(cat::Obj& fst_, cat::Obj& snd_)
{
   if       (auto p = t_product<int>         (fst_, snd_))
      return p;
   else if  (auto p = t_product<double>      (fst_, snd_))
      return p;
   else if  (auto p = sproduct               (fst_, snd_))
      return p;
   else
      return std::optional<cat::Obj>();
}
