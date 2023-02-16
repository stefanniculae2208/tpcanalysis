#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp 1

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"
#include "dataUVW.hpp"
#include "hitPoints.hpp"



#include "TH1.h"






/**
 * @brief A general data storage class that contains object of different types.
 * Each object of generalDataStorage type must only contain 1 entry from the tree.
 * 
 */
class generalDataStorage
{
    public:
    generalDataStorage(){};
    ~generalDataStorage(){};





    /// @brief Stores the raw data from the root file.
    std::vector<rawData> root_raw_data;

    /// @brief Stores the data in the UVW format.
    std::vector<dataUVW> uvw_data;

    /// @brief Stores the data in the XYZ format.
    std::vector<dataXYZ> xyz_data;

    /// @brief Stores the hit information.
    std::vector<hitPoints> hit_data;

    /// @brief Stores the histograms for each channel in an entry. Redundant and will be removed.
    std::vector<TH1D*> raw_hist_container;

    //The number of entries from the tree.
    Long64_t n_entries;

    


};






#endif