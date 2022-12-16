#ifndef convertUVW_hpp
#define convertUVW_hpp 1



#include <string>
#include <fstream>
#include <sstream>


#include "rawData.hpp"
#include "loadData.hpp"
#include "generalDataStorage.hpp"
#include "dataUVW.hpp"


#include "TSpectrum.h"
#include "TH1.h"




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
    convertUVW(std::vector<rawData> data_vec);
    ~convertUVW(){};


    int setRawData(std::vector<rawData> data_vec);
    int openSpecFile();
    int makeConversion();
    int substractBl();
    int convertToCSV(std::string file_name);
    std::vector<dataUVW> returnDataUVW();



    private:
    std::map<std::pair<int, int>, std::pair<GEM, int>> fPositionMap;
    std::vector<rawData> m_data_vec;
    std::vector<dataUVW> m_uvw_vec;

    






};



#endif