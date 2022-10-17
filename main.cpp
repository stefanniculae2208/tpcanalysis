#include<iostream>
#include<vector>
#include<string>

#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"



#include "../include/convMatData.hpp"
#include "../include/dataXYZ.hpp"
#include "../include/viewGraph.hpp"



int main(int argc, char **argv)
{

    convMatData matrix_data;

    constexpr double driftVelocity = 0.724;

    viewGraph graph_draw;

    std::vector<dataXYZ> data_xyz;

    std::shared_ptr<TCanvas> canvas_xyz;




    matrix_data.setMatrix(driftVelocity);

    matrix_data.printMatrix();






    auto fileName = "./rootdata/stefan2.root";
    TFile *iFile = new TFile(fileName, "RECREATE");

    TTree *data_tree = iFile->Get<TTree>("tree_data");


    int plane_holder = 0;
    int channel_holder = 0;
    int numberData = 100000;
    double signal_holder[100000]{0};

  


    //iFile->GetObject("tree_data", data_tree);




/*     data_tree->SetBranchAddress("plane", &plane_holder);
    data_tree->SetBranchAddress("channel", &channel_holder);
    data_tree->SetBranchAddress("numberData", &numberData);
    data_tree->SetBranchAddress("signal", signal_holder);


    auto nEntries = data_tree->GetEntries();

    data_tree->GetEntry(0);

    std::cout<<"plane: "<<plane_holder<<std::endl<<"channel: "<<channel_holder<<std::endl<<"numberData: "<<numberData<<std::endl;



    data_tree->ResetBranchAddresses();

 */


    //TODO
    //canvas_xyz = graph_draw.drawTrack(data_xyz);




    iFile->Close();



    std::cout<<"Press Enter to continue "<<std::endl;
	getchar();


    return 0;

}