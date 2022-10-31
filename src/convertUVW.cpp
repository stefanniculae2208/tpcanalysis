#include "../include/convertUVW.hpp"




/*
 Opens the specification file and makes the map.
 Returns 0 upon success and -1 if it fails to open the file.
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




/*
Makes the UVW conversion.
Returns 0 upon success.
*/
int convertUVW::makeConversion(std::vector<rawData> &data_vec)
{
    auto i  = 0;

    for(auto &data_inst : data_vec){

        try{
            auto uvwPosition = fPositionMap.at({data_inst.chip_nr, data_inst.ch_nr});
            data_inst.plane_val = int(uvwPosition.first);
            data_inst.strip_nr = int(uvwPosition.second);
        }catch(...){
            	std::cout << "AGET: " << data_inst.chip_nr << "\tChannel: " << data_inst.ch_nr << " is not assigned." << std::endl;
            data_vec.erase(data_vec.begin() + i);     
        }

        i++;

    }

    return 0;

}




/*
 Substracts the baseline from the signal.
 The baseline is calculated as the mean of the first 64 bins from the signal.
*/
int convertUVW::substractBl(std::vector<rawData> &data_vec)
{

    const int sampleRegion = 64;

    double baseline;


    for(auto &data_el : data_vec){

        baseline = 0;

        for(auto i = 0; i < 64; i++){

            baseline += data_el.signal_val.at(i);

        }

        baseline /= sampleRegion;

        for(auto &sig_el : data_el.signal_val){

            sig_el -= baseline;

            if(sig_el < 0)
                sig_el = 0;

        }



        

    }














    return 0;

}