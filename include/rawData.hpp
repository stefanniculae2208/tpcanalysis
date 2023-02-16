#ifndef rawData_hpp
#define rawData_hpp 1

#include <cstdint>
#include <vector>



/**
 * @brief Used for data storage.
 * Contains the raw data extracted from the .root file.
 * 
 */
class rawData
{
    public:
    rawData(){};
    ~rawData(){};


    /// @brief The channel number. Must use new_geometry_mini_eTPC.dat to make sense of it.
    int  ch_nr;

    /// @brief The chip number. Must use new_geometry_mini_eTPC.dat to make sense of it.
    int chip_nr;

    /// @brief The enry from the TTree from which the signal originates.
    int entry_nr;

    /// @brief The signal.
    std::vector<double> signal_val;







};




#endif