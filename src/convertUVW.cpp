#include "../include/convertUVW.hpp"


/**
 * @brief Construct a new convert U V W::convert U V W object
 * 
 * @param data_vec the raw data vector
 */
convertUVW::convertUVW(std::vector<rawData> data_vec)
{
    m_data_vec = data_vec;
}


/**
 * @brief sets the raw data vactor
 * 
 * @param data_vec the raw data vector to be set
 * @return int 0 for success and -3 for vector size 0
 */
int convertUVW::setRawData(std::vector<rawData> data_vec)
{

    if(data_vec.size() == 0)
        return -3;

    m_data_vec = data_vec;
    return 0;

}





/**
 * @brief Opens the specification file and makes the map.
 * 
 * @return int Returns 0 upon success and -1 if it fails to open the file.
 */
int convertUVW::openSpecFile()
{

    std::string specfilename = "new_geometry_mini_eTPC.dat";

    std::ifstream spec_file(specfilename.c_str());

    if(!spec_file.is_open()){
        std::cerr << "Error opening file." << std::endl;
        return -1;
    }

    std::string buf;

    while(std::getline(spec_file, buf)){

        if(buf[0] == std::string("#")){
            continue;
        }else if(buf[0] == std::string("U") || buf[0] == std::string("V") || buf[0] == std::string("W")){

            std::stringstream line(buf);
            std::string filler;
            std::string plane;
            int strip;
            int AGET;//chip nr
            int channel;

            line >> plane >> filler >> strip >> filler >> filler
	        >> AGET >> channel >> filler >> filler >> filler;


            GEM gem = GEM::size;
            if(plane == std::string("U")) gem = GEM::U;
            else if(plane == std::string("V")) gem = GEM::V;
            else if(plane == std::string("W")) gem = GEM::W;

            fPositionMap.insert({{AGET, channel}, {gem, strip}});
            

            
        }

    }




    return 0;

}




/**
 * @brief Makes the UVW conversion.
 * 
 * @return int Returns 0 upon success, -3 if the vector size is incorrect.
 */
int convertUVW::makeConversion()
{
    auto i  = 0;
    auto err_code = 0;

    dataUVW loc_data_uvw;

    if(m_data_vec.size() == 0)
        return -3;




    for(auto &data_inst : m_data_vec){

        try{
            auto uvwPosition = fPositionMap.at({data_inst.chip_nr, data_inst.ch_nr});
            loc_data_uvw.plane_val = int(uvwPosition.first);
            loc_data_uvw.strip_nr = int(uvwPosition.second);
            loc_data_uvw.signal_val = data_inst.signal_val;
            loc_data_uvw.entry_nr = data_inst.entry_nr;
            m_uvw_vec.push_back(loc_data_uvw);
        }catch(...){
            std::cout << "AGET: " << data_inst.chip_nr << "\tChannel: " << data_inst.ch_nr << " is not assigned." << std::endl;
            m_data_vec.erase(m_data_vec.begin() + i);
            err_code = -4;//there exists data not assigned     
        }

        i++;

    }



    return err_code;

}




/**
 * @brief Substracts the baseline from the signal.
 * 
 * @return int The baseline is calculated as the mean of the first 64 bins from the signal.
 */
int convertUVW::substractBl()
{

    const int sampleRegion = 64;

    double baseline;


    if(m_uvw_vec.size() < sampleRegion)
        return -3;//invalid size


    for(auto &data_el : m_uvw_vec){

        baseline = 0;

        for(auto i = 0; i < sampleRegion; i++){

            baseline += data_el.signal_val.at(i);

        }

        baseline /= sampleRegion;

        for(auto &sig_el : data_el.signal_val){

            sig_el -= baseline;

            if(sig_el < 0)
                sig_el = 0;

        }


        for(auto i = 500; i<512; i++){
            data_el.signal_val.at(i) = 0;
        }


        

    }














    return 0;

}



/**
 * @brief returns the dataUVW vector
 * 
 * @return std::vector<dataUVW> the vector to be returned containing the data converted to UVW
 */
std::vector<dataUVW> convertUVW::returnDataUVW()
{

    return m_uvw_vec;

}








