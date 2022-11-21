#ifndef rawData_hpp
#define rawData_hpp 1

#include <cstdint>
#include <vector>


class rawData
{
    public:
    rawData(){};
    ~rawData(){};


    int  ch_nr;
    int chip_nr;
    int entry_nr;
    std::vector<double> signal_val;







};




#endif