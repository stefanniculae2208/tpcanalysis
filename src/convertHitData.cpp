#include "../include/convertHitData.hpp"


/**
 * @brief Construct a new convert Hit Data::convert Hit Data object
 * 
 * @param uvw_data the uvw data vector to be used when finding the hits
 */
convertHitData::convertHitData(std::vector<dataUVW> uvw_data)
{
    m_uvw_data = uvw_data;
}



/**
 * @brief sets the uvw vector to be used when finding the hits
 * 
 * @param uvw_data the vector in question
 * @return int error codes
 */
int convertHitData::setUVWData(std::vector<dataUVW> uvw_data)
{

    if(uvw_data.size() == 0)
        return -3;


    m_uvw_data = uvw_data;
    return 0;

}



/**
 * @brief returns the vector of hits
 * 
 * @return std::vector<hitPoints> the vector
 */
std::vector<hitPoints> convertHitData::returnHitData()
{
    return m_hit_data;
}




/**
 * @brief returns the vector of histograms
 * 
 * @return std::vector<TH1D*> the vector
 */
std::vector<TH1D*> convertHitData::returnHistData()
{
    return m_raw_hist_data;
}



/**
 * @brief gets the hit info and saves the data obtained in a hitData vector and in a histogram vector
 * Firstly, TSpectrum is used to find the number of peaks.
 * We then eliminate the peaks that are below a threshold value (currently 200).
 * A function of form ‘pol0+gaus+gaus+...’ is created based on the numbers of peaks found.
 * We use this function to fit the histograms.
 * The parameters obtained from the histogram fitting are saved in the hitData type vector. 
 * 
 * @return int error codes
 */
int convertHitData::getHitInfo()
{

    if(m_uvw_data.size() == 0)
        return -3;//invalid size






    //first create the histograms
    int bin = 0;
    int strip = 1;

    for(auto &iter : m_uvw_data){

        auto loc_hist = new TH1D(Form("hist_at_strip%d_plane%d", iter.strip_nr, iter.plane_val), Form("hist%d", strip), 512, 1, 512);


        for(auto &sig_iter : iter.signal_val){
            loc_hist->SetBinContent(++bin, sig_iter);
        }

        m_raw_hist_data.push_back(loc_hist);

        bin = 0;
        strip++;
    }






    //the calculate the hit data
    auto spec_analyzer = new TSpectrum();

    Double_t *pos_holder_x;
    Double_t *pos_holder_y;
    int npeaks;

    int curr_iter = 0;



    for(auto &hist_iter : m_raw_hist_data){



        //find the peaks in the histogram with TSpectrum
        spec_analyzer->Search(hist_iter, 5, "nodraw", 0.2);
        npeaks = spec_analyzer->GetNPeaks();
        pos_holder_x = spec_analyzer->GetPositionX();
        pos_holder_y = spec_analyzer->GetPositionY();


        //find which peaks are good
        std::vector<Double_t> peaks_x;//x of valid peaks
        std::vector<Double_t> peaks_y;//y of valid peaks
        int nrealpeaks = 0;//number of valid peaks set to 0
        Double_t peak_th = (double)50;//threshold value for valid peaks



        for(auto i = 0; i < npeaks; i++){
            if(pos_holder_y[i] > peak_th){
                peaks_x.push_back(pos_holder_x[i]);
                peaks_y.push_back(pos_holder_y[i]);
                nrealpeaks++;
            }
        }

        //std::cout<<"Peaks found: "<<npeaks<<" of which valid are: "<<curr_point.npeaks<<std::endl;


        //build the function based on the number of valid peaks found
        TString func_holder("pol0(0)");

        for(auto i = 0; i < nrealpeaks; i++)
            func_holder.Append(Form("+gaus(%d)", (3*i+1)));

        auto gaus_and_pol0 = new TF1("gaus_and_pol0", func_holder.Data(), 1, 512);

        //std::cout<<"Function is "<<func_holder.Data()<<std::endl;
        gaus_and_pol0->SetParameter(0, 3);
        for(auto i = 0; i < nrealpeaks; i++){
            gaus_and_pol0->SetParameter((3*i+1), peaks_y.at(i));
            gaus_and_pol0->SetParameter((3*i+2), peaks_x.at(i));
            gaus_and_pol0->SetParameter((3*i+3), 10);
        }

        hist_iter->Fit("gaus_and_pol0", "Q");




        for(auto i = 0; i < nrealpeaks; i++){

            hitPoints curr_point;


            curr_point.peak_x = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+2));
            curr_point.peak_y = hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+1));
            curr_point.fwhm = 2.355 * hist_iter->GetFunction("gaus_and_pol0")->GetParameter((3*i+3));
            curr_point.plane = m_uvw_data.at(curr_iter).plane_val;
            curr_point.strip = m_uvw_data.at(curr_iter).strip_nr;
            curr_point.base_line = hist_iter->GetFunction("gaus_and_pol0")->GetParameter(0);

            m_hit_data.push_back(curr_point);
        }






        curr_iter++;


    }










    return 0;


}