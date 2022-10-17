#include "../include/viewGraph.hpp"



viewGraph::viewGraph(){};
viewGraph::~viewGraph(){};




//Will be used to generate a 3D graph displaying X in relation to Y and Z
//Will return a pointer to a canvas
//!!!!Drawing will close when the program ends!!!!

std::shared_ptr<TCanvas> viewGraph::drawTrack(std::vector<dataXYZ> &data_xyz)
{

    std::shared_ptr<TCanvas> canvas_xyz = std::make_shared<TCanvas>("my_canvas", "Graph for XYZ axes", 0, 0, 30000, 30000);

/*     std::unique_ptr<TGraph2D> graph_xyz = std::make_unique<TGraph2D>(data_xyz.size());

    for(auto i = 0; i < data_xyz.size(); i++){

        graph_xyz->SetPoint(i, data_xyz.at(i).data_x, data_xyz.at(i).data_y, data_xyz.at(i).data_z);

    }

    graph_xyz->Draw("surf1"); */


    std::unique_ptr<TPolyMarker3D> marker_xyz = std::make_unique<TPolyMarker3D>(data_xyz.size(), 1);

    for(auto i = 0; i < data_xyz.size(); i++){

        marker_xyz->SetPoint(i, data_xyz.at(i).data_x, data_xyz.at(i).data_y, data_xyz.at(i).data_z);

    }

    marker_xyz->Draw();




    canvas_xyz->Update();



    return canvas_xyz;

}