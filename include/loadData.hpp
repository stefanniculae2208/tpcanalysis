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



/**
 * @brief Reads the specified TTree from the specified '.root' file.
 * Can only decode one entry at a time.
 * 
 */
class loadData
{
    public:
    loadData(){};
    ~loadData();

    loadData(TString filename, TString treename);

    int openFile();

    int readData();

    int decodeData(int entryNr);

    std::vector<rawData> returnRawData();

    Long64_t returnNEntries();





    private:

    /// @brief The name of the file to be opened.
    TString m_filename;

    /// @brief The name of the TTree in the file.
    TString m_treename;

    /// @brief The file containg the data.
    std::shared_ptr<TFile> m_rootfile;

    /// @brief The TTree read from the file.
    TTree *m_roottree;

    /// @brief The raw data taken from the TTree. Conatins only 1 entry.
    std::vector<rawData> m_root_raw_data;

    /// @brief The number of entries in the TTree.
    Long64_t m_nEntries = 0;

    GDataFrame *m_data_branch;



};




#endif