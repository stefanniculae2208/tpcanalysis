#ifndef generalDataStorage_hpp
#define generalDataStorage_hpp 1

#include <numeric>
#include <vector>

#include "dataUVW.hpp"
#include "dataXYZ.hpp"
#include "hitPoints.hpp"
#include "rawData.hpp"

#include "TH1.h"

/**
 * @brief A general data storage class that contains object of different types.
 * Each object of generalDataStorage type must only contain 1 entry from the
 * tree.
 *
 */
class generalDataStorage {
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

    /// @brief Stores the histograms for each channel in an entry. Redundant and
    /// will be removed.
    std::vector<TH1D *> raw_hist_container;

    /// @brief The label obtained from filterEventsXY. At the moment it can be 1
    /// or 2.
    int filter_label = 0;

    /// @brief Obtained from convertHitData::containsVerticalLine(). Should be 1
    /// or 0.
    int contains_vertical_line = -1;

    /// @brief The calculated value of th Mean Square Error
    double mse_value = std::numeric_limits<double>::max();

    /// @brief  The number of the entry from the tree.
    Long64_t n_entry = 0;
};

static_assert(std::is_same<decltype(generalDataStorage::root_raw_data),
                           std::vector<rawData>>::value,
              "root_raw_data should be a vector of rawData");
static_assert(std::is_same<decltype(generalDataStorage::uvw_data),
                           std::vector<dataUVW>>::value,
              "uvw_data should be a vector of dataUVW");
static_assert(std::is_same<decltype(generalDataStorage::xyz_data),
                           std::vector<dataXYZ>>::value,
              "xyz_data should be a vector of dataXYZ");
static_assert(std::is_same<decltype(generalDataStorage::hit_data),
                           std::vector<hitPoints>>::value,
              "hit_data should be a vector of hitPoints");
static_assert(std::is_same<decltype(generalDataStorage::raw_hist_container),
                           std::vector<TH1D *>>::value,
              "raw_hist_container should be a vector of pointers to TH1D");
static_assert(
    std::is_same<decltype(generalDataStorage::filter_label), int>::value,
    "filter_label should be an int");
static_assert(
    std::is_same<decltype(generalDataStorage::mse_value), double>::value,
    "mse_value should be a double");
static_assert(
    std::is_same<decltype(generalDataStorage::n_entry), Long64_t>::value,
    "n_entry should be a Long64_t");

#endif