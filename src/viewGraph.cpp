#include "../include/viewGraph.hpp"



viewGraph::viewGraph(){};
viewGraph::~viewGraph(){};

std::shared_ptr<TCanvas> viewGraph::drawGraph(std::vector<dataXYZ> &data_xyz)
{

    std::shared_ptr<TCanvas> canvas_xyz = std::make_shared<TCanvas>("my_canvas", "Graph for XYZ axes", 0, 0, 30000, 30000);

    std::unique_ptr<TGraph2D> graph_xyz = std::make_unique<TGraph2D>(data_xyz.size());

    for(auto i = 0; i < data_xyz.size(); i++){

        graph_xyz->SetPoint(i, data_xyz.at(i).data_x, data_xyz.at(i).data_y, data_xyz.at(i).data_z);

    }

    graph_xyz->Draw("surf1");

    return canvas_xyz;

}