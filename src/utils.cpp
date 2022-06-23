#include "utils.h"

#include <fstream>

using namespace cat;

void cat::export_cytoscape(const Cat& cat_, const std::string& path_, const std::string& prefix_, const TCoords& coords_, bool skip_identity_, bool show_morphisms_)
{
   std::string srctemplate = R"(<!DOCTYPE>
         <html>
           <meta charset="UTF-8">
           <head>
             <title>$(title)</title>
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
                     let data = {$(data)};
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
                             name: '$(pattern)'
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
      if (coords_.empty())
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", obj.GetName().c_str(), obj.GetName().c_str(), "Obj");
      else
      {
         const TVec2& crd = coords_.at(obj);
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", obj.GetName().c_str(), obj.GetName().c_str(), "Obj", crd.first, crd.second);
      }

      nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
   }

   if (show_morphisms_)
   {
      for (const MorphDef& mrph : cat_.GetMorphisms())
      {
         if (skip_identity_ && mrph.source == mrph.target)
            continue;

         // nodes
         char buffern[256];
         if (coords_.empty())
            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", mrph.morph_name.c_str(), mrph.morph_name.c_str(), "Morph");
         else
         {
            const TVec2& source_crd = coords_.at(mrph.source);
            const TVec2& target_crd = coords_.at(mrph.target);

            int x = (source_crd.first + target_crd.first) * 0.5;
            int y = (source_crd.second + target_crd.second) * 0.5;

            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", mrph.morph_name.c_str(), mrph.morph_name.c_str(), "Morph", x, y);
         }

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
         if (skip_identity_ & mrph.source == mrph.target)
            continue;

         // edges
         char buffere[256];

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.source.GetName().c_str(), mrph.morph_name.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.morph_name.c_str(), mrph.target.GetName().c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
      }
   }

   // aggregate data into string
   std::string data = "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";

   // filling template
   auto ind = srctemplate.find("$(title)");
   srctemplate.replace(ind, std::string("$(title)").length(), cat_.GetName());

   ind = srctemplate.find("$(data)");
   srctemplate.replace(ind, std::string("$(data)").length(), data);

   ind = srctemplate.find("$(pattern)");
   srctemplate.replace(ind, std::string("$(pattern)").length(), coords_.empty() ? "circle" : "preset");
   
   // file dumping
   std::ofstream file(path_ + "/" + prefix_ + cat_.GetName() +  ".html", std::ofstream::out);
   if (file.is_open())
   {
      file << srctemplate;
      file.close();
   }
}
