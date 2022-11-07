#include <iostream>



#include "src/loadData.cpp"
#include "src/convertUVW.cpp"
#include "src/convertXYZ.cpp"
#include "src/convertHitData.cpp"
#include "include/ErrorCodesMap.hpp"

#include "include/generalDataStorage.hpp"



#include "TSystem.h"
#include "TROOT.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TError.h"



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


    err = noFileNoTree.decodeData();
    nofile_data_container.root_raw_data = noFileNoTree.returnRawData();


    if(err != -2)
        std::cerr << "Error: file not existing did not return good error code: " << err << std::endl;

    err = goodFileBadTree.decodeData();
    bad_data_container.root_raw_data = goodFileBadTree.returnRawData();

    if(err != -1)
        std::cerr << "Error: bad tree returned bad error code: " << err << std::endl;

    err = goodFileGoodTree.decodeData();
    good_data_container.root_raw_data = goodFileGoodTree.returnRawData();

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
    err = good_data.decodeData();
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_converter(data_container.root_raw_data);

    err = loc_converter.openSpecFile();

    if(err != 0)
        return;

    err = loc_converter.makeConversion();

    for(auto i : data_container.uvw_data){

        std::cout << "Signal value size is " << i.signal_val.size() <<
        " decoded plane value is " << i.plane_val << " decoded strip nr is " << i.strip_nr <<std::endl;

    }

}


void test_viewdata()
{



    generalDataStorage data_container;






    auto goodFile = "./rootdata/data.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto returned_file = good_data.openFile();
    auto err = good_data.readData();
    err = good_data.decodeData();
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_conv_uvw(data_container.root_raw_data);


    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
        std::cout<<"Make conversion error code "<<err<<std::endl;



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    data_container.uvw_data = loc_conv_uvw.returnDataUVW();






    TCanvas *print_canv = new TCanvas("Test_canvas", "Test_canvas", 1000, 1000);



    TH2D *raw_hist = new TH2D("my_hist", "hist", 512, 1, 513, 300, 1, 301);
    raw_hist->SetDirectory(0);


    //err = loc_conv_uvw.getHitInfo(data_container.root_raw_data, data_container.hit_data, data_container.raw_hist_container);

    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if(err != 0)
        std::cout<<"Error get hit info code "<<err<<std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout<<"Hit data size "<<data_container.hit_data.size()<<std::endl;
    std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<std::endl;


/* 
    int bin = 0;
    int strip = 1;

    for(auto iter : data_container.root_raw_data){

        //if(iter.plane_val == 2 && iter.strip_nr == 45)
        for(auto sig_iter : iter.signal_val){
            raw_hist->SetBinContent(++bin, strip, sig_iter);
        }

        bin = 0;
        strip++;
    }


    raw_hist->Draw("COLZ"); */





    




    print_canv->Print("test_hist01.pdf[");

    //ignore Info level messages
    //if you don't do this you get spammed with useless messages
    gErrorIgnoreLevel = kWarning;
    for(auto hist_iter : data_container.raw_hist_container){



        hist_iter->Draw();
        print_canv->Print("test_hist01.pdf", "Test title");


    }

    //reset message level
    gErrorIgnoreLevel = kPrint;
    print_canv->Print("test_hist01.pdf]");




/*     for(auto hit_iter : data_container.hit_data){

        std::cout<<"\n\n\n\n"<<hit_iter.npeaks<<" at plane "<<hit_iter.plane<<" and strip "<<hit_iter.strip<<"\n";
        for(auto p_iter : hit_iter.peaks_info){
            std::cout<<"X is "<<p_iter.peak_x<<" Y is "<<p_iter.peak_y<<" fwhm is "<<p_iter.fwhm<<"\n";
        }

    }
 */




}





void test_convertXYZ()
{


    generalDataStorage data_container;






    auto goodFile = "./rootdata/data.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto returned_file = good_data.openFile();
    auto err = good_data.readData();
    err = good_data.decodeData();
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_conv_uvw(data_container.root_raw_data);


    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
        std::cout<<"Make conversion error code "<<err<<std::endl;



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    data_container.uvw_data = loc_conv_uvw.returnDataUVW();



    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if(err != 0)
        std::cout<<"Error get hit info code "<<err<<std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout<<"Hit data size "<<data_container.hit_data.size()<<std::endl;
    std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<std::endl;



    convertXYZ loc_conv_xyz(data_container.hit_data);

    err = loc_conv_xyz.makeConversionXY();



}






void test()
{

/*     auto getLib = "dict/build/libMyLib.so";
    gSystem->Load(getLib); */


    //test_loadData();
    //test_convertUVW();
    //test_viewdata();
    test_convertXYZ();








}