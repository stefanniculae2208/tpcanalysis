#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"


struct hitPeakInfo
{

    Double_t peak_x;
    Double_t peak_y;
    Double_t fwhm;//full width at half maximum

};


class hitPoints
{
    public:
    hitPoints(){};
    ~hitPoints(){};


    int npeaks;//number of peaks
    int strip;
    int plane;
    int base_line;
    std::vector<hitPeakInfo> peaks_info;//each element holds info about one peak


};







class generalDataStorage
{
    public:
    generalDataStorage(){};
    ~generalDataStorage(){};






    std::vector<rawData> root_raw_data;
    std::vector<dataXYZ> converted_data;

    std::vector<hitPoints> hit_data;
    std::vector<TH1D*> raw_hist_container;

    


};






#endif