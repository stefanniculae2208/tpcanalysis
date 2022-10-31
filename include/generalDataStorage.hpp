#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp

#include <vector>


#include "rawData.hpp"
#include "dataXYZ.hpp"


class generalDataStorage
{
    public:
    generalDataStorage(){};
    ~generalDataStorage(){};




    std::vector<rawData> root_raw_data;
    std::vector<dataXYZ> converted_data;
    


};






#endif