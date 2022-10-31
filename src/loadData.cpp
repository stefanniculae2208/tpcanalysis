#include "../include/loadData.hpp"




loadData::loadData(TString filename, TString treename)
{

    this->filename = filename;
    this->treename = treename;

}



/*
 Checks if the file exists and opens it. If file isn't opened returns null ptr.
 On success returns pointer to file.
*/
std::shared_ptr<TFile> loadData::openFile()
{

    rootfile = std::make_shared<TFile> (filename, "READ");

    if(rootfile->IsOpen())
        return rootfile;


    
    return nullptr;

}




int loadData::readData()
{

    

    return 0;

}



std::shared_ptr<TTree> loadData::returnTree()
{

    return roottree;

}