#pragma once

#include "cat.h"
#include "cacat.h"

namespace cat
{
   using TVec2    = std::pair<int, int>;
   template <typename T>
   using TCoords  = std::map<T, TVec2>;

   /**
    * @brief Export category data to Cytoscape
    * @param cat_ - export category
    * @param path_ - export folder path
    * @param prefix_ - file name prefix
    * @param coords_ - object coordinates (optional)
    * @param skip_identity_ - flag to skip identity morphisms
    * @param show_morphisms_ - flag to show morphisms as nodes
    */
   CAT_EXPORT void export_cytoscape(const Cat& cat_, const std::string& path_, const std::string& prefix_, const TCoords<Obj>& coords_, bool skip_identity_ = true, bool show_morphisms_ = false);

   /**
    * @brief Export category of categories data to Cytoscape
    * @param ccat_ - category of categories
    * @param path_ - export folder path
    * @param prefix_ - file name prefix
    * @param coords_ - category coordinates (optional)
    * @param show_functors_ - flag to show functors as nodes
    */
   CAT_EXPORT void export_cytoscape(const CACat& ccat_, const std::string& path_, const std::string& prefix_, const TCoords<Cat>& coords_, bool show_functors_ = false);
}
