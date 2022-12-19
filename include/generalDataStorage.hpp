#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp 1

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"
#include "dataUVW.hpp"
#include "hitPoints.hpp"



#include "TH1.h"






/**
 * @brief a general data storage class that contains object of different types
 * each object of generalDataStorage type must only contain 1 entry from the tree
 * when reading multiple entries you should have a std::vector<generalDataStorage>
 * 
 */
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

    //number of entries from the tree
    Long64_t n_entries;

    


};






#endif