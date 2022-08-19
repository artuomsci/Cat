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

enum class ArrowType
{
      eMorphism
   ,  eFunctor
};

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
template <typename TNode, ArrowType ArrowType, typename TContainer>
static std::vector<Arrow> get_chains(const std::string& name_, const TNode& source_, const TNode& target_, const TContainer& domain_, const TContainer& codomain_, EExpType expr_type_)
{
   std::vector<Arrow> ret;

   auto fnCheckSource = [&]()
   {
      if (expr_type_ == EExpType::eStatement)
         return true;

      if (domain_.find(source_) == domain_.end())
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

      if (codomain_.find(target_) == codomain_.end())
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

      ret.push_back(Arrow(source_.Name(), target_.Name(), name_));
   }
   // * :: a -> b
   else if  (name_ == sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(source_.Name(), target_.Name()));
   }
   // * :: * -> *
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      for (const auto& [dnode, _d] : domain_)
      {
         for (const auto& [cnode, _c] : codomain_)
         {
            ret.push_back(Arrow(dnode.Name(), cnode.Name()));
         }
      }
   }
   // * :: * -> b
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& [dnode, _] : domain_)
         ret.push_back(Arrow(dnode.Name(), target_.Name()));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.Name() != sAny && target_.Name() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& [cnode, _] : codomain_)
         ret.push_back(Arrow(source_.Name(), cnode.Name()));
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

      for (const auto& [dnode, _] : domain_)
         ret.push_back(Arrow(dnode.Name(), target_.Name(), name_));
   }
   // f :: * -> *
   else if (name_ != sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      print_error("Incorrect definition");
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
template <ArrowType T>
std::string chain_symbol();

template <> std::string chain_symbol<ArrowType::eMorphism>() { return "->"; }
template <> std::string chain_symbol<ArrowType::eFunctor >() { return "=>"; }

//-----------------------------------------------------------------------------------------
template <ArrowType ArrowType, typename TNode, typename TContainer>
static std::vector<Arrow> get_chain(const std::string& line_, const TContainer& domain_, const TContainer& codomain_, EExpType expr_type_)
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
      TNode source(args[i + 0]);
      TNode target(args[i + 1]);

      for (const auto& it : get_chains<TNode, ArrowType, TContainer>(head, source, target, domain_, codomain_, expr_type_))
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
      auto it = ccat_.Nodes().find(Node(line_));

      if (it == ccat_.Nodes().end())
      {
         crt_cat_.emplace(line_);
      }
      else
      {
         const auto& [cat, _] = *it;

         crt_cat_.emplace(cat);
      }
   };

   auto fnEndCategory = [](std::optional<Node>& crt_cat_, Node& ccat_)
   {
      if (crt_cat_)
      {
         Arrow::Vec backup;
         for (const auto& arrow : ccat_.Arrows())
         {
            if (arrow.source == crt_cat_->Name() || arrow.target == crt_cat_->Name())
               backup.push_back(arrow);
         }

         ccat_.EraseNode(crt_cat_->Name());

         ccat_.AddNode(crt_cat_.value());

         for (const auto& arrow : backup)
         {
            if (!ccat_.Proof(arrow))
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

         if (!crt_cat_->AddNode(Node(nodeName)))
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

      const Node::Node::Map& nodes = crt_cat_->Nodes();

      std::vector<Arrow> morphs = get_chain<ArrowType::eMorphism, Node>(line_, nodes, nodes, expr_type_);
      if (morphs.empty())
      {
         print_error("Incorrect morphism definition " + line_ + " in category " + crt_cat_->Name());
         return false;
      }

      for (const Arrow& morph : morphs)
      {
         if (expr_type_ == EExpType::eStatement)
         {
            if (!crt_cat_->Node::Proof(Node(morph.source)))
               crt_cat_->AddNode(Node(morph.source));

            if (!crt_cat_->Node::Proof(Node(morph.target)))
               crt_cat_->AddNode(Node(morph.target));
         }

         if (!crt_cat_->AddArrow(morph))
            return false;
      }

      return true;
   };

   auto fnBeginFunctor = [](const std::string& line_, std::optional<Arrow>& crt_func_, Node& ccat_, EExpType expr_type_)
   {
      std::vector<Arrow> funcs = get_chain<ArrowType::eFunctor, Node>(line_, ccat_.Nodes(), ccat_.Nodes(), expr_type_);
      if (funcs.size() != 1)
      {
         print_error("Incorrect functor definition: " + line_);
         return false;
      }

      if (expr_type_ == EExpType::eStatement)
      {
         if (!ccat_.Node::Proof(Node(funcs.front().source)))
            ccat_.AddNode(Node(funcs.front().source));

         if (!ccat_.Node::Proof(Node(funcs.front().target)))
            ccat_.AddNode(Node(funcs.front().target));
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
            if (crt_func->arrows.empty())
            {
               auto it = node_.Nodes().find(Node(crt_func->source));
               if (it == node_.Nodes().end())
                  return false;

               const auto& [cat, _] = *it;

               for (const auto& [domain, _] : cat.Nodes())
                  crt_func->arrows.emplace_back(domain.Name(), domain.Name());
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
      const auto& cats = ccat_.Nodes();

      auto itSourceCat = cats.find(Node(crt_func_.value().source));
      auto itTargetCat = cats.find(Node(crt_func_.value().target));

      std::vector<Arrow> morphs = get_chain<ArrowType::eMorphism, Node>(line_, (*itSourceCat).first.Nodes(), (*itTargetCat).first.Nodes(), expr_type_);
      if (morphs.empty())
      {

         print_error("Incorrect morphism definition " + line_ + " in functor " + crt_func_->name);
         return false;
      }

      for (const Arrow& morph : morphs)
         crt_func_.value().arrows.push_back(morph);

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

//-----------------------------------------------------------------------------------------
Node::List solve_sequence(const Node& node_, const Node& from_, const Node& to_)
{
   Node::List ret;

   std::vector<Node::PairSet> stack;

   std::optional<Node> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         for (auto & [nodei, _] : stack)
            ret.push_back(nodei);

         ret.push_back(current_node.value());

         return ret;
      }

      stack.emplace_back(current_node.value(), node_.Nodes().at(current_node.value()));

      // Remove identity morphism
      stack.back().second.erase(current_node.value());

      current_node.reset();

      while (!current_node.has_value())
      {
         // Trying new set of nodes
         Node::Set& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one node forward
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node == current_node.value())
            {
               current_node.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::vector<Node::List> solve_sequences(const Node& node_, const Node& from_, const Node& to_)
{
   std::vector<Node::List> ret;

   std::vector<Node::PairSet> stack;

   std::optional<Node> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         Node::List seq;

         for (auto & [nodei, _] : stack)
            seq.push_back(nodei);

         seq.push_back(current_node.value());

         ret.push_back(seq);
      }
      else
      {
         // Stacking forward movements
         stack.emplace_back(current_node.value(), node_.Nodes().at(current_node.value()));

         // Removing identity morphism
         stack.back().second.erase(current_node.value());
      }

      current_node.reset();

      while (!current_node.has_value())
      {
         // Trying new sets of nodes
         Node::Set& forward_codomain = stack.back().second;

         if (forward_codomain.empty())
         {
            stack.pop_back();

            if (stack.empty())
               return ret;

            continue;
         }

         // Moving one node forward
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node == current_node.value())
            {
               current_node.reset();
               break;
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List map_nodes2arrows(const Node::List& nodes_, const Node& node_)
{
   Arrow::List ret;

   const Arrow::List& arrows = node_.Arrows();

   auto it_last = std::prev(nodes_.end());

   for (auto itn = nodes_.begin(); itn != it_last; ++itn)
   {
      auto it = std::find_if(arrows.begin(), arrows.end(), [&](const Arrow::List::value_type& elem_)
      {
         return itn->Name() == elem_.source && std::next(itn)->Name() == elem_.target;
      });

      ret.push_back(it != arrows.end() ? *it : Arrow("", ""));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
void solve_compositions(Node& node_)
{
   for (const auto & [domain, codomain] : node_.Nodes())
   {
      Node::Set traverse = codomain;

      Node::Set new_codomain;
      while (!traverse.empty())
      {
         new_codomain.insert(traverse.begin(), traverse.end());

         Node::Set new_traverse;
         for (const Node& node : traverse)
         {
            if (node == domain)
               continue;

            const Node::Set& sub_codomain = node_.Nodes().at(node);

            for (const Node& sub_node : sub_codomain)
            {
               if (new_codomain.find(sub_node) != new_codomain.end())
                  continue;

               new_traverse.insert(sub_node);
            }
         }

         traverse = new_traverse;
      }

      Node::Set domain_diff;
      std::set_difference(new_codomain.begin(), new_codomain.end(), codomain.begin(), codomain.end(), std::inserter(domain_diff, domain_diff.begin()));

      for (const auto& codomain_node : domain_diff)
         node_.AddArrow(Arrow(domain.Name(), codomain_node.Name()));
   }
}

//-----------------------------------------------------------------------------------------
void inverse(Node& node_)
{
   Arrow::List arrows = node_.Arrows();

   node_.EraseArrows();

   for (const Arrow& arrow : arrows)
   {
      std::string name = default_arrow_name(arrow.source, arrow.target) == arrow.name ? default_arrow_name(arrow.target, arrow.source) : arrow.name;
      node_.AddArrow(Arrow(arrow.target, arrow.source, name));
   }
}

//-----------------------------------------------------------------------------------------
CAT_EXPORT std::optional<cat::Node> mask(cat::Node& node_, std::string mask_)
{
   cat::Node ret(node_.Name());

   std::string tr_mask = trim_sp(mask_);

   StringVec argss = split(tr_mask, "->");
   for (std::string& it : argss)
      it = trim_sp(it);

   if (argss.size() == 1)
   {
      StringVec args = split(argss.front(), ':');
      for (std::string& it : args)
         it = trim_sp(it);

      if (args.size() == 1)
      {
         if (args.front() == sAny)
            return node_;
         else
         {
            for (const StringVec::value_type& name : split(args.front(), ','))
            {
               if (std::optional<Node> node = node_.FindNode(trim_sp(name)))
               {
                  if (!ret.AddNode(*node))
                     return std::optional<cat::Node>();
               }
            }

            for (const Arrow& arrow : node_.Arrows())
            {
               if (ret.Proof(Node(arrow.source)) && ret.Proof(Node(arrow.target)) && arrow.source != arrow.target)
               {
                  if (!ret.AddArrow(arrow))
                     return std::optional<cat::Node>();
               }
            }
         }
      }
      else
      {
         const auto& node_name = args.front();
         const auto& sub_mask  = args.back ();

         if (auto node = node_.FindNode(node_name))
         {
            auto masked_node = mask(*node, sub_mask);
            if (!masked_node)
               return std::optional<cat::Node>();

            if (!ret.AddNode(*masked_node))
               return std::optional<cat::Node>();
         }
         else
            return std::optional<cat::Node>();
      }

      return ret;
   }
   else
   {
      auto it_last = std::prev(argss.end());

      for (StringVec::iterator it = argss.begin(); it != it_last; ++it)
      {
         auto source_arg = *it;
         auto target_arg = *std::next(it);

         auto source_holder = mask(node_, source_arg);
         if (!source_holder)
            return std::optional<cat::Node>();

         auto target_holder = mask(node_, target_arg);
         if (!target_holder)
            return std::optional<cat::Node>();

         auto [source, src_set] = *source_holder->Nodes().begin();
         auto [target, trg_set] = *target_holder->Nodes().begin();

         if (!ret.AddNode(source))
            return std::optional<cat::Node>();

         if (!ret.AddNode(target))
            return std::optional<cat::Node>();

         auto arrow = node_.FindArrow(source.Name(), target.Name());
         if (!arrow)
            return std::optional<cat::Node>();

         for (Arrow::List::iterator it = arrow->arrows.begin(); it != arrow->arrows.end();)
         {
            if (!source.Proof(Node(it->source)))
               it = arrow->arrows.erase(it);
            else
               it++;
         }

         if (!ret.AddArrow(*arrow))
            return std::optional<cat::Node>();
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Node::List initial(Node& node_)
{
   Node::List ret;

   const Node::Node::Map& nodes = node_.Nodes();

   for (const auto& [domain, codomain] : nodes)
   {
      if (nodes.size() == codomain.size())
         ret.push_back(domain);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Node::List terminal(Node& node_)
{
   Node::List ret;

   for (const auto& [domain, _] : node_.Nodes())
   {
      bool is_terminal { true };

      for (const auto& [domain_int, codomain_int] : node_.Nodes())
      {
         if (std::find_if(codomain_int.begin(), codomain_int.end(), [&](const Node& node_){ return domain == node_; }) == codomain_int.end())
         {
            is_terminal = false;
            break;
         }
      }

      if (is_terminal)
         ret.push_back(domain);
   }

   return ret;
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
