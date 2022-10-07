#include<iostream>

#include "../include/convMatData.hpp"



int main(int argc, char **argv)
{

    convMatData matrix_data;

    constexpr double driftVelocity = 0.724;





    matrix_data.setMatrix(driftVelocity);

    matrix_data.printMatrix();




    

    return 0;

}