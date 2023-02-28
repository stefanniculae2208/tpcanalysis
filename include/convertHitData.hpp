#ifndef convertHitData_hpp
#define convertHitData_hpp 1

#include <numeric>

#include "dataUVW.hpp"
#include "hitPoints.hpp"

#include "TCanvas.h"
#include "TF1.h"
#include "TH1.h"
#include "TSpectrum.h"
#include "TSystem.h"

/**
 * @brief Takes in the data from each plane and strip and finds the hits.
 *
 */
class convertHitData {
  public:
    convertHitData(){};
    ~convertHitData();
    convertHitData(std::vector<dataUVW> uvw_data);

    int getHitInfo(Double_t sensitivity_avg = (double) 2.5,
                   Double_t sensitivity_max = (double) 0.75);

    int setUVWData(std::vector<dataUVW> uvw_data);

    std::vector<hitPoints> returnHitData();

    std::vector<TH1D *> returnHistData();

  private:
    /// @brief Vector containing the data in the UVW format. Taken as input.
    std::vector<dataUVW> m_uvw_data;

    /// @brief Vector containing the histograms. Used mainly for visualising the
    /// data from each plane and for debug.
    std::vector<TH1D *> m_raw_hist_data;

    /// @brief Vector containing the data of the hits detected. Is the output.
    std::vector<hitPoints> m_hit_data;
};

#endif