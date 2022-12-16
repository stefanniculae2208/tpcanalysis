#ifndef convertHitData_hpp
#define convertHitData_hpp 1



#include "hitPoints.hpp"
#include "dataUVW.hpp"


#include "TSpectrum.h"
#include "TH1.h"
#include "TF1.h"
#include "TSystem.h"



class convertHitData
{
    public:
    convertHitData(){};
    ~convertHitData(){};
    convertHitData(std::vector<dataUVW> uvw_data);



    int getHitInfo(Double_t peak_th = (double)50);
    int setUVWData(std::vector<dataUVW> uvw_data);


    std::vector<hitPoints> returnHitData();
    std::vector<TH1D*> returnHistData();



    private:
    std::vector<dataUVW> m_uvw_data;
    std::vector<TH1D*> m_raw_hist_data;
    std::vector<hitPoints> m_hit_data;


    

};






#endif