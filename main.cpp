#include<iostream>
#include<vector>

#include "../include/convMatData.hpp"
#include "../include/dataXYZ.hpp"



int main(int argc, char **argv)
{

    convMatData matrix_data;

    constexpr double driftVelocity = 0.724;

    std::vector<dataXYZ> data_xyz;



    matrix_data.setMatrix(driftVelocity);

    matrix_data.printMatrix();






    return 0;

}