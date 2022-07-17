#include "utils.h"

#include <fstream>

using namespace cat;

static std::string get_template()
{
   return R"(<!DOCTYPE>
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
                              selector: 'node[type="Node"][name]',
                                 style:
                                 {
                                    'content': 'data(name)'
                                 }
                              },
                              {
                              selector: 'node[type="Link"][name]',
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
}

void cat::export_cytoscape(const Cat& cat_, const std::string& path_, const std::string& prefix_, const TCoords<Obj>& coords_, bool skip_identity_, bool show_morphisms_)
{
   std::string nodes;

   for (const auto& [obj, objset] : cat_.GetObjects())
   {
      // nodes
      char buffern[1024];
      if (coords_.empty())
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", obj.GetName().c_str(), obj.GetName().c_str(), "Node");
      else
      {
         const TVec2& crd = coords_.at(obj);
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", obj.GetName().c_str(), obj.GetName().c_str(), "Node", crd.first, crd.second);
      }

      nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
   }

   if (show_morphisms_)
   {
      for (const Morph& mrph : cat_.GetMorphisms())
      {
         if (skip_identity_ && mrph.source == mrph.target)
            continue;

         // nodes
         char buffern[1024];
         if (coords_.empty())
            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", mrph.name.c_str(), mrph.name.c_str(), "Link");
         else
         {
            const TVec2& source_crd = coords_.at(mrph.source);
            const TVec2& target_crd = coords_.at(mrph.target);

            int x = (source_crd.first + target_crd.first) * 0.5;
            int y = (source_crd.second + target_crd.second) * 0.5;

            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", mrph.name.c_str(), mrph.name.c_str(), "Link", x, y);
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
            char buffere[1024];
            sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", source.GetName().c_str(), target.GetName().c_str());

            edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
         }
      }
   }
   else
   {
      for (const Morph& mrph : cat_.GetMorphisms())
      {
         if (skip_identity_ & mrph.source == mrph.target)
            continue;

         // edges
         char buffere[1024];

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.source.GetName().c_str(), mrph.name.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", mrph.name.c_str(), mrph.target.GetName().c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
      }
   }

   // aggregate data into string
   std::string data = "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";

   std::string srctemplate = get_template();
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

void cat::export_cytoscape(const CACat& ccat_, const std::string& path_, const std::string& prefix_, const TCoords<Cat>& coords_, bool show_functors_)
{
   std::string nodes;

   for (const auto& [cat, _] : ccat_.Categories())
   {
      // nodes
      char buffern[1024];
      if (coords_.empty())
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", cat.GetName().c_str(), cat.GetName().c_str(), "Node");
      else
      {
         const TVec2& crd = coords_.at(cat);
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", cat.GetName().c_str(), cat.GetName().c_str(), "Node", crd.first, crd.second);
      }

      nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
   }

   if (show_functors_)
   {
      for (const Func& func : ccat_.Functors())
      {
         // nodes
         char buffern[1024];
         if (coords_.empty())
            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", func.name.c_str(), func.name.c_str(), "Link");
         else
         {
            const TVec2& source_crd = coords_.at(Cat(func.source));
            const TVec2& target_crd = coords_.at(Cat(func.target));

            int x = (source_crd.first + target_crd.first) * 0.5;
            int y = (source_crd.second + target_crd.second) * 0.5;

            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", func.name.c_str(), func.name.c_str(), "Link", x, y);
         }

         nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
      }
   }

   std::string edges;

   if (!show_functors_)
   {
      for (const Func& func : ccat_.Functors())
      {
         // edges
         char buffere[1024];
         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", func.source.c_str(), func.target.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
      }
   }
   else
   {
      for (const Func& func : ccat_.Functors())
      {
         // edges
         char buffere[1024];

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", func.source.c_str(), func.name.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", func.name.c_str(), func.target.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
      }
   }

   // aggregate data into string
   std::string data = "nodes: [" + nodes + "]" + "," + "edges: [" + edges + "]";

   std::string srctemplate = get_template();
   // filling template
   auto ind = srctemplate.find("$(title)");
   srctemplate.replace(ind, std::string("$(title)").length(), "CACat");

   ind = srctemplate.find("$(data)");
   srctemplate.replace(ind, std::string("$(data)").length(), data);

   ind = srctemplate.find("$(pattern)");
   srctemplate.replace(ind, std::string("$(pattern)").length(), coords_.empty() ? "circle" : "preset");

   // file dumping
   std::ofstream file(path_ + "/" + prefix_ + "CACat" +  ".html", std::ofstream::out);
   if (file.is_open())
   {
      file << srctemplate;
      file.close();
   }
}
