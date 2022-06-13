#pragma once

#include "cat.h"

#include <fstream>

namespace cat
{
   /**
    * @brief Export data to Cytoscape
    * @param cat_ - export category
    * @param path_ - export path
    * @param skip_identity_ - flag to skip identity morphisms
    * @param show_morphisms_ - flag to show morphisms as objects
    */
   inline void export_cytoscape(const Cat& cat_, const std::string& path_, bool skip_identity_ = true, bool show_morphisms_ = false)
   {
      std::string srctemplate = R"(<!DOCTYPE>
            <html>

              <head>
                <title>Graph</title>
                <script src="cytoscape.min.js"></script>

                <style>
                  #cy {
                  height: 100%;
                  width: 100%;
                  position: absolute;
                  left: 0;
                  top: 0;
                  float: left;
                  }
                </style>

              </head>

              <body>
                <div id="cy"></div>
                <script>
                        let data = {};
                        var cy = cytoscape({
                            container: document.getElementById('cy'),
                            elements: data,
                            style: [
                                    {
                                    selector: 'node[type="Obj"][name]',
                                       style:
                                       {
                                          'content': 'data(name)'
                                       }
                                    },
                                    {
                                    selector: 'node[type="Morph"][name]',
                                       style:
                                       {
                                          'content': 'data(name)',
                                          'text-valign': 'center',
                                          'text-halign': 'center',
                                          'background-color': 'white'
                                       }
                                    },
                                    {
                                    selector: 'edge',
                                       style:
                                       {
                                          'curve-style': 'bezier',
                                          'target-arrow-shape': 'triangle'
                                       }
                                    }
                            ],
                            layout: {
                                name: 'breadthfirst'
                            }
                        });

                </script>
              </body>

            </html>)";

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

      // aggregate data into string
      std::string data = "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";

      // filling template
      auto ind = srctemplate.find("{}");
      srctemplate.insert(ind + 1, data);

      // file dumping
      std::ofstream file(path_ + "/index.html", std::ofstream::out);
      if (file.is_open())
      {
         file << srctemplate;
         file.close();
      }
   }
}
