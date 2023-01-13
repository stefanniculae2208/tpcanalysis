#ifndef convertUVW_hpp
#define convertUVW_hpp 1



#include <string>
#include <fstream>
#include <sstream>
#include <numeric>


#include "rawData.hpp"
#include "loadData.hpp"
#include "generalDataStorage.hpp"
#include "dataUVW.hpp"


#include "TSpectrum.h"
#include "TH1.h"
#include "TCanvas.h"



/**
 * @brief Used to make the conversion.
 * 
 */
enum class GEM{
    U,
    V,
    W,
    size
};



/**
 * @brief Uses the 'new_geometry_mini_eTPC.dat' file to dind the strip and plane from the chip and channel,
 * thus bringing it to the UVW format.
 * Is also used to smooth the signal and remove the baseline, but that can be moved to another class in the future.
 * 
 */
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

    int drawChargeHist();



    private:

    /// @brief The map used to make the conversion. Built from 'new_geometry_mini_eTPC.dat'.
    std::map<std::pair<int, int>, std::pair<GEM, int>> fPositionMap;

    /// @brief The vector containing the raw data. Taken as input
    std::vector<rawData> m_data_vec;

    /// @brief The vector containing the converted data. Is the output.
    std::vector<dataUVW> m_uvw_vec;

    /// @brief The charge histogram. Not used currently but can be used in the future.
    TH1D *m_charge_hist = nullptr;




    void calculateChargeHist();

    void smoothSignal(std::vector<double> &v);

    






};



#endif