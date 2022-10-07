#ifndef covvMatData_hpp
#define covvMatData_hpp 1

#define _USE_MATH_DEFINES
#include <array>
#include <cmath>
#include <iostream>

#include "dataXYZ.hpp"


#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif



class convMatData
{
    public:


    convMatData();
    ~convMatData();

    void setMatrix(double newDriftVel);

    void printMatrix();

    int convertCoords(double data_u, double data_v, double data_w, double data_t, dataXYZ &data_xyz);








    private:


    double driftVel;

    std::array<std::array<double, 3>, 4> matrixData;

    

};





#endif