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
