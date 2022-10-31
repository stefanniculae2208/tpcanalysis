#ifndef loadData_hpp
#define loadData_hpp 

#include "TString.h"
#include "TFile.h"
#include "TTree.h"


class loadData
{
    public:
    loadData(){};
    ~loadData(){};

    loadData(TString filename, TString treename);

    std::shared_ptr<TFile> openFile();
    int readData();
    std::shared_ptr<TTree> returnTree();


    private:
    TString filename;
    TString treename;

    std::shared_ptr<TFile> rootfile;
    std::shared_ptr<TTree> roottree;


};




#endif