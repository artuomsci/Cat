#pragma once

#include <string>
#include <vector>

#include "cat_export.h"

namespace cat
{
   using StringPair  = std::pair<std::string, std::string>;
   using StringVec   = std::vector<std::string>;

   /**
    * @brief Remove specified symbol from string
    * @param string_ - string to remove symbol from
    * @param symbol_ - symbol to remove
    * @return Resulting string
    */
   CAT_EXPORT std::string remove(const std::string& string_, std::string::value_type symbol_);

   /**
    * @brief Remove symbol repetitions
    * @param string_ - string to remove repetitions from
    * @param symbol_ - symbol to remove repetitions
    * @return Resulting string
    */
   CAT_EXPORT std::string remove_rep(const std::string& string_, std::string::value_type symbol_);

   /**
    * @brief Split string by symbol
    * @param string_ - string to split
    * @param symbol_ - symbol to split by
    * @param keep_empty_ - flag to keep empty strings in result
    * @return Splitted string
    */
   CAT_EXPORT StringVec split(const std::string& string_, std::string::value_type symbol_, bool keep_empty_ = true);

   /**
    * @brief Split string by string
    * @param string_ - string to split
    * @param splitter_ - string to split by
    * @param keep_empty_ - flag to keep empty strings in result
    * @return String split into chunks
    */
   CAT_EXPORT StringVec split(const std::string& string_, const std::string& splitter_, bool keep_empty_ = true);
}
