#ifndef dataUVW_hpp
#define dataUVW_hpp 1

#include <cstdint>
#include <vector>



class dataUVW
{
    public:
    dataUVW(){};
    ~dataUVW(){};



    int plane_val = INT32_MAX;//absurd default value
    int strip_nr = INT32_MAX;//absurd default value
    int entry_nr = INT32_MAX;
    std::vector<double> signal_val;



};





#endif