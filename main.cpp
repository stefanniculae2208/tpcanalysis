#include<iostream>

#include "../include/convMatData.hpp"



int main(int argc, char **argv)
{

    convMatData matrix_data;

    matrix_data.setDriftVel(0.724);

    matrix_data.setMatrix();

    matrix_data.printMatrix();

    return 0;

}