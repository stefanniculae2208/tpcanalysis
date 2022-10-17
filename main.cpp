#include<iostream>
#include<vector>
#include<string>

#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TApplication.h"



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



    TApplication rApp("Root app", &argc, argv);


    auto fileName = "./rootdata/stefan2.root";
    TFile *iFile = new TFile(fileName, "READ");

    TTree *data_tree = iFile->Get<TTree>("tree_data");


    int plane_holder = 0;
    int channel_holder = 0;
    int numberData = 100000;
    double signal_holder[100000]{0};

  






    data_tree->SetBranchAddress("plane", &plane_holder);
    data_tree->SetBranchAddress("channel", &channel_holder);
    data_tree->SetBranchAddress("numberData", &numberData);
    data_tree->SetBranchAddress("signal", signal_holder);


    auto nEntries = data_tree->GetEntries();


    std::cout<<"Number of entries is "<<nEntries<<std::endl;

    //data_tree->GetEntry(0);

    //std::cout<<"plane: "<<plane_holder<<std::endl<<"channel: "<<channel_holder<<std::endl<<"numberData: "<<numberData<<std::endl;


    TCanvas *canv = new TCanvas("c1","histos");





    
    canv->Divide(2, 2, 0, 0);

    /* TH2D *hist_1 = new TH2D("h1", "hist1", 100, 0, 100, 512, 1, 513);

    for(auto i = 0; i < nEntries; i++){

        data_tree->GetEntry();

        //if(plane_holder = 0)
        hist_1->Fill(channel_holder, signal_holder[100]);

    }

    hist_1->Draw(); */




    TH2D *hist_2_u = new TH2D("h2", "hist2", 512, 1, 513, 100, 0, 100);

    TH2D *hist_2_v = new TH2D("h2", "hist2", 512, 1, 513, 100, 0, 100);

    TH2D *hist_2_w = new TH2D("h2", "hist2", 512, 1, 513, 100, 0, 100);


    for(auto i = 0; i < nEntries/100; i++){

        data_tree->GetEntry(i);


        for(auto j : signal_holder){

            if(plane_holder == 0)
                hist_2_u->Fill(j, channel_holder);
            if(plane_holder == 1)
                hist_2_v->Fill(j, channel_holder);
            if(plane_holder == 2)
                hist_2_w->Fill(j, channel_holder);

        }
    }






    canv->cd(1);
    hist_2_u->Draw("COLZ");

    canv->cd(2);
    hist_2_v->Draw("COLZ");

    canv->cd(3);
    hist_2_w->Draw("COLZ");

	canv->Update();


    rApp.Run();
    rApp.ReturnFromRun();




    //TODO
    //canvas_xyz = graph_draw.drawTrack(data_xyz);







    std::cout<<"Press Enter to continue "<<std::endl;
	getchar();


    delete(canv);

    iFile->Close();


    return 0;

}