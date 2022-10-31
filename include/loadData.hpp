#ifndef loadData_hpp
#define loadData_hpp 


#include <iostream>



#include "TString.h"
#include "TFile.h"
#include "TTree.h"


// From GET
#include "../dict/include/GDataSample.h"
#include "../dict/include/GDataChannel.h"
#include "../dict/include/GFrameHeader.h"
#include "../dict/include/GDataFrame.h"


class loadData
{
    public:
    loadData(){};
    ~loadData(){};

    loadData(TString filename, TString treename);

    std::shared_ptr<TFile> openFile();
    int readData();
    TTree* returnTree();


    private:
    TString filename;
    TString treename;

    std::shared_ptr<TFile> rootfile;
    TTree *roottree;


};




#endif