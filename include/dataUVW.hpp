#ifndef dataUVW_hpp
#define dataUVW_hpp 

#include <cstdint>
#include <vector>



class dataUVW
{
    public:
    dataUVW(){};
    ~dataUVW(){};



    int plane_val = INT32_MAX;//absurd default value
    int strip_nr = INT32_MAX;//absurd default value
    std::vector<double> signal_val;



};





#endif