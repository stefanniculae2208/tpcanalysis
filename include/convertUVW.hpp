#ifndef convertUVW_hpp
#define convertUVW_hpp 1

#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>

#include "dataUVW.hpp"
#include "generalDataStorage.hpp"
#include "loadData.hpp"
#include "rawData.hpp"

#include "TCanvas.h"
#include "TH1.h"
#include "TSpectrum.h"

/**
 * @brief Used to make the conversion.
 *
 */
enum class GEM { U, V, W, size };

/**
 * @brief Uses the 'new_geometry_mini_eTPC.dat' file to dind the strip and plane
 * from the chip and channel, thus bringing it to the UVW format. Is also used
 * to smooth the signal and remove the baseline, but that can be moved to
 * another class in the future.
 *
 */
class convertUVW {

  public:
    convertUVW(){};
    convertUVW(std::vector<rawData> data_vec);
    ~convertUVW(){};

    int setRawData(std::vector<rawData> data_vec);

    int setUVWData(std::vector<dataUVW> data_vec);

    int openSpecFile();

    int makeConversion();

    int convertToCSV(std::string file_name);

    std::vector<dataUVW> returnDataUVW();

    int buildNormalizationMap();

    int normalizeChannels();

  private:
    /// @brief The map used to make the conversion. Built from
    /// 'new_geometry_mini_eTPC.dat'.
    std::map<std::pair<int, int>, std::pair<GEM, int>> fPositionMap;

    /// @brief The vector containing the raw data. Taken as input
    std::vector<rawData> m_data_vec;

    /// @brief The vector containing the converted data. Is the output.
    std::vector<dataUVW> m_uvw_vec;

    /// @brief The charge histogram. Not used currently but can be used in the
    /// future.
    TH1D *m_charge_hist = nullptr;

    /// @brief Used for debugging
    bool m_verbose = false;

    /// @brief Used for saving the data found in the normalization csv.
    std::map<std::pair<int, int>, double> m_ch_ratio_map;
};

#endif