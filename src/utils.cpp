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

template <typename T, typename TNode>
static void export_cytoscape_t(const std::string& name_, const T& ccat_, const std::string& path_, const std::string& prefix_, const TCoords<TNode>& coords_, const std::set<TNode>& exclude_, bool skip_identity_, bool show_arrows_)
{
   std::string nodes;

   for (const auto& [node, _] : ccat_.Nodes())
   {
      if (exclude_.find(node) != exclude_.end())
         continue;

      // nodes
      char buffern[1024];

      auto it_crd = coords_.find(node);
      if (it_crd != coords_.end())
      {
         const TVec2& crd = it_crd->second;
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", node.Name().c_str(), node.Name().c_str(), "Node", crd.first, crd.second);
      }
      else
      {
         sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", node.Name().c_str(), node.Name().c_str(), "Node");
      }

      nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
   }

   if (show_arrows_)
   {
      for (const auto& arrow : ccat_.Arrows())
      {
         if (skip_identity_ && arrow.source == arrow.target)
            continue;

         // nodes
         char buffern[1024];

         auto it_source_crd = coords_.find(TNode(arrow.source));
         auto it_target_crd = coords_.find(TNode(arrow.target));

         if (exclude_.find(TNode(arrow.source)) != exclude_.end())
            continue;

         if (exclude_.find(TNode(arrow.target)) != exclude_.end())
            continue;

         if (it_source_crd != coords_.end() && it_target_crd != coords_.end())
         {
            const TVec2& source_crd = coords_.at(TNode(arrow.source));
            const TVec2& target_crd = coords_.at(TNode(arrow.target));

            int x = (source_crd.first + target_crd.first) * 0.5;
            int y = (source_crd.second + target_crd.second) * 0.5;

            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' }, position: { x: %d, y: %d } }", arrow.name.c_str(), arrow.name.c_str(), "Link", x, y);
         }
         else
         {
            sprintf(buffern, "{ data: { id: '%s', name: '%s', type: '%s' } }", arrow.name.c_str(), arrow.name.c_str(), "Link");
         }

         nodes += (nodes.empty() ? "" : ",") +  std::string(buffern) + "\n";
      }
   }

   std::string edges;

   if (!show_arrows_)
   {
      for (const auto& arrow : ccat_.Arrows())
      {
         if (skip_identity_ && arrow.source == arrow.target)
            continue;

         if (exclude_.find(TNode(arrow.source)) != exclude_.end())
            continue;

         if (exclude_.find(TNode(arrow.target)) != exclude_.end())
            continue;

         // edges
         char buffere[1024];
         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", arrow.source.c_str(), arrow.target.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";
      }
   }
   else
   {
      for (const auto& arrow : ccat_.Arrows())
      {
         if (skip_identity_ && arrow.source == arrow.target)
            continue;

         if (exclude_.find(TNode(arrow.source)) != exclude_.end())
            continue;

         if (exclude_.find(TNode(arrow.target)) != exclude_.end())
            continue;

         // edges
         char buffere[1024];

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", arrow.source.c_str(), arrow.name.c_str());

         edges += (edges.empty() ? "" : ",") +  std::string(buffere) + "\n";

         sprintf(buffere, "{ data: { source: '%s', target: '%s' } }", arrow.name.c_str(), arrow.target.c_str());

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
   std::ofstream file(path_ + "/" + prefix_ + name_ +  ".html", std::ofstream::out);
   if (file.is_open())
   {
      file << srctemplate;
      file.close();
   }
}

void cat::export_cytoscape(const CACat& ccat_, const std::string& path_, const std::string& prefix_, const TCoords<Cat>& coords_, const std::set<Cat>& exclude_, bool skip_identity_, bool show_functors_)
{
   export_cytoscape_t<CACat, Cat>("CACat", ccat_, path_, prefix_, coords_, exclude_, skip_identity_, show_functors_);
}

void cat::export_cytoscape(const Cat& cat_, const std::string& path_, const std::string& prefix_, const TCoords<Obj>& coords_, const std::set<Obj>& exclude_, bool skip_identity_, bool show_morphisms_)
{
   export_cytoscape_t<Cat, Obj>(cat_.Name(), cat_, path_, prefix_, coords_, exclude_, skip_identity_, show_morphisms_);
}
