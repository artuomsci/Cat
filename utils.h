#pragma once

#include "cat.h"

namespace cat
{
   /**
    * @brief Export data to Cytoscape
    * @param cat_ - export category
    * @param skip_identity_ - flag to skip identity morphisms
    * @param show_morphisms_ - flag to show morphisms as objects
    * @return Cytoscape string data representation
    */
   inline std::string export_cytoscape(const Cat& cat_, bool skip_identity_ = true, bool show_morphisms_ = false)
   {
      std::string nodes;

      for (const auto& [obj, objset] : cat_.GetObjects())
      {
         // nodes
         char buffern[256];
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", obj.GetName().c_str(), obj.GetName().c_str(), "Obj");

         nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
      }

      if (show_morphisms_)
      {
         for (const MorphDef& mrph : cat_.GetMorphisms())
         {
            if (skip_identity_ && mrph.start == mrph.end)
               continue;

            // nodes
            char buffern[256];
            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", mrph.morph_name.c_str(), mrph.morph_name.c_str(), "Morph");

            nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
         }
      }

      std::string edges;

      if (!show_morphisms_)
      {
         for (const auto& [obj, objset] : cat_.GetObjects())
         {
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
      }
      else
      {
         for (const MorphDef& mrph : cat_.GetMorphisms())
         {
            if (skip_identity_ & mrph.start == mrph.end)
               continue;

            // edges
            char buffere[256];

            sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.start.GetName().c_str(), mrph.morph_name.c_str());

            edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";

            sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.morph_name.c_str(), mrph.end.GetName().c_str());

            edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
         }
      }

      return "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";
   }
}
