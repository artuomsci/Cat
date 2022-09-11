#include "node.h"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <iterator>
#include <fstream>
#include <sstream>

#include "str_utils.h"

using namespace cat;

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

// Arrow name guards
static const char* const sMArrowNameBegin = "-[";
static const char* const sMArrowNameEnd   = "]->";

static const char* const sFArrowNameBegin = "=[";
static const char* const sFArrowNameEnd   = "]=>";

// Arrow type
static const char* sFunctor  = "=>";
static const char* sMorphism = "->";

// Logic statements
static const char sAND = '&';
static const char sOR  = '|';

// File extension
static const char* const sExt = ".cat";

/**
 * @brief The EExpType enum represents type of the expression
 */
enum class EExpType
{
      eStatement
   ,  eProof
};

//-----------------------------------------------------------------------------------------
static std::string trim_sp(const std::string& string_)
{
   return trim_right(trim_left(string_, ' '), ' ');
}

//-----------------------------------------------------------------------------------------
static bool is_morphism(const std::string& string_)
{
   return (string_.find("::") != -1) && (string_.find(sMorphism) != -1);
}

//-----------------------------------------------------------------------------------------
static bool is_functor(const std::string& string_)
{
   return (string_.find("::") != -1) && (string_.find(sFunctor) != -1);
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
static std::vector<Arrow> resolve_arrows(const std::string& name_, Arrow::EType arrow_type_,  const Node& source_, const Node& target_, const Node::List& domain_, const Node::List& codomain_, EExpType expr_type_)
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

      ret.push_back(Arrow(arrow_type_, source_.Name(), target_.Name(), name_));
   }
   // * :: a -> b
   else if  (name_ == sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(arrow_type_, source_.Name(), target_.Name()));
   }
   // * :: * -> *
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      for (const Node& dnode : domain_)
      {
         for (const auto& cnode : codomain_)
         {
            ret.push_back(Arrow(arrow_type_, dnode.Name(), cnode.Name()));
         }
      }
   }
   // * :: * -> b
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(Arrow(arrow_type_, dnode.Name(), target_.Name()));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.Name() != sAny && target_.Name() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& cnode : codomain_)
         ret.push_back(Arrow(arrow_type_, source_.Name(), cnode.Name()));
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
         ret.push_back(Arrow(arrow_type_, dnode.Name(), target_.Name(), name_));
   }
   // f :: * -> *
   else if (name_ != sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      print_error("Incorrect definition");
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
static std::vector<Arrow> get_arrows(const std::string& line_, Arrow::EType arrow_type_, const Node::List& domain_, const Node::List& codomain_, EExpType expr_type_)
{
   StringVec subsections = split(line_, "::");
   if (subsections.size() != 2)
      return std::vector<Arrow>();

   for (auto& str : subsections)
      str = trim_sp(str);

   const std::string& head = subsections[0];
   const std::string& tail = subsections[1];

   StringVec args = split(tail, arrow_type_ == Arrow::EType::eMorphism ? sMorphism : sFunctor, false);
   if (args.size() < 2)
      return std::vector<Arrow>();

   for (auto& str : args)
      str = trim_sp(str);

   std::vector<Arrow> ret; ret.reserve(args.size() - 1);
   for (int i = 0; i < (int)args.size() - 1; ++i)
   {
      auto node_type = arrow_type_ == Arrow::EType::eMorphism ? Node::EType::eObject : Node::EType::eSCategory;

      Node source(args[i + 0], node_type);
      Node target(args[i + 1], node_type);

      for (const auto& it : resolve_arrows(head, arrow_type_, source, target, domain_, codomain_, expr_type_))
         ret.push_back(it);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Arrow::Arrow(EType type_, const std::string& source_, const std::string& target_, const std::string& arrow_name_) :
      m_source   (source_)
   ,  m_target   (target_)
   ,  m_name     (arrow_name_)
   ,  m_type     (type_)
{};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(EType type_, const std::string& source_, const std::string& target_) :
      m_source   (source_)
   ,  m_target   (target_)
   ,  m_name     (DefaultArrowName(source_, target_))
   ,  m_type     (type_)
{};

//-----------------------------------------------------------------------------------------
bool Arrow::operator<(const Arrow& arrow_) const
{
   return std::tie(m_source, m_target, m_name) < std::tie(arrow_.m_source, arrow_.m_target, arrow_.m_name);
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator==(const Arrow& arrow_) const
{
   return
         m_source      == arrow_.m_source
      && m_target      == arrow_.m_target
      && m_name        == arrow_.m_name
      && m_arrows      == arrow_.m_arrows
      && m_type        == arrow_.m_type;
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator!=(const Arrow& arrow_) const
{
   return
         m_source      != arrow_.m_source
      || m_target      != arrow_.m_target
      || m_name        != arrow_.m_name
      || m_arrows      != arrow_.m_arrows
      || m_type        != arrow_.m_type;
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::operator()(const std::optional<Node>& node_) const
{
   if (node_->Name() != m_source)
      return std::optional<Node>();

   Node ret(m_target, node_->Type());

   // Mapping of nodes
   for (const auto& node : node_->QueryNodes("*"))
   {
      Node mapped_node = SingleMap(node).value();
      if (ret.QueryNodes(mapped_node.Name()).empty())
         ret.AddNode(mapped_node);
   }

   // Mapping of arrows
   for (const Arrow& arrow : node_->QueryArrows("* -> *"))
   {
      auto source = SingleMap(Node(arrow.m_source, Node::EType::eObject));
      auto target = SingleMap(Node(arrow.m_target, Node::EType::eObject));

      Arrow mapped_arrow(EType::eMorphism, source->Name(), target->Name());
      if (ret.QueryArrows(mapped_arrow.Source() + sMArrowNameBegin + mapped_arrow.Name() + sMArrowNameEnd + mapped_arrow.Target()).empty())
         ret.AddArrow(mapped_arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::DefaultArrowName(const std::string& source_, const std::string& target_)
{
   return source_ + "-" + target_;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::IdArrowName(const std::string& name_)
{
   return DefaultArrowName(name_, name_);
}

//-----------------------------------------------------------------------------------------
const std::string& Arrow::Source() const
{
   return m_source;
}

//-----------------------------------------------------------------------------------------
const std::string& Arrow::Target() const
{
   return m_target;
}

//-----------------------------------------------------------------------------------------
const Arrow::AName& Arrow::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
void Arrow::AddArrow(const Arrow& arrow_)
{
   if (m_type == Arrow::EType::eFunctor && arrow_.Type() == Arrow::EType::eMorphism)
      m_arrows.push_back(arrow_);
}

//-----------------------------------------------------------------------------------------
void Arrow::EraseArrow(const Arrow::AName& arrow_)
{   
   auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const List::value_type& element_)
   {
      return element_.Name() == arrow_;
   });

   if (it != m_arrows.end())
      m_arrows.erase(it);
}

//-----------------------------------------------------------------------------------------
void Arrow::EraseArrows()
{
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
static std::optional<Arrow> extract_arrow_from_query(Arrow::EType type_, const std::string& query_)
{
   auto query = trim_sp(query_);

   auto begin = type_ == Arrow::EType::eMorphism ? sMArrowNameBegin : sFArrowNameBegin;
   auto end   = type_ == Arrow::EType::eMorphism ? sMArrowNameEnd   : sFArrowNameEnd  ;

   // Arrow name is optional, checking for it
   if (query.find(begin) != -1 && query.find(end) != -1)
   {
      auto head = split(query, begin, false);
      for (auto& it : head)
         it = trim_sp(it);

      if (head.size() != 2)
         return std::optional<Arrow>();

      auto tail = split(head[1], end, false);
      for (auto& it : tail)
         it = trim_sp(it);

      if (tail.size() != 2)
         return std::optional<Arrow>();

      return Arrow(type_, head[0], tail[1], tail[0]);
   }
   else
   {
      auto parts = split(query, type_ == Arrow::EType::eMorphism ? sMorphism : sFunctor, false);
      for (auto& it : parts)
         it = trim_sp(it);

      if (parts.size() != 2)
         return std::optional<Arrow>();

      return Arrow(type_, parts[0], parts[1], sAny);
   }
}

//-----------------------------------------------------------------------------------------
Arrow::List query_arrows(Arrow::EType type_, const std::string& query_, const Arrow::List& arrows_, std::optional<size_t> matchCount_)
{
   auto qarrow = extract_arrow_from_query(type_, query_);
   if (!qarrow)
      return Arrow::List();

   Arrow::List ret;

   if       (qarrow->Source() == sAny && qarrow->Target() == sAny)
   {
      if (qarrow->Name() == sAny)
      {
         return arrows_;
      }
      else
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Name() == qarrow->Name())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Name() == qarrow->Name())
                  ret.push_back(arrow);
            }
         }
      }
   }
   else if  (qarrow->Source() != sAny && qarrow->Target() == sAny)
   {
      if (qarrow->Name() == sAny)
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source())
                  ret.push_back(arrow);
            }
         }
      }
      else
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Name() == qarrow->Name())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Name() == qarrow->Name())
                  ret.push_back(arrow);
            }
         }
      }
   }
   else if  (qarrow->Source() == sAny && qarrow->Target() != sAny)
   {
      if (qarrow->Name() == sAny)
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Target() == qarrow->Target())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Target() == qarrow->Target())
                  ret.push_back(arrow);
            }
         }
      }
      else
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
                  ret.push_back(arrow);
            }
         }
      }
   }
   else if  (qarrow->Source() != sAny && qarrow->Target() != sAny)
   {
      if (qarrow->Name() == sAny)
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target())
                  ret.push_back(arrow);
            }
         }
      }
      else
      {
         if (matchCount_)
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
               {
                  ret.push_back(arrow);

                  if (ret.size() == matchCount_)
                     break;
               }
            }
         }
         else
         {
            for (const auto& arrow : arrows_)
            {
               if (arrow.Source() == qarrow->Source() && arrow.Target() == qarrow->Target() && arrow.Name() == qarrow->Name())
                  ret.push_back(arrow);
            }
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Arrow::List Arrow::QueryArrows(const std::string& query_, std::optional<size_t> matchCount_) const
{
   return query_arrows(Arrow::EType::eMorphism, query_, m_arrows, matchCount_);
}

//-----------------------------------------------------------------------------------------
bool Arrow::IsEmpty() const
{
   return m_arrows.empty();
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::SingleMap(const std::optional<Node>& node_) const
{
   if (!node_)
      return std::optional<Node>();

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Source() == node_->Name())
      {
         return Node(arrow.Target(), Node::EType::eObject);
      }
   }

   return std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
std::optional<Node> Arrow::SingleMap(const std::string& name_) const
{
   for (const Arrow& arrow : m_arrows)
   {
      if (arrow.Source() == name_)
      {
         return Node(arrow.Target(), Node::EType::eObject);
      }
   }

   return std::optional<Node>();
}

//-----------------------------------------------------------------------------------------
void Arrow::Inverse()
{
   m_name = DefaultArrowName(m_source, m_target) == m_name ? DefaultArrowName(m_target, m_source) : m_name;

   std::swap(m_source, m_target);

   for (auto& arrow : m_arrows)
      arrow.Inverse();
}

//-----------------------------------------------------------------------------------------
Arrow::EType Arrow::Type() const
{
   return m_type;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool Node::operator<(const Node& cat_) const
{
   return m_name < cat_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Node::operator==(const Node& cat_) const
{
   return m_name == cat_.m_name && m_type == cat_.m_type;
}

//-----------------------------------------------------------------------------------------
bool Node::operator!=(const Node& cat_) const
{
   return m_name != cat_.m_name || m_type != cat_.m_type;
}

//-----------------------------------------------------------------------------------------
Node::Node(const NName& name_, EType type_) : m_name(name_), m_type(type_)
{
}

//-----------------------------------------------------------------------------------------
bool Node::Parse(const std::string& path_)
{
   return parse_source(path_);
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrow(const Arrow& arrow_)
{
   if (m_type == EType::eObject)
   {
      print_error("Arrows aren't allowed");
      return false;
   }

   if (!QueryArrows(arrow_.Source() + sMArrowNameBegin + arrow_.Name() + sMArrowNameEnd + arrow_.Target()).empty())
   {
      print_error("Arrow redefinition: " + arrow_.Name());
      return false;
   }

   for (const Arrow& arrow : m_arrows)
   {
      if (arrow_.Name() == arrow.Name() && arrow_.Target() != arrow.Target())
      {
         print_error("Arrow redefinition: " + arrow_.Name());
         return false;
      }
   }

   if (!Verify(arrow_))
      return false;

   auto it = m_nodes.find(Node(arrow_.Target(), InternalNode()));

   const auto& [target, _] = *it;

   m_nodes.at(Node(arrow_.Source(), InternalNode())).insert(target);

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrows(const Arrow::Vec& arrows_)
{
   for (const Arrow& arrow : arrows_)
   {
      if (!AddArrow(arrow))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseArrow(const Arrow::AName& name_)
{
   for (Arrow& arrow : m_arrows)
   {
      if (arrow.Name() == name_)
      {
         if (arrow.Source() == arrow.Target())
         {
            print_error("Deleting identity arrow " + name_);
            return false;
         }

         auto it_node = m_nodes.find(Node(arrow.Source(), InternalNode()));
         if (it_node != m_nodes.end())
         {
            if (arrow.Source() == arrow.Target())
               return false;

            auto& [_, node_set] = *it_node;

            node_set.erase(Node(arrow.Target(), InternalNode()));
         }
      }
   }

   auto it_delete = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
   {
      return element_.Name() == name_;
   });

   if (it_delete == m_arrows.end())
      return false;

   m_arrows.erase(it_delete);

   return true;
}

//-----------------------------------------------------------------------------------------
void Node::EraseArrows()
{
   for (auto it = m_arrows.begin(); it != m_arrows.end();)
   {
      if (it->Source() != it->Target())
         it = m_arrows.erase(it);
      else
         ++it;
   }

   for (auto& [node, nodeset] : m_nodes)
   {
      nodeset.clear();
      nodeset.insert(node);
   }
}

//-----------------------------------------------------------------------------------------
bool Node::IsArrowsEmpty() const
{
   return m_arrows.empty();
}

//-----------------------------------------------------------------------------------------
bool Node::AddNode(const Node& node_)
{
   if (node_.Name().empty())
      return false;

   if (m_nodes.find(node_) == m_nodes.end())
   {
      m_nodes[node_];

      Arrow func(InternalArrow(), node_.Name(), node_.Name(), Arrow::IdArrowName(node_.Name()));

      for (const auto& id : node_.QueryNodes("*"))
         func.AddArrow(Arrow(Arrow::EType::eMorphism, id.Name(), id.Name()));

      if (!AddArrow(func))
         return false;
   }
   else
   {
      print_error("Node redefinition: " + node_.Name());
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::AddNodes(const Vec& nodes_)
{
   for (const Node& node : nodes_)
   {
      if (!AddNode(node))
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------
bool Node::EraseNode(const NName& node_)
{
   NName name_copy = node_;

   auto it = m_nodes.find(Node(name_copy, InternalNode()));
   if (it != m_nodes.end())
   {
      m_nodes.erase(it);

      for (auto& [_, codomain] : m_nodes)
         codomain.erase(Node(name_copy, InternalNode()));

      auto it_end = std::remove_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::Vec::value_type& element_)
      {
         return element_.Source() == name_copy || element_.Target() == name_copy;
      });

      m_arrows.erase(it_end, m_arrows.end());

      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
void Node::EraseNodes()
{
   m_nodes .clear();
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
Node::List Node::FindByTargets(const std::list<Node::NName>& targets_) const
{
   Node::List ret;

   for (const auto& [domain, codomain] : m_nodes)
   {
      auto it_last = std::prev(targets_.end());

      for (auto it = targets_.begin(); it != targets_.end(); ++it)
      {
         // skipping identities
         if (domain.Name() == *it)
            continue;

         if (codomain.find(Node(*it, InternalNode())) == codomain.end())
            break;

         if (it == it_last)
            ret.push_back(domain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::IsNodesEmpty() const
{
   return m_nodes.empty();
}

//-----------------------------------------------------------------------------------------
Arrow::List Node::QueryArrows(const std::string& query_, std::optional<size_t> matchCount_) const
{
   return query_arrows(InternalArrow(), query_, m_arrows, matchCount_);
}

//-----------------------------------------------------------------------------------------
Node::List Node::QueryNodes(const std::string& query_) const
{
   auto query = trim_sp(query_);

   Node::List ret;

   if (query == sAny)
   {
      for (const auto& [node, _] : m_nodes)
      {
         ret.push_back(node);
      }
   }
   else
   {
      auto node_names = split(query, sOR, false);

      for (auto& it : node_names)
         it = trim_sp(it);

      for (auto& name : node_names)
      {
         auto it = m_nodes.find(Node(name, InternalNode()));
         if (it != m_nodes.end())
            ret.push_back(it->first);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::Verify(const Arrow& arrow_) const
{
   auto itSourceCat = m_nodes.find(Node(arrow_.Source(), InternalNode()));
   auto itTargetCat = m_nodes.find(Node(arrow_.Target(), InternalNode()));

   if (itSourceCat == m_nodes.end())
   {
      print_error("No such source node: " + arrow_.Source());
      return false;
   }

   if (itTargetCat == m_nodes.end())
   {
      print_error("No such target node: " + arrow_.Target());
      return false;
   }

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   using TSource2Arrow = std::set<std::pair<Node::NName, Arrow::AName>>;
   TSource2Arrow visited;

   for (const Arrow& arrow : arrow_.QueryArrows("*->*"))
   {
      auto head = TSource2Arrow::value_type(arrow.Source(), arrow.Name());

      auto itv = visited.find(head);
      if (itv != visited.end())
      {
         auto msg = "Functor: " + arrow_.Name() + " : ";
         msg += "Mapping the same node " + arrow.Source() + " multiple times with arrow " + arrow.Name();
         print_error(msg);
         return false;
      }

      visited.insert(head);

      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("Missing node for " + arrow.Source() + sMorphism + arrow.Target());
         return false;
      }
   }

   // Checking mapping of objects
   for (const auto& obj : source_cat.QueryNodes("*"))
   {
      if (!arrow_.SingleMap(obj))
      {
         print_error("Failure to map node: " + obj.Name());
         return false;
      }
   }

   for (const Arrow& arrow : source_cat.QueryArrows("* -> *"))
   {
      auto objs = arrow_.SingleMap(Node(arrow.Source(), source_cat.InternalNode()));
      auto objt = arrow_.SingleMap(Node(arrow.Target(), source_cat.InternalNode()));

      if (!objs)
      {
         print_error("Failure to map node: " + arrow.Source());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("No such node " + arrow.Source() + " in node " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objs->Name()).empty())
      {
         print_error("No such node " + objs->Name() + " in node " + target_cat.Name());
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map node: " + arrow.Target());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Target()).empty())
      {
         print_error("No such node " + arrow.Target() + " in node " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objt->Name()).empty())
      {
         print_error("No such node " + objt->Name() + " in node " + target_cat.Name());
         return false;
      }

      // Checking mapping of arrows
      if (target_cat.QueryArrows(objs->Name() + sMorphism + objt->Name()).empty())
      {
         print_error("Failure to match arrow: " + objs->Name() + sMorphism + objt->Name());
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
const Node::NName& Node::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
bool Node::Statement(const Arrow& arrow_)
{
   Node target(arrow_.Target(), InternalNode());

   auto it = m_nodes.find(Node(arrow_.Target(), InternalNode()));
   if (it != m_nodes.end())
      target = it->first;

   auto source = QueryNodes(arrow_.Source());
   if (source.empty())
      return false;

   // Mapping nodes
   for (const auto& node : source.front().QueryNodes("*"))
   {
      std::optional<Node> mnode = arrow_.SingleMap(node);
      if (!mnode)
      {
         print_error("Missing arrow for node " + node.Name() + " in functor " + arrow_.Name());
         return false;
      }

      if (target.QueryNodes(mnode->Name()).empty())
         target.AddNode(mnode.value());
   }

   // Mapping arrows
   for (const Arrow& arrow : source.front().QueryArrows("* -> *"))
   {
      auto nodes = arrow_.SingleMap(Node(arrow.Source(), source.front().InternalNode()));
      auto nodet = arrow_.SingleMap(Node(arrow.Target(), source.front().InternalNode()));

      if (target.QueryArrows(nodes->Name() + sMorphism + nodet->Name()).empty())
         target.AddArrow(Arrow(Arrow::EType::eMorphism, nodes->Name(), nodet->Name()));
   }

   for (auto& [domain, codomain] : m_nodes)
   {
      auto itTarget = codomain.find(target);
      if (itTarget != codomain.end())
      {
         codomain.erase(itTarget);
         codomain.insert(target);
      }
   }

   auto itTarget = m_nodes.find(target);
   if (itTarget != m_nodes.end())
   {
      auto back_up = itTarget->second;

      m_nodes.erase(itTarget);

      m_nodes[target] = back_up;
   }

   if (!QueryArrows(arrow_.Source() + sMArrowNameBegin + arrow_.Name() + sMArrowNameEnd + arrow_.Target()).empty())
   {
      print_error("Arrow already defined: " + arrow_.Name());
      return false;
   }

   m_nodes[Node(arrow_.Source(), InternalNode())].insert(Node(arrow_.Target(), InternalNode()));

   m_arrows.push_back(arrow_);

   return true;
}

//-----------------------------------------------------------------------------------------
void Node::SolveCompositions()
{
   for (const auto & [domain, codomain] : m_nodes)
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

            const auto& sub_codomain = m_nodes.at(node);

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
         AddArrow(Arrow(InternalArrow(), domain.Name(), codomain_node.Name()));
   }
}

//-----------------------------------------------------------------------------------------
Node::List Node::Initial() const
{
   Node::List ret;

   for (const auto& [domain, codomain] : m_nodes)
   {
      if (m_nodes.size() == codomain.size())
         ret.push_back(domain);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
Node::List Node::Terminal() const
{
   Node::List ret;

   for (const auto& [domain, _] : m_nodes)
   {
      bool is_terminal { true };

      for (const auto& [domain_int, codomain_int] : m_nodes)
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

//-----------------------------------------------------------------------------------------
Node::List Node::SolveSequence(const Node& from_, const Node& to_) const
{
   Node::List ret;

   std::list<Node::PairSet> stack;

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

      stack.emplace_back(current_node.value(), m_nodes.at(current_node.value()));

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
std::list<Node::List> Node::SolveSequences(const Node& from_, const Node& to_) const
{
   std::list<Node::List> ret;

   std::list<Node::PairSet> stack;

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
         stack.emplace_back(current_node.value(), m_nodes.at(current_node.value()));

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
Arrow::List Node::MapNodes2Arrows(const Node::List& nodes_) const
{
   Arrow::List ret;

   auto it_last = std::prev(nodes_.end());

   for (auto itn = nodes_.begin(); itn != it_last; ++itn)
   {
      auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::List::value_type& elem_)
      {
         return itn->Name() == elem_.Source() && std::next(itn)->Name() == elem_.Target();
      });

      ret.push_back(it != m_arrows.end() ? *it : Arrow(InternalArrow(), "", ""));
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
void Node::Inverse()
{
   Arrow::List tmp = m_arrows;

   EraseArrows();

   for (Arrow& arrow : tmp)
   {
      arrow.Inverse();
      AddArrow(arrow);
   }
}

//-----------------------------------------------------------------------------------------
Node::EType Node::Type() const
{
   return m_type;
}

//-----------------------------------------------------------------------------------------
Node::EType Node::InternalNode() const
{
   if       (m_type == EType::eLCategory)
      return EType::eSCategory;
   else if  (m_type == EType::eSCategory)
      return EType::eObject;
   else
      return EType::eUndefined;
}

//-----------------------------------------------------------------------------------------
Arrow::EType Node::InternalArrow() const
{
   return m_type == Node::EType::eLCategory ? Arrow::EType::eFunctor : Arrow::EType::eMorphism;
}

//-----------------------------------------------------------------------------------------
bool Node::validate_node_data() const
{
   size_t sz {};
   for (const auto& [_, codomain] : m_nodes)
      sz += codomain.size();

   return sz == m_arrows.size();
}

//-----------------------------------------------------------------------------------------
bool Node::parse_source(const std::string& path_)
{
   std::ifstream input(path_);
   if (!input.is_open())
      return false;

   std::stringstream descr;
   descr << input.rdbuf();

   input.close();

   std::filesystem::path path(path_);

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

   auto fnBeginCategory = [&](const std::string& line_, std::optional<Node>& crt_cat_)
   {
      auto nodes = QueryNodes(line_);
      if (nodes.empty())
         crt_cat_.emplace(line_, Node::EType::eSCategory);
      else
         crt_cat_.emplace(nodes.front());
   };

   auto fnEndCategory = [&](std::optional<Node>& crt_cat_)
   {
      if (crt_cat_)
      {
         Arrow::Vec backup;
         for (const auto& arrow : QueryArrows("* -> *"))
         {
            if (arrow.Source() == arrow.Target())
               continue;

            if (arrow.Source() == crt_cat_->Name() || arrow.Target() == crt_cat_->Name())
               backup.push_back(arrow);
         }

         EraseNode(crt_cat_->Name());

         AddNode(crt_cat_.value());

         for (const auto& arrow : backup)
         {
            if (QueryArrows(arrow.Source() + sMArrowNameBegin + arrow.Name() + sMArrowNameEnd + arrow.Target()).empty())
               AddArrow(arrow);
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

      for (auto& itNodeName : split(line_, sAND, false))
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

   auto fnAddMorphisms = [&](const std::string& line_, std::optional<Node>& crt_cat_, EExpType expr_type_)
   {
      if (!crt_cat_)
      {
         print_error("No category to add morphism: " + line_);
         return false;
      }

      const auto& nodes = crt_cat_->QueryNodes("*");

      std::vector<Arrow> morphs = get_arrows(line_, Arrow::EType::eMorphism, nodes, nodes, expr_type_);
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

   auto fnBeginFunctor = [&](const std::string& line_, std::optional<Arrow>& crt_func_, EExpType expr_type_)
   {
      auto nodes = QueryNodes("*");

      std::vector<Arrow> funcs = get_arrows(line_, Arrow::EType::eFunctor, nodes, nodes, expr_type_);
      if (funcs.size() != 1)
      {
         print_error("Incorrect functor definition: " + line_);
         return false;
      }

      if (expr_type_ == EExpType::eStatement)
      {
         if (QueryNodes(funcs.front().Source()).empty())
            AddNode(Node(funcs.front().Source(), Node::EType::eSCategory));

         if (QueryNodes(funcs.front().Target()).empty())
            AddNode(Node(funcs.front().Target(), Node::EType::eSCategory));
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
               auto nodes = QueryNodes(crt_func->Source());
               if (nodes.empty())
                  return false;

               for (const auto& domain : nodes.front().QueryNodes("*"))
                  crt_func->AddArrow(Arrow(Arrow::EType::eMorphism, domain.Name(), domain.Name()));
            }

            if (!Statement(crt_func.value()))
               return false;
         }
         else
         {
            if (!AddArrow(crt_func.value()))
               return false;
         }

         crt_func.reset();
      }

      return true;
   };

   auto fnAddFMorphisms = [&](const std::string& line_, std::optional<Arrow>& crt_func_, EExpType expr_type_)
   {
      auto itSourceCat = QueryNodes(crt_func_->Source()).front();
      auto itTargetCat = QueryNodes(crt_func_->Target()).front();

      std::vector<Arrow> morphs = get_arrows(line_, Arrow::EType::eMorphism, itSourceCat.QueryNodes("*"), itTargetCat.QueryNodes("*"), expr_type_);
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
      return fnEndCategory(crt_cat) && fnEndFunctor(expr_type);
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

   StringVec lines = split(conform(descr.str()), '\n');

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
         if (!fnImport(path_, tail, i, lines))
            return false;
      }
      // Category
      else if (head == sCat)
      {
         if (!fnStateSwitch())
            return false;

         fnBeginCategory(tail, crt_cat);

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

         if (!fnBeginFunctor(line, crt_func, expr_type))
            return false;
      }
      // Functor morphism
      else if (process_entity == ECurrentEntity::eFunctor && is_morphism(line))
      {
         if (!fnAddFMorphisms(line, crt_func, expr_type))
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
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
