#ifndef cleanUVW_hpp
#define cleanUVW_hpp 1

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "dataUVW.hpp"

#include "TCanvas.h"
#include "TH1.h"

/**
 * @brief The purpose of this class is to clean the UVW signals by removing the
 * baseline and smoothing the signals.
 *
 */
class cleanUVW {
  public:
    cleanUVW()
        : m_coefficients(m_window_size) {   // Compute the Savitzky-Golay
                                            // coefficients for smoothing. Only
                                            // needs to be done once.
        computeSavitzkyGolayCoefficients();
    };
    cleanUVW(std::vector<dataUVW> uvw_vec);
    ~cleanUVW(){};

    int setUVWData(std::vector<dataUVW> uvw_vec);

    template <typename pI> int substractBl();
    template <typename pI> int getChargeHist(TH1D *&charge_hist);

    std::vector<dataUVW> returnDataUVW();

    /**
     * @brief The struct contains information regarding the U plane.
     * The U, V and W structs are used as types for the functions of the class.
     *
     */
    struct planeInfoU {
        static const int plane_nr = 0;
        static const int plane_size = 72;
        inline static const std::string plane_hist_name = "Charge_hist_plane_u";
    };

    /**
     * @brief The struct contains information regarding the V plane.
     * The U, V and W structs are used as types for the functions of the class.
     *
     */
    struct planeInfoV {
        static const int plane_nr = 1;
        static const int plane_size = 92;
        inline static const std::string plane_hist_name = "Charge_hist_plane_v";
    };

    /**
     * @brief The struct contains information regarding the W plane.
     * The U, V and W structs are used as types for the functions of the class.
     *
     */
    struct planeInfoW {
        static const int plane_nr = 2;
        static const int plane_size = 92;
        inline static const std::string plane_hist_name = "Charge_hist_plane_w";
    };

  private:
    /// @brief The vector containing the UVW data.
    std::vector<dataUVW> m_uvw_vec;
    /// @brief The baseline extracted fromt he signal.
    double m_baseline = std::numeric_limits<double>::max();
    /// @brief The vector containing the charge histogram.
    std::array<double, 512> m_charge_val = {0};

    /// @brief The window size of the savitzkyGolayFilter.
    static const int m_window_size = 8;
    /// @brief The polynomial order of the savitzkyGolayFilter.
    static const int m_poly_order = 2;
    /// @brief The computed coefficients of the savitzkyGolayFilter.
    std::vector<double> m_coefficients;

    void smoothChannel(std::vector<double> &v);
    void calculateBaseline();
    template <typename pI> void calculateChargeHist();

    void computeSavitzkyGolayCoefficients();
    std::vector<double> savitzkyGolayFilter(const std::vector<double> &signal);
};

#endif