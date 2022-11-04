#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"


class hitPoints
{
    public:
    hitPoints(){};
    ~hitPoints(){};

    double mean;
    int ch;
    int plane;
    double sigma;

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