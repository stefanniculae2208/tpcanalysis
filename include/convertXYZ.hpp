#ifndef convertXYZ_hpp
#define convertXYZ_hpp 1

#define _USE_MATH_DEFINES
#include <cmath>



#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif




#include "rawData.hpp"
#include "loadData.hpp"
#include "dataXYZ.hpp"

//TODO
class convertXYZ
{
    public:
    convertXYZ(){};
    ~convertXYZ(){};


    int buildArray();
    int makeConversion(std::vector<rawData> &raw_data_vec, std::vector<dataXYZ> &converted_data_vec);

    

    private:
    const double drift_vel = 0.724;
    double matrix_params[4][3];



};





#endif