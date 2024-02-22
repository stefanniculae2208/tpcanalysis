#ifndef dataUVW_hpp
#define dataUVW_hpp 1

#include <cstdint>
#include <vector>

/**
 * @brief Used to make the conversion.
 *
 */
enum class GEM { U, V, W, size };

/**
 * @brief Used for data storage.
 * The information is stored here in the UVW format.
 *
 */
class dataUVW {
  public:
    dataUVW(){};
    ~dataUVW(){};

    /// @brief The plane the signal belongs to. O is U plane, 1 is V plane, 2 is
    /// W plane.
    int plane_val = INT32_MAX;

    /// @brief Thw section of the strip. Is always 0 for miniTPC.
    int strip_section = INT32_MAX;

    /// @brief The number of the strip. Interval [1, 72].
    int strip_nr = INT32_MAX;

    /// @brief The number of the entry the signal belongs to.
    int entry_nr = INT32_MAX;

    /// @brief  The baseline extracted from the signal.
    double baseline_val = INT32_MAX;

    /// @brief The signal vector.
    std::vector<double> signal_val;
};

#endif