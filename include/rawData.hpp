#ifndef rawData_hpp
#define rawData_hpp 

#include <cstdint>
#include <vector>


class rawData
{
    public:
    rawData(){};
    ~rawData(){};


    int  ch_nr;
    int chip_nr;
    std::vector<double> signal_val;


    int plane_val = INT32_MAX;//absurd default value
    int strip_nr = INT32_MAX;





};




#endif