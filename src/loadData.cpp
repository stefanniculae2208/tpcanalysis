#include "../include/loadData.hpp"




loadData::loadData(TString filename, TString treename)
{

    this->filename = filename;
    this->treename = treename;

}




/*
 Checks if the file exists and opens it. If file isn't opened returns nullptr.
 On success returns pointer to file.
*/
std::shared_ptr<TFile> loadData::openFile()
{

    rootfile = std::make_shared<TFile> (filename, "READ");

    if(rootfile->IsOpen())
        return rootfile;


    
    return nullptr;

}





/*
 Reads the data from rootfile in roottree.
 Return different codes. Upon success returns 0.
*/
int loadData::readData()
{

    if(rootfile->IsZombie()){

        std::cerr << "Error: file is not open." <<std::endl;
        return -2;

    }

    if(!(rootfile->GetListOfKeys()->Contains(treename))){

        std::cerr << "Error: file does not contain tree " << treename.Data() << std::endl;
        return -1;

    }


    roottree = rootfile->Get<TTree>(treename);


    return 0;

}


/*
 Returns pointer to the tree containing data.
*/
TTree* loadData::returnTree()
{

    return roottree;

}