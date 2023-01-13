#include "../include/convertUVW.hpp"


/**
 * @brief Construct a new convert U V W::convert U V W object
 * 
 * @param data_vec the raw data vector to be converted to the UVW format
 */
convertUVW::convertUVW(std::vector<rawData> data_vec)
{
    m_data_vec = data_vec;
}


/**
 * @brief Sets the raw data vector to a new value.
 * 
 * @param data_vec the raw data vector to be set
 * @return error codes
 */
int convertUVW::setRawData(std::vector<rawData> data_vec)
{

    if(data_vec.size() == 0)
        return -3;

    m_data_vec = data_vec;
    
    std::vector<dataUVW>().swap(m_uvw_vec);

    return 0;

}





/**
 * @brief Opens the specification file and makes the map.
 * 
 * @return error codes
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
 * @brief Makes the conversion to the UVW format.
 * 
 * @return error codes
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
 * @brief Calculates the charge for each time bin. The charge is calculated by adding the charge from each strip on each plane
 * for the same time bin. 
 * 
 */
void convertUVW::calculateChargeHist()
{

    std::array<double, 512> charge_val;

    m_charge_hist = new TH1D("Charge_hist", "Charge histogram", 512, 1, 512);

    charge_val.fill(0);

    for(auto &data_el : m_uvw_vec){

        for(auto i = 0; static_cast<std::vector<double>::size_type>(i) < data_el.signal_val.size(); i++){

            if(i > 511){
                std::cout<<"Error: signal has more than 512 bins.\n";
                break;
            }
            charge_val.at(i) += data_el.signal_val.at(i);

        }

    }

    for(auto i = 0; static_cast<std::vector<double>::size_type>(i) < charge_val.size(); i++){

        m_charge_hist->SetBinContent(i, charge_val.at(i));

    }

}




/**
 * @brief Draws the histogram of the charge added on each time bin.
 * 
 * @return int error codes
 */
int convertUVW::drawChargeHist()
{

    if(m_charge_hist == nullptr){

        calculateChargeHist();

    }
    

    if(m_charge_hist->GetEntries() == 0){
        return -3;
    }

    auto *charge_canv = new TCanvas("Charge_canvas", "Charge_canvas");

    m_charge_hist->Draw();

    charge_canv->Update();

    return 0;

}









/**
 * @brief Substracts the baseline from the signal and smooths the signal using the rolling average algorithm.
 * OLD: The baseline is calculated as the mean of the first 64 bins from the signal.
 * NEW: The baseline is now the smallest non 0 element in the vector.
 * 
 * @return int error codes
 */
int convertUVW::substractBl()
{

    const int sampleRegion = 64;

    double baseline = 0;


    if(m_uvw_vec.size() < sampleRegion)
        return -3;//invalid size


    calculateChargeHist();




    for(auto &data_el : m_uvw_vec){

        baseline = 0;




    
        //Calculate the baseline as the smallest non 0 element.
        auto start_iter = std::next(data_el.signal_val.begin(), 20);
        baseline = *std::min_element(start_iter, data_el.signal_val.end(), [](int a, int b) {
                        return (a > 0 && b > 0) ? (a < b) : (a > b);
                    });


        

        //Extract the baseline from the signal. Make any elements lower than 0 equal to 0 so we don't have negative signal values.
        std::transform(data_el.signal_val.begin(), data_el.signal_val.end(), data_el.signal_val.begin(),
               [baseline](double sig_el) { return std::max(0.0, (sig_el - baseline)); });




        smoothSignal(data_el.signal_val);


        //Set the final 12 elements to 0 because of artifacts. To be removed when working with good data.
        std::fill(data_el.signal_val.begin() + 500, data_el.signal_val.begin() + 512, 0);

        


        

    }














    return 0;

}



/**
 * @brief Returns the dataUVW vector containing the converted data.
 * 
 * @return std::vector<dataUVW> the vector to be returned containing the data converted to UVW
 */
std::vector<dataUVW> convertUVW::returnDataUVW()
{

    return m_uvw_vec;

}




/**
 * @brief Writes the data from m_uvw_vec to a .csv file.
 * The format of the .csv file is the following:
 *  First column is the plane value.
 *  Second colums is the strip number.
 *  Third column in the entry number. (Sould be the same for every element)
 *  The remaining columns contain the values of the signal.
 *  
 * @param file_name the name of the .csv file
 * @return int error codes
 */
int convertUVW::convertToCSV(std::string file_name)
{

    std::ofstream out_file(file_name);

    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open file for output" << std::endl;
        return -1;
    }

    //header
    out_file << "plane_val,strip_nr,entry_nr,signal_val\n";


    for(auto &data_entry : m_uvw_vec){

        out_file << data_entry.plane_val << "," << data_entry.strip_nr << "," << data_entry.entry_nr << ",";

        for(auto i = 0; static_cast<std::vector<double>::size_type>(i) < data_entry.signal_val.size(); i++){

            out_file << data_entry.signal_val[i];
            if (static_cast<std::vector<double>::size_type>(i) < data_entry.signal_val.size() - 1) {
                out_file << ",";
            }

        }

        out_file << "\n";

    }





    out_file.close();

    return 0;

}




/**
 * @brief Smooths the signal using the rolling average algorithm.
 * 
 * @param v the signal to be smoothed
 * @return std::vector<double> the smoothed signal
 */
void convertUVW::smoothSignal(std::vector<double> &v)
{
    int window_size = 8;


    for (auto i = 0; i < v.size(); i++) {

        int start = std::max(0, i - window_size / 2); // Start index of the window
        int end = std::min(int(v.size()) - 1, i + window_size / 2); // End index of the window


        v.at(i) = std::accumulate(&v.at(start), &v.at(end), 0.0) / (end - start); // Average of the elements in the window
    }


}








