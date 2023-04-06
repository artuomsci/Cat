#include "node.h"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <iterator>
#include <fstream>
#include <sstream>

#include "str_utils.h"
#include "tokenizer.h"
#include "parser.h"

using namespace cat;

// Keywords

// Logic statements
static const char sAND = '&';
static const char sOR  = '|';

// entity names
static const char* sNObject    = "object";
static const char* sNSCategory = "category";
static const char* sNLCategory = "large category";

static const char* sNFunctor   = "functor";
static const char* sNMorphism  = "morphism";

static const char* sVoid       = "void";

//-----------------------------------------------------------------------------------------
static std::string trim_sp_s(const std::string& string_, std::string::value_type symbol_)
{
   return trim_right(trim_left(string_, symbol_), symbol_);
}

//-----------------------------------------------------------------------------------------
static std::string trim_sp(const std::string& string_)
{
   return trim_sp_s(string_, ' ');
}

//-----------------------------------------------------------------------------------------
static std::vector<Arrow> resolve_arrows(const std::string& name_, const Node& source_, const Node& target_, const Node::List& domain_, const Node::List& codomain_)
{
   std::vector<Arrow> ret;

   auto fnCheckSource = [&]()
   {
      auto it_d = std::find_if(domain_.begin(), domain_.end(), [&](const Node& element_)
      {
         return element_ == source_;
      });

      if (it_d == domain_.end())
      {
         print_error("No such source " + Node::Type2Name(source_.Type()) + ": " + source_.Name());
         return false;
      }
      return true;
   };

   auto fnCheckTarget = [&]()
   {
      auto it_c = std::find_if(codomain_.begin(), codomain_.end(), [&](const Node& element_)
      {
         return element_ == target_;
      });

      if (it_c == codomain_.end())
      {
         print_error("No such target " + Node::Type2Name(target_.Type()) + ": " + target_.Name());
         return false;
      }

      return true;
   };

   std::string sAny(1, ASTERISK::id);

   // f :: a -> b
   if       (name_ != sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(source_, target_, name_));
   }
   // * :: a -> b
   else if  (name_ == sAny && source_.Name() != sAny && target_.Name() != sAny)
   {
      if (!fnCheckSource())
         return ret;
      if (!fnCheckTarget())
         return ret;

      ret.push_back(Arrow(source_, target_));
   }
   // * :: * -> *
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      for (const Node& dnode : domain_)
      {
         for (const auto& cnode : codomain_)
         {
            ret.push_back(Arrow(dnode, cnode));
         }
      }
   }
   // * :: * -> b
   else if (name_ == sAny && source_.Name() == sAny && target_.Name() != sAny)
   {
      if (!fnCheckTarget())
         return ret;

      for (const auto& dnode : domain_)
         ret.push_back(Arrow(dnode, target_));
   }
   // * :: a -> *
   else if (name_ == sAny && source_.Name() != sAny && target_.Name() == sAny)
   {
      if (!fnCheckSource())
         return ret;

      for (const auto& cnode : codomain_)
         ret.push_back(Arrow(source_, cnode));
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
         ret.push_back(Arrow(dnode, target_, name_));
   }
   // f :: * -> *
   else if (name_ != sAny && source_.Name() == sAny && target_.Name() == sAny)
   {
      print_error("Incorrect definition");
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
static std::vector<Arrow> get_arrows(const std::string& line_, Arrow::EType arrow_type_, const Node::List& domain_, const Node::List& codomain_, bool resolve_)
{
   std::list<TToken> tks = Tokenizer::Process(line_);

   Arrow::List arrows;
   auto start_tk = tks.begin();
   Parser::parse_arrow(start_tk, tks.end(), arrows);

   std::vector<Arrow> ret; ret.reserve(arrows.size());
   for (const auto& arrow : arrows)
   {
      auto node_type = arrow_type_ == Arrow::EType::eMorphism ? Node::EType::eObject : Node::EType::eSCategory;

      Node source(arrow.Source(), node_type);
      Node target(arrow.Target(), node_type);

      if (resolve_)
      {
         for (const auto& it : resolve_arrows(arrow.Name(), source, target, domain_, codomain_))
            ret.push_back(it);
      }
      else
         ret.push_back(arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::string Arrow::Type2Name(EType type_)
{
   if       (type_ == EType::eFunctor)
      return sNFunctor;
   else if  (type_ == EType::eMorphism)
      return sNMorphism;
   else
      return "";
}

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
Arrow::Arrow(const Node& source_, const Node& target_, const std::string& arrow_name_) :
   m_source (source_.Name())
 , m_target (target_.Name())
 , m_name   (arrow_name_)
 , m_type   (source_.ExternalArrow())
{}

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const Node& source_, const Node& target_) :
   Arrow(source_, target_, DefaultArrowName(source_.Name(), target_.Name()))
{}

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
   for (const Arrow& arrow : node_->QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()))
   {
      auto source = SingleMap(Node(arrow.m_source, Node::EType::eObject));
      auto target = SingleMap(Node(arrow.m_target, Node::EType::eObject));

      Arrow mapped_arrow(*source, *target);
      if (ret.QueryArrows(mapped_arrow.AsQuery()).empty())
         ret.AddArrow(mapped_arrow);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::string Arrow::DefaultArrowName(const std::string& source_, const std::string& target_)
{
   std::string sAny(1, ASTERISK::id);

   if (source_ == sAny || target_ == sAny)
      return sAny;
   else
      return source_ + "_" + target_;
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
void Arrow::SetSource(const std::string& source_)
{
   if (DefaultArrowName(m_source, m_target) == m_name)
   {
      m_source = source_;
      m_name = DefaultArrowName(m_source, m_target);
   }
   else
      m_source = source_;
}

//-----------------------------------------------------------------------------------------
const std::string& Arrow::Target() const
{
   return m_target;
}

//-----------------------------------------------------------------------------------------
void Arrow::SetTarget(const std::string& target_)
{
   if (DefaultArrowName(m_source, m_target) == m_name)
   {
      m_target = target_;
      m_name = DefaultArrowName(m_source, m_target);
   }
   else
      m_target = target_;
}

//-----------------------------------------------------------------------------------------
const Arrow::AName& Arrow::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
void Arrow::AddArrow(const Arrow& arrow_)
{
   if (m_type == Arrow::EType::eFunctor  && arrow_.Type() == Arrow::EType::eMorphism ||
       m_type == Arrow::EType::eMorphism && arrow_.Type() == Arrow::EType::eFunction)
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
Arrow::List query_arrows(Arrow::EType type_, const std::string& query_, const Arrow::List& arrows_, std::optional<size_t> matchCount_)
{
   if (matchCount_ && matchCount_ == 0)
      return Arrow::List();

   auto qarrow = get_arrows(query_, type_, Node::List(), Node::List(), false);
   if (qarrow.size() != 1)
      return Arrow::List();

   const auto& source = qarrow[0].Source();
   const auto& target = qarrow[0].Target();
   const auto& name   = qarrow[0].Name  ();

   Arrow::List ret;

   std::string sAny(1, ASTERISK::id);

   bool name_check = name != sAny;

   if       (source == sAny && target == sAny)
   {
      if (!name_check && !matchCount_)
         return arrows_;

      for (const auto& arrow : arrows_)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Name() == name)
         {
            ret.push_back(arrow);

            if (matchCount_ && ret.size() == matchCount_)
               break;
         }
      }
   }
   else if  (source != sAny && target == sAny)
   {
      for (const auto& arrow : arrows_)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Source() == source)
         {
            ret.push_back(arrow);

            if (matchCount_ && ret.size() == matchCount_)
               break;
         }
      }
   }
   else if  (source == sAny && target != sAny)
   {
      for (const auto& arrow : arrows_)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Target() == target)
         {
            ret.push_back(arrow);

            if (matchCount_ && ret.size() == matchCount_)
               break;
         }
      }
   }
   else if  (source != sAny && target != sAny)
   {
      for (const auto& arrow : arrows_)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Source() == source && arrow.Target() == target)
         {
            ret.push_back(arrow);

            if (matchCount_ && ret.size() == matchCount_)
               break;
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
std::string Arrow::AsQuery() const
{
   if (m_type == Arrow::EType::eMorphism)
      return m_source + BEGIN_SINGLE_ARROW::id + m_name + END_SINGLE_ARROW::id + m_target + SEMICOLON::id;
   else
      return m_source + BEGIN_DOUBLE_ARROW::id + m_name + END_DOUBLE_ARROW::id + m_target + SEMICOLON::id;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

std::string Node::Type2Name(Node::EType type_)
{
   if       (type_ == EType::eObject)
      return sNObject;
   else if  (type_ == EType::eSCategory)
      return sNSCategory;
   else if  (type_ == EType::eLCategory)
      return sNLCategory;
   else
      return "";
}

//-----------------------------------------------------------------------------------------
std::string Node::Type2Str(EType type_)
{
   if       (type_ == EType::eObject)
      return "eObject";
   else if  (type_ == EType::eSCategory)
      return "eSCategory";
   else if  (type_ == EType::eLCategory)
      return "eLCategory";
   else if  (type_ == EType::eUndefined)
      return "eUndefined";

   return "";
}

//-----------------------------------------------------------------------------------------
Node::EType Node::Str2Type(const std::string& type_)
{
   if       (type_ == "eObject")
      return EType::eObject;
   else if  (type_ == "eSCategory")
      return EType::eSCategory;
   else if  (type_ == "eLCategory")
      return EType::eLCategory;
   else if  (type_ == "eUndefined")
      return EType::eUndefined;

   return EType::eUndefined;
}

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
   if (type_ == EType::eUndefined)
      throw std::runtime_error("EType::eUndefined is not allowed as a node type");
}

//-----------------------------------------------------------------------------------------
bool Node::AddArrow(const Arrow& arrow_)
{
   bool constraint =
   (m_type == EType::eObject     ) && (arrow_.Type() == Arrow::EType::eFunction  ) ||
   (m_type == EType::eSCategory  ) && (arrow_.Type() == Arrow::EType::eMorphism  ) ||
   (m_type == EType::eLCategory  ) && (arrow_.Type() == Arrow::EType::eFunctor   );

   if (!constraint)
   {
      print_error("Incompatible type of arrows");
      return false;
   }

   if (!QueryArrows(arrow_.AsQuery()).empty())
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
size_t Node::CountArrows() const
{
   return m_arrows.size();
}

//-----------------------------------------------------------------------------------------
bool Node::AddNode(const Node& node_)
{
   if (node_.Name().empty())
      return false;

   if (m_nodes.find(node_) == m_nodes.end())
   {
      m_nodes[node_];

      Arrow func(node_, node_, Arrow::IdArrowName(node_.Name()));

      for (const auto& id : node_.QueryNodes("*"))
         func.AddArrow(Arrow(id, id));

      if (!AddArrow(func))
         return false;
   }
   else
   {
      print_error("Redefinition of " + Node::Type2Name(node_.Type()) + ": " + node_.Name());
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
void Node::ReplaceNode(const Node& node_)
{
   auto it = m_nodes.find(node_);
   if (it != m_nodes.end())
   {
      auto codomain = it->second;

      codomain.erase(node_);

      codomain.insert(node_);

      m_nodes.erase(it);

      m_nodes[node_] = codomain;
   }
   else
      AddNode(node_);
}

//-----------------------------------------------------------------------------------------
void Node::EraseNodes()
{
   m_nodes .clear();
   m_arrows.clear();
}

//-----------------------------------------------------------------------------------------
bool Node::IsNodesEmpty() const
{
   return m_nodes.empty();
}

//-----------------------------------------------------------------------------------------
size_t Node::CountNodes() const
{
   return m_nodes.size();
}

//-----------------------------------------------------------------------------------------
void Node::CloneNode(const NName& old_, const NName& new_)
{
   Node::List nodes = QueryNodes(old_);
   if (nodes.size() != 1)
      return;

   Node node = nodes.front();
   node.SetName(new_);

   AddNode(node);

   std::string sAny(1, ASTERISK::id);

   // outward
   {
      Arrow::List arrows = QueryArrows(Arrow(InternalArrow(), old_, sAny, sAny).AsQuery());

      for (Arrow& arrow : arrows)
      {
         if (arrow.Source() == arrow.Target())
            continue;

         arrow.SetSource(new_);

         AddArrow(arrow);
      }
   }

   // inward
   {
      Arrow::List arrows = QueryArrows(Arrow(InternalArrow(), sAny, old_, sAny).AsQuery());

      for (Arrow& arrow : arrows)
      {
         if (arrow.Source() == arrow.Target())
            continue;

         arrow.SetTarget(new_);

         AddArrow(arrow);
      }
   }
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

   std::string sAny(1, ASTERISK::id);

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
Node Node::Query(const std::string& query_, std::optional<size_t> matchCount_) const
{
   if (matchCount_ && matchCount_ == 0)
      return Node("", Node::EType::eUndefined);

   auto qarrow = get_arrows(query_, InternalArrow(), Node::List(), Node::List(), false);
   if (qarrow.size() != 1)
      return Node("", Node::EType::eUndefined);

   Node ret(m_name, m_type);
   size_t counter {};

   const auto& source = qarrow[0].Source();
   const auto& target = qarrow[0].Target();
   const auto& name   = qarrow[0].Name  ();

   std::string sAny(1, ASTERISK::id);

   bool name_check = name != sAny;

   if       (source == sAny && target == sAny)
   {
      if (!name_check && !matchCount_)
         return *this;

      for (const Arrow& arrow : m_arrows)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (ret.QueryNodes(arrow.Source()).empty())
         {
            auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
            ret.AddNode(it->first);
         }

         if (ret.QueryNodes(arrow.Target()).empty())
         {
            auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
            ret.AddNode(it->first);
         }

         ret.AddArrow(arrow);

         if (matchCount_ && ++counter == matchCount_)
            break;
      }
   }
   else if  (source != sAny && target == sAny)
   {
      for (const Arrow& arrow : m_arrows)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Source() == source)
         {
            if (ret.QueryNodes(arrow.Source()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
               ret.AddNode(it->first);
            }

            if (ret.QueryNodes(arrow.Target()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
               ret.AddNode(it->first);
            }

            ret.AddArrow(arrow);

            if (matchCount_ && ++counter == matchCount_)
               break;
         }
      }
   }
   else if  (source == sAny && target != sAny)
   {
      for (const auto& arrow : m_arrows)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Target() == target)
         {
            if (ret.QueryNodes(arrow.Source()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
               ret.AddNode(it->first);
            }

            if (ret.QueryNodes(arrow.Target()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
               ret.AddNode(it->first);
            }

            ret.AddArrow(arrow);

            if (matchCount_ && ++counter == matchCount_)
               break;
         }
      }
   }
   else if  (source != sAny && target != sAny)
   {
      for (const auto& arrow : m_arrows)
      {
         if (name_check && arrow.Name() != name)
            continue;

         if (arrow.Source() == source && arrow.Target() == target)
         {
            if (ret.QueryNodes(arrow.Source()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Source(), InternalNode()));
               ret.AddNode(it->first);
            }

            if (ret.QueryNodes(arrow.Target()).empty())
            {
               auto it = m_nodes.find(Node(arrow.Target(), InternalNode()));
               ret.AddNode(it->first);
            }

            ret.AddArrow(arrow);

            if (matchCount_ && ++counter == matchCount_)
               break;
         }
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
bool Node::Verify(const Arrow& arrow_) const
{
   auto source_node = Node(arrow_.Source(), InternalNode());
   auto target_node = Node(arrow_.Target(), InternalNode());

   auto itSourceCat = m_nodes.find(source_node);
   auto itTargetCat = m_nodes.find(target_node);

   if (itSourceCat == m_nodes.end())
   {
      print_error("No such source " + Node::Type2Name(source_node.Type()) + ": " + arrow_.Source());
      return false;
   }

   if (itTargetCat == m_nodes.end())
   {
      print_error("No such target " + Node::Type2Name(target_node.Type()) + ": " + arrow_.Target());
      return false;
   }

   if (arrow_.Type() == Arrow::EType::eMorphism || arrow_.Type() == Arrow::EType::eFunction)
      return true;

   const auto& [source_cat, _s] = *itSourceCat;
   const auto& [target_cat, _t] = *itTargetCat;

   using TSource2Arrow = std::set<std::pair<Node::NName, Arrow::AName>>;
   TSource2Arrow visited;

   for (const Arrow& arrow : arrow_.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()))
   {
      auto head = TSource2Arrow::value_type(arrow.Source(), arrow.Name());

      auto itv = visited.find(head);
      if (itv != visited.end())
      {
         auto msg = "Functor: " + arrow_.Name() + " : ";
         msg += "Mapping the same object " + arrow.Source() + " multiple times with morphism " + arrow.Name();
         print_error(msg);
         return false;
      }

      visited.insert(head);

      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("Missing object for " + arrow.Source() + " to " + arrow.Target());
         return false;
      }
   }

   std::string sAny(1, ASTERISK::id);

   // Checking mapping of objects
   for (const auto& obj : source_cat.QueryNodes(sAny))
   {
      if (!arrow_.SingleMap(obj))
      {
         print_error("Failure to map " + Node::Type2Name(obj.Type()) + ": " + obj.Name());
         return false;
      }
   }

   for (const Arrow& arrow : source_cat.QueryArrows(Arrow(Arrow::EType::eMorphism, "*", "*").AsQuery()))
   {
      auto mapped_source = Node(arrow.Source(), source_cat.InternalNode());
      auto mapped_target = Node(arrow.Target(), source_cat.InternalNode());

      auto objs = arrow_.SingleMap(mapped_source);
      auto objt = arrow_.SingleMap(mapped_target);

      if (!objs)
      {
         print_error("Failure to map " + Node::Type2Name(mapped_source.Type()) + " " + mapped_source.Name());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Source()).empty())
      {
         print_error("No such " + Node::Type2Name(mapped_source.Type()) + " " + mapped_source.Name() + " in " + Node::Type2Name(source_cat.Type()) + " " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objs->Name()).empty())
      {
         print_error("No such " + Node::Type2Name(objs->Type()) + " " + objs->Name() + " in " + Node::Type2Name(target_cat.Type()) + " " + target_cat.Name());
         return false;
      }

      if (!objt)
      {
         print_error("Failure to map " + Node::Type2Name(mapped_target.Type()) + " " + mapped_target.Name());
         return false;
      }

      if (source_cat.QueryNodes(arrow.Target()).empty())
      {
         print_error("No such " + Node::Type2Name(mapped_target.Type()) + " " + mapped_target.Name() + " in " + Node::Type2Name(source_cat.Type()) + " " + source_cat.Name());
         return false;
      }

      if (target_cat.QueryNodes(objt->Name()).empty())
      {
         print_error("No such " + Node::Type2Name(objt->Type()) + " " + objt->Name() + " in " + Node::Type2Name(target_cat.Type()) + " " + target_cat.Name());
         return false;
      }

      // Checking mapping of arrows
      if (target_cat.QueryArrows(Arrow(Arrow::EType::eMorphism, objs->Name(), objt->Name()).AsQuery()).empty())
      {
         print_error("Failure to match morphism: " + objs->Name() + " to " + objt->Name());
         return false;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------------------
void Node::SetName(const NName& name_)
{
   m_name = name_;
}

//-----------------------------------------------------------------------------------------
const Node::NName& Node::Name() const
{
   return m_name;
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
         AddArrow(Arrow(domain, codomain_node));
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
std::list<Node::NName> Node::SolveSequence(const Node::NName& from_, const Node::NName& to_, std::optional<size_t> length_) const
{
   std::list<Node::NName> ret;

   std::list<Node::PairSet> stack;

   std::optional<Node::NName> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         bool pass = !length_ || (length_ && length_ == stack.size() + 1);

         if (pass)
         {
            for (auto & [nodei, _] : stack)
               ret.push_back(nodei.Name());

            ret.push_back(current_node.value());

            return ret;
         }
      }

      stack.emplace_back(Node(current_node.value(), InternalNode()), m_nodes.at(Node(current_node.value(), InternalNode())));

      // Remove identity morphism
      stack.back().second.erase(Node(current_node.value(), InternalNode()));

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
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value().Name());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node.Name() == current_node.value())
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
std::list<std::list<Node::NName>> Node::SolveSequences(const Node::NName& from_, const Node::NName& to_, std::optional<size_t> length_) const
{
   std::list<std::list<Node::NName>> ret;

   std::list<Node::PairSet> stack;

   std::optional<Node::NName> current_node(from_);

   while (true)
   {
      // Checking for destination
      if (current_node.value() == to_)
      {
         std::list<Node::NName> seq;

         bool pass = !length_ || (length_ && length_ == stack.size() + 1);

         if (pass)
         {
            for (auto& [nodei, _] : stack)
               seq.push_back(nodei.Name());

            seq.push_back(current_node.value());

            ret.push_back(seq);
         }
      }
      else
      {
         // Stacking forward movements
         stack.emplace_back(Node(current_node.value(), InternalNode()), m_nodes.at(Node(current_node.value(), InternalNode())));

         // Removing identity morphism
         stack.back().second.erase(Node(current_node.value(), InternalNode()));
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
         current_node.emplace(forward_codomain.extract(forward_codomain.begin()).value().Name());

         // Checking for loops
         for (const auto& [node, _] : stack)
         {
            // Is already visited
            if (node.Name() == current_node.value())
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
Arrow::List Node::MapNodes2Arrows(const std::list<Node::NName>& nodes_) const
{
   Arrow::List ret;

   auto it_last = std::prev(nodes_.end());

   for (auto itn = nodes_.begin(); itn != it_last; ++itn)
   {
      auto it = std::find_if(m_arrows.begin(), m_arrows.end(), [&](const Arrow::List::value_type& elem_)
      {
         return *itn == elem_.Source() && *std::next(itn) == elem_.Target();
      });

      if (it != m_arrows.end())
         ret.push_back(*it);
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
   else if  (m_type == EType::eObject)
      return EType::eSet;
   else
      return EType::eUndefined;
}

//-----------------------------------------------------------------------------------------
Arrow::EType Node::InternalArrow() const
{
   return m_type == Node::EType::eLCategory ? Arrow::EType::eFunctor : Arrow::EType::eMorphism;
}

//-----------------------------------------------------------------------------------------
Arrow::EType Node::ExternalArrow() const
{
   if       (m_type == Node::EType::eSet)
      return Arrow::EType::eFunction;
   else if  (m_type == Node::EType::eObject)
      return Arrow::EType::eMorphism;
   else if  (m_type == Node::EType::eSCategory)
      return Arrow::EType::eFunctor;
   else if  (m_type == Node::EType::eLCategory)
      return Arrow::EType::eUndefined;

   return Arrow::EType::eUndefined;
}

//-----------------------------------------------------------------------------------------
void Node::SetValue(const TSetValue& value_)
{
   m_value = value_;
}

//-----------------------------------------------------------------------------------------
const TSetValue& Node::GetValue() const
{
   return m_value;
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
//-----------------------------------------------------------------------------------------
std::size_t NodeKeyHasher::operator()(const Node& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
