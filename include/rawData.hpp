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







};




#endif