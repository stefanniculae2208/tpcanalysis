#include "../include/loadData.hpp"




loadData::loadData(TString filename, TString treename)
{

    this->filename = filename;
    this->treename = treename;

}




/**
* Checks if the file exists and opens it. If file isn't opened returns nullptr.
* On success returns pointer to file.
*/
std::shared_ptr<TFile> loadData::openFile()
{

    rootfile = std::make_shared<TFile> (filename, "READ");

    if(rootfile->IsOpen())
        return rootfile;


    
    return nullptr;

}





/**
* Reads the data from rootfile in roottree.
* Return different codes. Upon success returns 0.
*/
int loadData::readData()
{

    if(rootfile->IsZombie()){

        std::cerr << "Error: file is not open." <<std::endl;
        return -1;

    }

    if(!(rootfile->GetListOfKeys()->Contains(treename))){

        std::cerr << "Error: file does not contain tree " << treename.Data() << std::endl;
        return -2;

    }


    roottree = rootfile->Get<TTree>(treename);


    return 0;

}


/**
* Returns pointer to the tree containing data.
*/
TTree* loadData::returnTree()
{

    return roottree;

}



/**
* Decodes the data and saves it in a rawData type vector.
* Returns 0 upon success.
* @param root_raw_data the vector in which the data is saved
*/
int loadData::decodeData(std::vector<rawData> &root_raw_data)
{

    if(rootfile->IsZombie()){

        std::cerr << "Error: file is not open." <<std::endl;
        return -1;

    }

    if(!(rootfile->GetListOfKeys()->Contains(treename))){

        std::cerr << "Error: file does not contain tree " << treename.Data() << std::endl;
        return -2;

    }



    auto data = new GDataFrame();
    roottree->SetBranchAddress("GDataFrame", &data);

    const auto nEvents = roottree->GetEntries();

    rawData loc_data;

    for(auto i = 0; i<1; i++){

        roottree->GetEntry(i);


        TIter channelIT((TCollection *)data->GetChannels());
        GDataChannel *channel = nullptr;
        while ((channel = (GDataChannel *)channelIT.Next())) {
            TIter sampleIT((TCollection *)&channel->fSamples);
            GDataSample *sample = nullptr;
            auto counter = 0;
            while ((sample = (GDataSample *)sampleIT.Next())) {

                loc_data.chip_nr = channel->fAgetIdx;
                loc_data.ch_nr = channel->fChanIdx;
                loc_data.signal_val.push_back(sample->fValue);

	            
            }
            root_raw_data.push_back(loc_data);
            std::vector<double>().swap(loc_data.signal_val);
        }


    }



    return 0;

}