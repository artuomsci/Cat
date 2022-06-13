#pragma once

#include "cat.h"

namespace cat
{
   /**
    * @brief Export data to Cytoscape
    * @param cat_ - export category
    * @param path_ - export folder path
    * @param skip_identity_ - flag to skip identity morphisms
    * @param show_morphisms_ - flag to show morphisms as objects
    */
   CAT_EXPORT void export_cytoscape(const Cat& cat_, const std::string& path_, bool skip_identity_ = true, bool show_morphisms_ = false);
}
