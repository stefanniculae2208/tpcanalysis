#include "../include/convertUVW.hpp"




/**
* Opens the specification file and makes the map.
* Returns 0 upon success and -1 if it fails to open the file.
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
* Makes the UVW conversion.
* Returns 0 upon success.
* @param data_vec uses the data already existing in the chip_br and ch_nr variables and calculates the data for plane_val and strip_nr
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




/**
* Substracts the baseline from the signal.
* The baseline is calculated as the mean of the first 64 bins from the signal.
* @param data_vec contains the signal from which we extract the baseline
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



int convertUVW::getHitInfo(std::vector<rawData> &raw_data_vec, std::vector<hitPoints> &hit_points_vec, std::vector<TH1D*> &raw_hist_vec)
{

    //first create the histograms
    int bin = 0;
    int strip = 1;

    for(auto &iter : raw_data_vec){

        auto loc_hist = new TH1D(Form("hist_at_strip%d_plane%d", iter.strip_nr, iter.plane_val), Form("hist%d", strip), 512, 1, 512);


        for(auto sig_iter : iter.signal_val){
            loc_hist->SetBinContent(++bin, sig_iter);
        }

        raw_hist_vec.push_back(loc_hist);

        bin = 0;
        strip++;
    }







    //the calculate the hit data
    auto spec_analyzer = new TSpectrum();

    Double_t *pos_holder_x;
    Double_t *pos_holder_y;
    int npeaks;

    int curr_iter = 0;



    for(auto &hist_iter : raw_hist_vec){

        hitPoints curr_point;

        //find the peaks in the histogram with TSpectrum
        spec_analyzer->Search(hist_iter, 5, "nodraw", 0.2);
        npeaks = spec_analyzer->GetNPeaks();
        pos_holder_x = spec_analyzer->GetPositionX();
        pos_holder_y = spec_analyzer->GetPositionY();


        //find which peaks are good
        std::vector<Double_t> peaks_x;//x of valid peaks
        std::vector<Double_t> peaks_y;//y of valid peaks
        curr_point.npeaks = 0;//number of valid peaks set to 0
        Double_t peak_th = (double)200;//threshold value for valid peaks



        for(auto i = 0; i < npeaks; i++){
            if(pos_holder_y[i] > peak_th){
                peaks_x.push_back(pos_holder_x[i]);
                peaks_y.push_back(pos_holder_y[i]);
                curr_point.npeaks++;
            }
        }

        //std::cout<<"Peaks found: "<<npeaks<<" of which valid are: "<<curr_point.npeaks<<std::endl;


        //build the function based on the number of valid peaks found
        TString func_holder("pol0(0)");

        for(auto i = 0; i < curr_point.npeaks; i++)
            func_holder.Append(Form("+gaus(%d)", (3*i+1)));

        auto gaus_and_pol0 = new TF1("gaus_and_pol0", func_holder.Data(), 1, 512);

        //std::cout<<"Function is "<<func_holder.Data()<<std::endl;
        gaus_and_pol0->SetParameter(0, 3);
        for(auto i = 0; i < curr_point.npeaks; i++){
            gaus_and_pol0->SetParameter((3*i+1), peaks_y.at(i));
            gaus_and_pol0->SetParameter((3*i+2), peaks_x.at(i));
            gaus_and_pol0->SetParameter((3*i+3), 10);
        }

        hist_iter->Fit("gaus_and_pol0", "Q");


        //save the hit data obtained from fitting
        hitPeakInfo curr_peak;

        for(auto i = 0; i < curr_point.npeaks; i++){
            curr_peak.peak_x = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+2));
            curr_peak.peak_y = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+1));
            curr_peak.fwhm = 2.355 * hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+3));
            curr_point.peaks_info.push_back(curr_peak);
        }

        curr_point.plane = raw_data_vec.at(curr_iter).plane_val;
        curr_point.strip = raw_data_vec.at(curr_iter).strip_nr;
        curr_point.base_line = hist_iter->GetFunction("gaus_and_pol0")->GetParameter(0);


        hit_points_vec.push_back(curr_point);


        curr_iter++;


    }










    return 0;

}