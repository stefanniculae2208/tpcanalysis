#include "../include/loadData.hpp"



/**
 * @brief Construct a new load Data::load Data object
 * 
 * @param filename name of the file to be opened
 * @param treename name of the tree in the file
 */
loadData::loadData(TString filename, TString treename)
{

    m_filename = filename;
    m_treename = treename;

}




/**
 * @brief Checks if the file exists and opens it.
 * 
 * @return std::shared_ptr<TFile> On success returns pointer to file. If file isn't opened returns nullptr.
 */
std::shared_ptr<TFile> loadData::openFile()
{

    m_rootfile = std::make_shared<TFile> (m_filename, "READ");

    if(m_rootfile->IsOpen())
        return m_rootfile;


    
    return nullptr;

}





/**
 * @brief reads the tree from the file
 * 
 * @return int error codes. 0 is success
 */
int loadData::readData()
{

    if(m_rootfile->IsZombie()){

        std::cerr << "Error: file is not open." <<std::endl;
        return -1;

    }

    if(!(m_rootfile->GetListOfKeys()->Contains(m_treename))){

        std::cerr << "Error: file does not contain tree " << m_treename.Data() << std::endl;
        return -2;

    }


    m_roottree = m_rootfile->Get<TTree>(m_treename);


    return 0;

}



/**
 * @brief returns the tree
 * 
 * @return TTree* the tree from the file
 */
TTree* loadData::returnTree()
{

    return m_roottree;

}



/**
 * @brief decodes the data from an entry in the tree using the dictionary and saves it in a rawData vector
 * if multiple entries are to be read then you should have a vector and each element calls this function
 * with a different entryNr
 * 
 * @param entryNr the number of the entry from the tree to be decoded
 * @return int error codes
 */
int loadData::decodeData(int entryNr)
{

    if(m_rootfile->IsZombie()){

        std::cerr << "Error: file is not open." <<std::endl;
        return -1;

    }

    if(!(m_rootfile->GetListOfKeys()->Contains(m_treename))){

        std::cerr << "Error: file does not contain tree " << m_treename.Data() << std::endl;
        return -2;

    }

    //clear the rawData vector so we don't collect old data
    std::vector<rawData>().swap(m_root_raw_data);





    auto data = new GDataFrame();
    m_roottree->SetBranchAddress("GDataFrame", &data);

    //m_nEntries = m_roottree->GetEntries();


    rawData loc_data;



    m_roottree->GetEntry(entryNr);


    TIter channelIT((TCollection *)data->GetChannels());
    GDataChannel *channel = nullptr;
    while ((channel = (GDataChannel *)channelIT.Next())) {
        TIter sampleIT((TCollection *)&channel->fSamples);
        GDataSample *sample = nullptr;
        //auto counter = 0;
        while ((sample = (GDataSample *)sampleIT.Next())) {

            loc_data.chip_nr = channel->fAgetIdx;
            loc_data.ch_nr = channel->fChanIdx;
            loc_data.entry_nr = entryNr;
            loc_data.signal_val.push_back(sample->fValue);

	            
        }
        m_root_raw_data.push_back(loc_data);
        std::vector<double>().swap(loc_data.signal_val);
    }






    return 0;

}


/**
 * @brief returns the raw data vector. To be called only after the decodeData function
 * 
 * @return std::vector<rawData> the vector
 */
std::vector<rawData> loadData::returnRawData()
{
    return m_root_raw_data;
}



/**
 * @brief returns the number of entries. Only called agter the decodeData function.
 * 
 * @return Long64_t the number of entries taken from the file
 */
Long64_t loadData::returnNEntries()
{
    return m_roottree->GetEntries();
}