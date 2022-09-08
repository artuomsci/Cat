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

/**
 * @brief The EExpType enum represents type of the expression
 */
enum class EExpType
{
      eStatement
   ,  eProof
};

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
template <Arrow::EType ArrowType>
static std::vector<Arrow> get_chains(const std::string& name_, const Node& source_, const Node& target_, const Node::List& domain_, const Node::List& codomain_, EExpType expr_type_)
{
   std::vector<Arrow> ret;

   auto fnCheckSource = [&]()
   {
      if (expr_type_ == EExpType::eStatement)
         return true;

      auto it_d = std::find_if(domain_.begin(), domain_.end(), [&](const Node& element_)
      {
         return element_ == source_;
      });

      if (it_d == domain_.end())
      {
         print_error("No such source: " + source_.Name());
         return false;
      }
      return true;
   };

   auto fnCheckTarget = [&]()
   {
      if (expr_type_ == EExpType::eStatement)
         return true;

      auto it_c = std::find_if(codomain_.begin(), codomain_.end(), [&](const Node& element_)
      {
         return element_ == target_;
      });

      if (it_c == codomain_.end())
      {
         print_error("No such target: " + target_.Name());
         return false;
      }

      return true;
   };

   // f :: a -> b
   if       (name_ != sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(ArrowType, source_.Name(), target_.Name(), name_));
   }
   // * :: a -> b
   else if  (name_ == sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(ArrowType, source_.Name(), target_.Name()));
   }
   // * :: * -> *
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      for (const Node& dnode : domain_)
      {
         for (const auto& cnode : codomain_)
         {
            ret.push_back(Arrow(ArrowType, dnode.Name(), cnode.Name()));
         }
      }
   }
   // * :: * -> b
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(Arrow(ArrowType, dnode.Name(), target_.Name()));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.Name() != sAny && target_.Name() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& cnode : codomain_)
         ret.push_back(Arrow(ArrowType, source_.Name(), cnode.Name()));
   }
   // f :: a -> *
   else if (name_ != sAny && source_.Name() != sAny && target_.Name() == sAny)
   {
      print_error("Incorrect definition");
   }
   // f :: * -> b
   else if (name_ != sAny && source_.Name() == sAny && target_.Name() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(Arrow(ArrowType, dnode.Name(), target_.Name(), name_));
   }
   // f :: * -> *
   else if (name_ != sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      print_error("Incorrect definition");
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
template <Arrow::EType T>
std::string chain_symbol();

template <> std::string chain_symbol<Arrow::EType::eMorphism>() { return "->"; }
template <> std::string chain_symbol<Arrow::EType::eFunctor >() { return "=>"; }

//-----------------------------------------------------------------------------------------
template <Arrow::EType ArrowType>
static std::vector<Arrow> get_chain(const std::string& line_, const Node::List& domain_, const Node::List& codomain_, EExpType expr_type_)
{
   StringVec subsections = split(line_, "::");
   if (subsections.size() != 2)
      return std::vector<Arrow>();

   for (auto& str : subsections)
      str = trim_sp(str);

   const std::string& head = subsections[0];
   const std::string& tail = subsections[1];

   StringVec args = split(tail, chain_symbol<ArrowType>(), false);
   if (args.size() < 2)
      return std::vector<Arrow>();

   for (auto& str : args)
      str = trim_sp(str);

   std::vector<Arrow> ret; ret.reserve(args.size() - 1);
   for (int i = 0; i < (int)args.size() - 1; ++i)
   {
      Node source(args[i + 0], ArrowType == Arrow::EType::eMorphism ? Node::EType::eObject : Node::EType::eSCategory);
      Node target(args[i + 1], ArrowType == Arrow::EType::eMorphism ? Node::EType::eObject : Node::EType::eSCategory);

      for (const auto& it : get_chains<ArrowType>(head, source, target, domain_, codomain_, expr_type_))
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
bool SParser::parse_source(const std::string& source_, Node& node_)
{
   enum class ECurrentEntity
   {
         eCategory
      ,  eFunctor
      ,  eNone
   }
   process_entity { ECurrentEntity::eNone };

   EExpType expr_type { EExpType::eProof };

   std::optional<Node > crt_cat;
   std::optional<Arrow> crt_func;

   auto fnBeginCategory = [](const std::string& line_, std::optional<Node>& crt_cat_, Node& ccat_)
   {
      auto nodes = ccat_.QueryNodes(line_);
      if (nodes.empty())
         crt_cat_.emplace(line_, Node::EType::eSCategory);
      else
         crt_cat_.emplace(nodes.front());
   };

   auto fnEndCategory = [](std::optional<Node>& crt_cat_, Node& ccat_)
   {
      if (crt_cat_)
      {
         Arrow::Vec backup;
         for (const auto& arrow : ccat_.QueryArrows("* -> *"))
         {
            if (arrow.Source() == arrow.Target())
               continue;

            if (arrow.Source() == crt_cat_->Name() || arrow.Target() == crt_cat_->Name())
               backup.push_back(arrow);
         }

         ccat_.EraseNode(crt_cat_->Name());

         ccat_.AddNode(crt_cat_.value());

         for (const auto& arrow : backup)
         {
            if (ccat_.QueryArrows(arrow.Source() + "-[" + arrow.Name() + "]->" + arrow.Target()).empty())
               ccat_.AddArrow(arrow);
         }
      }
         
      crt_cat_.reset();

      return true;
   };

   auto fnAddNodes = [](const std::string& line_, std::optional<Node>& crt_cat_)
   {
      if (!crt_cat_)
      {
         print_error("No category to add node: " + line_);
         return false;
      }

      for (auto& itNodeName : split(line_, ',', false))
      {
         auto nodeName = trim_sp(itNodeName);

         if (!crt_cat_->AddNode(Node(nodeName, Node::EType::eObject)))
         {
            print_error("Failure to add node: " + nodeName);
            return false;
         }
      }

      return true;
   };

   auto fnAddMorphisms = [](const std::string& line_, std::optional<Node>& crt_cat_, EExpType expr_type_)
   {
      if (!crt_cat_)
      {
         print_error("No category to add morphism: " + line_);
         return false;
      }

      const auto& nodes = crt_cat_->QueryNodes("*");

      std::vector<Arrow> morphs = get_chain<Arrow::EType::eMorphism>(line_, nodes, nodes, expr_type_);
      if (morphs.empty())
      {
         print_error("Incorrect morphism definition " + line_ + " in category " + crt_cat_->Name());
         return false;
      }

      for (const Arrow& morph : morphs)
      {
         if (expr_type_ == EExpType::eStatement)
         {
            if (crt_cat_->QueryNodes(morph.Source()).empty())
               crt_cat_->AddNode(Node(morph.Source(), Node::EType::eObject));

            if (crt_cat_->QueryNodes(morph.Target()).empty())
               crt_cat_->AddNode(Node(morph.Target(), Node::EType::eObject));
         }

         if (!crt_cat_->AddArrow(morph))
            return false;
      }

      return true;
   };

   auto fnBeginFunctor = [](const std::string& line_, std::optional<Arrow>& crt_func_, Node& ccat_, EExpType expr_type_)
   {
      auto nodes = ccat_.QueryNodes("*");

      std::vector<Arrow> funcs = get_chain<Arrow::EType::eFunctor>(line_, nodes, nodes, expr_type_);
      if (funcs.size() != 1)
      {
         print_error("Incorrect functor definition: " + line_);
         return false;
      }

      if (expr_type_ == EExpType::eStatement)
      {
         if (ccat_.QueryNodes(funcs.front().Source()).empty())
            ccat_.AddNode(Node(funcs.front().Source(), Node::EType::eSCategory));

         if (ccat_.QueryNodes(funcs.front().Target()).empty())
            ccat_.AddNode(Node(funcs.front().Target(), Node::EType::eSCategory));
      }

      crt_func_.emplace(funcs.front());

      return true;
   };

   auto fnEndFunctor = [&](EExpType expr_type_)
   {
      if (crt_func)
      {
         if (expr_type_ == EExpType::eStatement)
         {
            if (crt_func->IsEmpty())
            {
               auto nodes = node_.QueryNodes(crt_func->Source());
               if (nodes.empty())
                  return false;

               for (const auto& domain : nodes.front().QueryNodes("*"))
                  crt_func->AddArrow(Arrow(Arrow::EType::eMorphism, domain.Name(), domain.Name()));
            }

            if (!node_.Statement(crt_func.value()))
               return false;
         }
         else
         {
            if (!node_.AddArrow(crt_func.value()))
               return false;
         }

         crt_func.reset();
      }

      return true;
   };

   auto fnAddFMorphisms = [](const std::string& line_, std::optional<Arrow>& crt_func_, Node& ccat_, EExpType expr_type_)
   {
      auto itSourceCat = ccat_.QueryNodes(crt_func_->Source()).front();
      auto itTargetCat = ccat_.QueryNodes(crt_func_->Target()).front();

      std::vector<Arrow> morphs = get_chain<Arrow::EType::eMorphism>(line_, itSourceCat.QueryNodes("*"), itTargetCat.QueryNodes("*"), expr_type_);
      if (morphs.empty())
      {
         print_error("Incorrect morphism definition " + line_ + " in functor " + crt_func_->Name());
         return false;
      }

      for (const Arrow& morph : morphs)
         crt_func_.value().AddArrow(morph);

     return true;
   };

   auto fnStateSwitch = [&]()
   {
      return fnEndCategory(crt_cat, node_) && fnEndFunctor(expr_type);
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

         fnBeginCategory(tail, crt_cat, node_);

         process_entity = ECurrentEntity::eCategory;
      }
      // Nodes
      else if (process_entity == ECurrentEntity::eCategory && head == sObj)
      {
         if (!fnAddNodes(tail, crt_cat))
            return false;
      }
      // Arrow
      else if (process_entity == ECurrentEntity::eCategory && is_morphism(line))
      {
         if (!fnAddMorphisms(line, crt_cat, expr_type))
            return false;
      }
      // Arrow
      else if (is_functor(line))
      {
         process_entity = ECurrentEntity::eFunctor;

         if (!fnStateSwitch())
            return false;

         if (!fnBeginFunctor(line, crt_func, node_, expr_type))
            return false;
      }
      // Functor morphism
      else if (process_entity == ECurrentEntity::eFunctor && is_morphism(line))
      {
         if (!fnAddFMorphisms(line, crt_func, node_, expr_type))
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
bool SParser::parse(cat::Node& node_)
{
   return load_source(m_path.string(), node_);
}

//-----------------------------------------------------------------------------------------
bool SParser::load_source(const std::string& path_, Node& node_)
{
   std::ifstream input(path_);
   if (input.is_open())
   {
      std::stringstream descr;
      descr << input.rdbuf();

      input.close();

      return parse_source(descr.str(), node_);
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

////-----------------------------------------------------------------------------------------
//template <typename T>
//static std::optional<cat::Obj> t_coproduct(cat::Obj& fst_, cat::Obj& snd_)
//{
//   auto pfst = std::get_if<T>(&fst_.Value());
//   auto psnd = std::get_if<T>(&snd_.Value());
//
//   if (pfst && psnd)
//   {
//      cat::Obj ret(fst_.Name() + "+" + snd_.Name());
//      ret.SetValue(*pfst + *psnd);
//
//      return ret;
//   }
//
//   return std::optional<cat::Obj>();
//}
//
////-----------------------------------------------------------------------------------------
//std::optional<cat::Obj> coproduct(cat::Obj& fst_, cat::Obj& snd_)
//{
//   if       (auto cp = t_coproduct<int>         (fst_, snd_))
//      return cp;
//   else if  (auto cp = t_coproduct<double>      (fst_, snd_))
//      return cp;
//   else if  (auto cp = t_coproduct<std::string> (fst_, snd_))
//      return cp;
//   else
//      return std::optional<cat::Obj>();
//}
//
////-----------------------------------------------------------------------------------------
//template <typename T>
//static std::optional<cat::Obj> t_product(cat::Obj& fst_, cat::Obj& snd_)
//{
//   auto pfst = std::get_if<T>(&fst_.Value());
//   auto psnd = std::get_if<T>(&snd_.Value());
//
//   if (pfst && psnd)
//   {
//      cat::Obj ret(fst_.Name() + "*" + snd_.Name());
//      ret.SetValue(*pfst * *psnd);
//
//      return ret;
//   }
//
//   return std::optional<cat::Obj>();
//}
//
////-----------------------------------------------------------------------------------------
//static std::optional<cat::Obj> sproduct(cat::Obj& fst_, cat::Obj& snd_)
//{
//   auto pfst = std::get_if<std::string>(&fst_.Value());
//   auto psnd = std::get_if<std::string>(&snd_.Value());
//
//   if (pfst && psnd)
//   {
//      cat::Obj ret(fst_.Name() + "*" + snd_.Name());
//
//      std::string prod;
//
//      for (auto i : *pfst)
//      {
//         for (auto j : *psnd)
//         {
//            prod += i;
//            prod += j;
//         }
//      }
//
//      ret.SetValue(prod);
//
//      return ret;
//   }
//
//   return std::optional<cat::Obj>();
//}
//
////-----------------------------------------------------------------------------------------
//std::optional<cat::Obj> product(cat::Obj& fst_, cat::Obj& snd_)
//{
//   if       (auto p = t_product<int>         (fst_, snd_))
//      return p;
//   else if  (auto p = t_product<double>      (fst_, snd_))
//      return p;
//   else if  (auto p = sproduct               (fst_, snd_))
//      return p;
//   else
//      return std::optional<cat::Obj>();
//}
