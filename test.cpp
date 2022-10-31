#include <iostream>


#include "src/loadData.cpp"

#include "TSystem.h"
#include "TROOT.h"



// From GET
#include "dict/include/GDataSample.h"
#include "dict/include/GDataChannel.h"
#include "dict/include/GFrameHeader.h"
#include "dict/include/GDataFrame.h"



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
    auto goodFile = "./rootdata/stefan2.root";


    auto badTree = "bad_tree_name";
    auto goodTree = "tree_data";


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





void test_loadData()
{

    test_openFile();

    test_readData();

}







void test()
{

    auto getLib = "dict/build/libMyLib.so";
    gSystem->Load(getLib);


    test_loadData();

}