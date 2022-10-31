#include <iostream>



#include "src/loadData.cpp"
#include "src/convertUVW.cpp"



#include "TSystem.h"
#include "TROOT.h"



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






    err = noFileNoTree.decodeData();


    if(err != -2)
        std::cerr << "Error: file not existing did not return good error code: " << err << std::endl;

    err = goodFileBadTree.decodeData();

    if(err != -1)
        std::cerr << "Error: bad tree returned bad error code: " << err << std::endl;

    err = goodFileGoodTree.decodeData();

    if(err != 0)
        std::cerr << "Error: good tree returned bad code: " << err << std::endl;
    else   
        std::cout << "Success! Size of vector is " << goodFileGoodTree.root_raw_data.size() << std::endl;

    

    for(auto i : goodFileGoodTree.root_raw_data){

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


    loadData good_data(goodFile, goodTree);

    auto returned_file = good_data.openFile();
    auto err = good_data.readData();
    err = good_data.decodeData();

    convertUVW loc_converter;

    err = loc_converter.openSpecFile();

    if(err != 0)
        return;

    err = loc_converter.makeConversion(good_data.root_raw_data);

    for(auto i : good_data.root_raw_data){

        std::cout << "Channel is " << i.ch_nr << " chip is " << i.chip_nr << " signal value size is " << i.signal_val.size() <<
        " decoded plane value is " << i.plane_val << " decoded strip nr is " << i.strip_nr <<std::endl;

    }

}







void test()
{

/*     auto getLib = "dict/build/libMyLib.so";
    gSystem->Load(getLib); */


    //test_loadData();
    test_convertUVW();








}