#ifndef convertUVW_hpp
#define convertUVW_hpp 



#include <string>
#include <fstream>
#include <sstream>


#include "rawData.hpp"
#include "loadData.hpp"



enum class GEM{
    U,
    V,
    W,
    size
};




class convertUVW
{

    public:

    convertUVW(){};
    ~convertUVW(){};

    int openSpecFile();
    int makeConversion(std::vector<rawData> &data_vec);
    int substractBl(std::vector<rawData> &data_vec);


    private:
    std::map<std::pair<int, int>, std::pair<GEM, int>> fPositionMap;
    






};



#endif