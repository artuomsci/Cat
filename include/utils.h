#pragma once

#include "cat.h"

namespace cat
{
   using TVec2    = std::pair<int, int>;
   using TCoords  = std::map<Obj, TVec2>;

   /**
    * @brief Export data to Cytoscape
    * @param cat_ - export category
    * @param path_ - export folder path
    * @param prefix_ - file name prefix
    * @param coords_ - object coordinates (optional)
    * @param skip_identity_ - flag to skip identity morphisms
    * @param show_morphisms_ - flag to show morphisms as objects
    */
   CAT_EXPORT void export_cytoscape(const Cat& cat_, const std::string& path_, const std::string& prefix_, const TCoords& coords_, bool skip_identity_ = true, bool show_morphisms_ = false);
}
