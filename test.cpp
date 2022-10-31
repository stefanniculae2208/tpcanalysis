#include <iostream>


#include "src/loadData.cpp"





void test_loadData()
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

void test()
{

    test_loadData();

}