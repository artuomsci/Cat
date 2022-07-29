#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "cat_export.h"

#include "cacat.h"

struct CAT_EXPORT SParser
{
   SParser(const std::string& file_path_);

   /**
   * @brief Parse file
   * @param ccat_ - category of categories
   * @return True if file was successfully parsed
   */
   bool parse(cat::CACat& ccat_);

private:
   /**
    * @brief Load source file
    * @param path_ - path to source file
    * @param ccat_ - category of categories
    * @return True if file was successfully loaded
    */
   bool load_source(const std::string& path_, cat::CACat& ccat_);

   /**
    * @brief Parse the contents of string
    * @param source_ - string to parse
    * @param ccat_ - category of categories
    * @return True if string was successfully parsed
    */
   bool parse_source(const std::string& source_, cat::CACat& ccat_);

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
 * @brief Find any sequence of objects between two given objects
 * @param cat_ - category to find sequence of objects in
 * @param from_ - initial object of sequence
 * @param to_ - terminal object of sequence
 * @return Sequence of objects
 */
CAT_EXPORT cat::ObjVec solve_sequence(const cat::Cat& cat_, const cat::Obj& from_, const cat::Obj& to_);

/**
 * @brief Find all sequences of objects between two given objects
 * @param cat_ - category to find sequences of objects in
 * @param from_ - initial object of sequences
 * @param to_ - terminal object of sequences
 * @return Sequences of objects
 */
CAT_EXPORT std::vector<cat::ObjVec> solve_sequences(const cat::Cat& cat_, const cat::Obj& from_, const cat::Obj& to_);

/**
 * @brief Map sequence of objects onto sequence of morphisms
 * @param objs_ - sequence of objects
 * @param cat_ - category to find morphisms in
 * @return Sequence of morphisms
 */
CAT_EXPORT std::vector<cat::Morph> map_obj2morphism(const cat::ObjVec& objs_, const cat::Cat& cat_);

/**
 * @brief Find all compositions
 * @param cat_ - category to find compositions in
 */
CAT_EXPORT void solve_compositions(cat::Cat& cat_);

/**
 * @brief Inverse category morphisms
 * @param cat_ - category to inverse morphisms
 */
CAT_EXPORT void inverse(cat::Cat& cat_);

/**
 * @brief Find initial objects. All morphism compositions
 * must be resolved before calling this method i.e. call "solve_compositions" first
 * @param cat_ - category to find initial objects in
 * @return Initial objects
 */
CAT_EXPORT cat::ObjVec initial(cat::Cat& cat_);

/**
 * @brief Find terminal objects. All morphism compositions
 * must be resolved before calling this method i.e. call "solve_compositions" first
 * @param cat_ - category to find terminal objects in
 * @return Terminal objects
 */
CAT_EXPORT cat::ObjVec terminal(cat::Cat& cat_);

/**
 * @brief Coproduct
 * @param fst_ - first object
 * @param snd_ - second object
 * @return Coproduct result
 */
CAT_EXPORT std::optional<cat::Obj> coproduct(cat::Obj& fst_, cat::Obj& snd_);

/**
 * @brief Product
 * @param fst_ - first object
 * @param snd_ - second object
 * @return Product result
 */
CAT_EXPORT std::optional<cat::Obj> product(cat::Obj& fst_, cat::Obj& snd_);
