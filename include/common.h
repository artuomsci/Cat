#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "cat_export.h"

#include "node.h"

struct CAT_EXPORT SParser
{
   SParser(const std::string& file_path_);

   /**
   * @brief Parse file
   * @param node_ - node
   * @return True if file was successfully parsed
   */
   bool parse(cat::Node& node_);

private:
   /**
    * @brief Load source file
    * @param path_ - path to source file
    * @param node_ - node
    * @return True if file was successfully loaded
    */
   bool load_source(const std::string& path_, cat::Node& node_);

   /**
    * @brief Parse the contents of string
    * @param source_ - string to parse
    * @param node_ - node
    * @return True if string was successfully parsed
    */
   bool parse_source(const std::string& source_, cat::Node& node_);

private:

   std::filesystem::path m_path;
};

/**
 * @brief Load text file into string
 * @param filename_ - path to file
 * @return String with contents of the file
 */
CAT_EXPORT std::optional<std::string> get_description(const std::string& filename_);

/**
 * @brief Find any sequence of nodes between two given nodes
 * @param node_ - node to find sequence of nodes in
 * @param from_ - source node of the sequence
 * @param to_ - target node of the sequence
 * @return Sequence of nodes
 */
CAT_EXPORT cat::Node::List solve_sequence(const cat::Node& node_, const cat::Node& from_, const cat::Node& to_);

/**
 * @brief Find all sequences of nodes between two given nodes
 * @param node_ - node to find sequences of nodes in
 * @param from_ - source node of the sequences
 * @param to_ - target node of the sequences
 * @return Sequences of nodes
 */
CAT_EXPORT std::vector<cat::Node::List> solve_sequences(const cat::Node& node_, const cat::Node& from_, const cat::Node& to_);

/**
 * @brief Map sequence of nodes onto sequence of arrows
 * @param nodes_ - sequence of nodes
 * @param node_ - node to find arrows in
 * @return Sequence of arrows
 */
CAT_EXPORT cat::Arrow::List map_nodes2arrows(const cat::Node::List& nodes_, const cat::Node& node_);

/**
 * @brief Find all compositions
 * @param nodes_ - node to find compositions in
 */
CAT_EXPORT void solve_compositions(cat::Node& node_);

/**
 * @brief Inverse arrows
 * @param node_ - node to inverse arrows
 */
CAT_EXPORT void inverse(cat::Node& node_);

/**
 * @brief Returns structure matching the mask
 * @param node_ - node for matching
 * @param mask_ - mask
 * @return Matching structure
 */
CAT_EXPORT std::optional<cat::Node> mask(cat::Node& node_, std::string mask_);

/**
 * @brief Find initial nodes. All arrow compositions
 * must be resolved before calling this method i.e. call "solve_compositions" first
 * @param node_ - node to find initial nodes in
 * @return Initial nodes
 */
CAT_EXPORT cat::Node::List initial(cat::Node& node_);

/**
 * @brief Find terminal nodes. All arrow compositions
 * must be resolved before calling this method i.e. call "solve_compositions" first
 * @param node_ - node to find terminal nodes in
 * @return Terminal nodes
 */
CAT_EXPORT cat::Node::List terminal(cat::Node& node_);

///**
// * @brief Coproduct
// * @param fst_ - first object
// * @param snd_ - second object
// * @return Coproduct result
// */
//CAT_EXPORT std::optional<cat::Obj> coproduct(cat::Obj& fst_, cat::Obj& snd_);
//
///**
// * @brief Product
// * @param fst_ - first object
// * @param snd_ - second object
// * @return Product result
// */
//CAT_EXPORT std::optional<cat::Obj> product(cat::Obj& fst_, cat::Obj& snd_);
