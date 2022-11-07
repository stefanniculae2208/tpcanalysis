#ifndef loadData_hpp
#define loadData_hpp 1


#include <iostream>


#include "rawData.hpp"



#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include <TSystem.h>
#include <TROOT.h>


// From GET
#include "../dict/include/GDataSample.h"
#include "../dict/include/GDataChannel.h"
#include "../dict/include/GFrameHeader.h"
#include "../dict/include/GDataFrame.h"

using namespace GET;


class loadData
{
    public:
    loadData(){};
    ~loadData(){};

    loadData(TString filename, TString treename);

    std::shared_ptr<TFile> openFile();
    int readData();
    TTree* returnTree();
    int decodeData();
    std::vector<rawData> returnRawData();





    private:
    TString m_filename;
    TString m_treename;

    std::shared_ptr<TFile> m_rootfile;
    TTree *m_roottree;
    std::vector<rawData> m_root_raw_data;



};




#endif