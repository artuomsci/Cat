#pragma once

#include "cat.h"

namespace cat
{
   /**
    * @brief Export data to Cytoscape
    * @param cat_ - export category
    * @return Cytoscape string data representation
    */
   inline std::string export_cytoscape(const Cat& cat_, bool skip_identity_)
   {
      std::string nodes;
      std::string edges;
      for (const auto& [obj, objset] : cat_.GetObjects())
      {
         // nodes
         char buffern[256];
         sprintf(buffern, "{ data: { id: '%s', name: '%s' } }", obj.GetName().c_str(), obj.GetName().c_str());

         nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";

         const Obj& source = obj;

         for (const Obj& target : objset)
         {
            if (skip_identity_ && source == target)
               continue;

            // edges
            char buffere[256];
            sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", source.GetName().c_str(), target.GetName().c_str());

            edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
         }
      }

      return "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";
   }
}
