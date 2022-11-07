#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp 1

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"
#include "dataUVW.hpp"
#include "hitPoints.hpp"







class generalDataStorage
{
    public:
    generalDataStorage(){};
    ~generalDataStorage(){};






    std::vector<rawData> root_raw_data;
    std::vector<dataUVW> uvw_data;
    std::vector<dataXYZ> xyz_data;

    std::vector<hitPoints> hit_data;
    std::vector<TH1D*> raw_hist_container;

    


};






#endif