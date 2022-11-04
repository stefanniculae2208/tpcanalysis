#include <iostream>



#include "src/loadData.cpp"
#include "src/convertUVW.cpp"
#include "src/convertXYZ.cpp"

#include "include/generalDataStorage.hpp"



#include "TSystem.h"
#include "TROOT.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TSpectrum.h"



// From GET
/* #include "dict/include/GDataSample.h"
#include "dict/include/GDataChannel.h"
#include "dict/include/GFrameHeader.h"
#include "dict/include/GDataFrame.h" */

#include "dict/src/GDataSample.cpp"
#include "dict/src/GDataChannel.cpp"
#include "dict/src/GFrameHeader.cpp"
#include "dict/src/GDataFrame.cpp"



void test_openFile()
{

    auto noFile = "./rootdata/stefan23214124.root";
    auto goodFile = "./rootdata/stefan2.root";
    loadData no_data_loader(noFile, "tree_data");

    auto testNoFile = no_data_loader.openFile();

    if(testNoFile.get() != nullptr) {
        std::cerr << "Error: loadData try to read empty file, but not return error\n";
    }


    loadData good_data_loader(goodFile, "tree_data");

    auto testGoodFile = good_data_loader.openFile();

    if(testGoodFile.get() == nullptr){
        std::cerr << "Error opening good file\n";
    }


}

void test_readData()
{

    auto noFile = "./rootdata/stefan23214124.root";
    auto goodFile = "./rootdata/data.root";


    auto badTree = "bad_tree_name";
    auto goodTree = "tree";


    loadData noFileNoTree(noFile, badTree);
    loadData goodFileBadTree(goodFile, badTree);
    loadData goodFileGoodTree(goodFile, goodTree);


    auto testNoFile = noFileNoTree.openFile();
    auto testBadTree = goodFileBadTree.openFile();
    auto testGoodTree = goodFileGoodTree.openFile();


    auto err = noFileNoTree.readData();


    if(err != -2)
        std::cerr << "Error: file not existing did not return good error code: " << err << std::endl;

    err = goodFileBadTree.readData();

    if(err != -1)
        std::cerr << "Error: bad tree returned bad error code: " << err << std::endl;

    err = goodFileGoodTree.readData();

    if(err != 0)
        std::cerr << "Error: good tree returned bad code: " << err << std::endl;



}


void test_decodeData()
{

    auto noFile = "./rootdata/stefan23214124.root";
    auto goodFile = "./rootdata/data.root";


    auto badTree = "bad_tree_name";
    auto goodTree = "tree";


    loadData noFileNoTree(noFile, badTree);
    loadData goodFileBadTree(goodFile, badTree);
    loadData goodFileGoodTree(goodFile, goodTree);


    auto testNoFile = noFileNoTree.openFile();
    auto testBadTree = goodFileBadTree.openFile();
    auto testGoodTree = goodFileGoodTree.openFile();


    auto err = noFileNoTree.readData();

    err = goodFileBadTree.readData();

    err = goodFileGoodTree.readData();



    generalDataStorage nofile_data_container;
    generalDataStorage bad_data_container;
    generalDataStorage good_data_container;


    err = noFileNoTree.decodeData(nofile_data_container.root_raw_data);


    if(err != -2)
        std::cerr << "Error: file not existing did not return good error code: " << err << std::endl;

    err = goodFileBadTree.decodeData(bad_data_container.root_raw_data);

    if(err != -1)
        std::cerr << "Error: bad tree returned bad error code: " << err << std::endl;

    err = goodFileGoodTree.decodeData(good_data_container.root_raw_data);

    if(err != 0)
        std::cerr << "Error: good tree returned bad code: " << err << std::endl;
    else   
        std::cout << "Success! Size of vector is " << good_data_container.root_raw_data.size() << std::endl;

    

    for(auto i : good_data_container.root_raw_data){

        std::cout << "Channel is " << i.ch_nr << " chip is " << i.chip_nr << " signal value size is " << i.signal_val.size() <<std::endl;

    }



}





void test_loadData()
{

/*     test_openFile();

    test_readData(); */

    test_decodeData();

}



void test_convertUVW()
{

    auto goodFile = "./rootdata/data.root";

    auto goodTree = "tree";


    generalDataStorage data_container;


    loadData good_data(goodFile, goodTree);

    auto returned_file = good_data.openFile();
    auto err = good_data.readData();
    err = good_data.decodeData(data_container.root_raw_data);

    convertUVW loc_converter;

    err = loc_converter.openSpecFile();

    if(err != 0)
        return;

    err = loc_converter.makeConversion(data_container.root_raw_data);

    for(auto i : data_container.root_raw_data){

        std::cout << "Channel is " << i.ch_nr << " chip is " << i.chip_nr << " signal value size is " << i.signal_val.size() <<
        " decoded plane value is " << i.plane_val << " decoded strip nr is " << i.strip_nr <<std::endl;

    }

}


void test_viewdata()
{


    convertXYZ loc_conv_xyz;
    auto err = loc_conv_xyz.buildArray();

    convertUVW loc_conv_uvw;

    generalDataStorage data_container;




    auto goodFile = "./rootdata/data.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto returned_file = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(data_container.root_raw_data);



    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion(data_container.root_raw_data);



    err = loc_conv_uvw.substractBl(data_container.root_raw_data);



    err = loc_conv_xyz.makeConversion(data_container.root_raw_data, data_container.converted_data);




    TCanvas *print_canv = new TCanvas("Test_canvas", "Test_canvas", 1000, 1000);

    //std::vector<TH1D*> hist_container;

    TH2D *raw_hist = new TH2D("my_hist", "hist", 512, 1, 513, 300, 1, 301);
    raw_hist->SetDirectory(0);


    loc_conv_uvw.getHitInfo(data_container.root_raw_data, data_container.hit_data, data_container.raw_hist_container);


/* 
    int bin = 0;
    int strip = 1;

    for(auto iter : data_container.root_raw_data){

        auto loc_hist = new TH1D(Form("hist_at_strip%d_plane%d", iter.strip_nr, iter.plane_val), Form("hist%d", strip), 512, 1, 512);

        //if(iter.plane_val == 2 && iter.strip_nr == 45)
        for(auto sig_iter : iter.signal_val){
            raw_hist->SetBinContent(++bin, strip, sig_iter);
            loc_hist->SetBinContent(bin, sig_iter);
        }

        data_container.raw_hist_container.push_back(loc_hist);

        bin = 0;
        strip++;
    }
 */
    /* raw_hist->Fit("gaus");

    auto mean = raw_hist->GetFunction("gaus")->GetParameter(1);
    auto sigma = raw_hist->GetFunction("gaus")->GetParameter(2);

    std::cout<<"value is "<<mean<<std::endl;
    std::cout<<"sigma is "<<sigma<<std::endl; */

    //raw_hist->Draw("COLZ");





    



    //auto gaus_and_pol0 = new TF1("gaus_and_pol0", "gaus(0)+pol0(3)", 1, 512);

/*     auto spec_analyzer = new TSpectrum();

    Double_t *pos_holder_x;
    Double_t *pos_holder_y;
    int npeaks;
    hitPoints curr_point;

    int curr_iter = 0;
 */


    print_canv->Print("test_hist01.pdf[");

    for(auto hist_iter : data_container.raw_hist_container){

/*         spec_analyzer->Search(hist_iter, 5, "nodraw", 0.2);
        npeaks = spec_analyzer->GetNPeaks();
        pos_holder_x = spec_analyzer->GetPositionX();
        pos_holder_y = spec_analyzer->GetPositionY();


        std::vector<Double_t> peaks_x;//x of valid peaks
        std::vector<Double_t> peaks_y;//y of valid peaks
        int n_true_peaks = 0;//number of valid peaks
        Double_t peak_th = (double)200;//threshold value for valid peaks

        for(auto i = 0; i < npeaks; i++){
            if(pos_holder_y[i] > peak_th){
                peaks_x.push_back(pos_holder_x[i]);
                peaks_y.push_back(pos_holder_y[i]);
                n_true_peaks++;
            }
        }
                

        TString func_holder("pol0(0)");

        for(auto i = 0; i < n_true_peaks; i++)
            func_holder.Append(Form("+gaus(%d)", (3*i+1)));

        auto gaus_and_pol0 = new TF1("gaus_and_pol0", func_holder.Data(), 1, 512);

        std::cout<<"Function is "<<func_holder.Data()<<std::endl;
        gaus_and_pol0->SetParameter(0, 3);
        for(auto i = 0; i < n_true_peaks; i++){
            gaus_and_pol0->SetParameter((3*i+1), peaks_y.at(i));
            gaus_and_pol0->SetParameter((3*i+2), peaks_x.at(i));
            gaus_and_pol0->SetParameter((3*i+3), 10);
        }

        hist_iter->Fit("gaus_and_pol0", "Q");


        hitPeakInfo curr_peak;

        for(auto i = 0; i < n_true_peaks; i++){
            curr_peak.peak_x = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+2));
            curr_peak.peak_y = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+1));
            curr_peak.fwhm = 2.355 * hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+3));
        }

        curr_point.npeaks = n_true_peaks;
        curr_point.peaks_info.push_back(curr_peak);
        curr_point.plane = data_container.root_raw_data.at(curr_iter).plane_val;
        curr_point.strip = data_container.root_raw_data.at(curr_iter).strip_nr;
        curr_point.base_line = hist_iter->GetFunction("gaus_and_pol0")->GetParameter(0);


        data_container.hit_data.push_back(curr_point);

 */
        hist_iter->Draw();
        print_canv->Print("test_hist01.pdf", "Test title");


    }

    print_canv->Print("test_hist01.pdf]");


/*     for(auto hit_iter : data_container.hit_data){

        std::cout<<"\n\n\n\n"<<hit_iter.npeaks<<" at plane "<<hit_iter.plane<<" and strip "<<hit_iter.strip<<"\n";
        for(auto p_iter : hit_iter.peaks_info){
            std::cout<<"X is "<<p_iter.peak_x<<" Y is "<<p_iter.peak_y<<" fwhm is "<<p_iter.fwhm<<"\n";
        }

    }
 */




}






void test()
{

/*     auto getLib = "dict/build/libMyLib.so";
    gSystem->Load(getLib); */


    //test_loadData();
    //test_convertUVW();
    test_viewdata();








}