#pragma once

#include <map>

#include "node.h"

namespace cat
{
   using TVec2    = std::pair<int, int>;
   using TCoords  = std::map<Node, std::map<Node, TVec2>>;

   /**
    * @brief Export structure
    * @param node_ - export structure
    * @param path_ - export folder path
    * @param prefix_ - file name prefix
    * @param coords_ - nodes coordinates (optional)
    * @param exclude_ - nodes to exclude
    * @param skip_identity_ - flag to skip identity arrows
    * @param show_arrows_ - flag to show arrows as nodes
    */
   CAT_EXPORT void export_cytoscape(const Node& node_, const std::string& path_, const std::string& prefix_, const TCoords& coords_, const Node::Set& exclude_, bool skip_identity_ = true, bool show_arrows_ = false);
}
