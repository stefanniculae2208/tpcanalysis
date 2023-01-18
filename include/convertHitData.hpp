#ifndef convertHitData_hpp
#define convertHitData_hpp 1


#include <numeric>


#include "hitPoints.hpp"
#include "dataUVW.hpp"


#include "TSpectrum.h"
#include "TH1.h"
#include "TF1.h"
#include "TSystem.h"
#include "TCanvas.h"





/**
 * @brief Takes in the data from each plane and strip and finds the hits.
 * 
 */
class convertHitData
{
    public:
    convertHitData(){};
    ~convertHitData();
    convertHitData(std::vector<dataUVW> uvw_data);



    int getHitInfo(Double_t peak_th = (double)50);

    int setUVWData(std::vector<dataUVW> uvw_data);


    std::vector<hitPoints> returnHitData();

    std::vector<TH1D*> returnHistData();



    private:

    /// @brief Vector containing the data in the UVW format. Taken as input.
    std::vector<dataUVW> m_uvw_data;

    /// @brief Vector containing the histograms. Used mainly for visualising the data from each plane and for debug.
    std::vector<TH1D*> m_raw_hist_data;


    /// @brief Vector containing the data of the hits detected. Is the output.
    std::vector<hitPoints> m_hit_data;


    

};






#endif