#include "../include/convertXYZ.hpp"



/**
* Builds the array used for conversion.
* The array holds the parameters for each element in the matrix.
*/
int convertXYZ::buildArray()
{

    matrix_params[0][0] = cos(0);

    matrix_params[0][1] = sin(0);

    matrix_params[0][2] = 0;


    matrix_params[1][0] = cos((-1) * 2 * M_PI /3);

    matrix_params[1][1] = sin((-1) * 2 * M_PI /3);

    matrix_params[1][2] = 0;


    matrix_params[2][0] = cos((-1) * M_PI /3);

    matrix_params[2][1] = sin((-1) * M_PI /3);

    matrix_params[2][2] = 0;


    matrix_params[3][0] = 0;

    matrix_params[3][1] = 0;

    matrix_params[3][2] = 1 / drift_vel;


    return 0;

}




/**
* Makes the conversion from UVW to XYZ.
* @param raw_data_vec contains the data in the UVW format
* @param converted_data_vec the data is converted to XYZ and saved in this variable
*/
int convertXYZ::makeConversion(std::vector<rawData> &raw_data_vec, std::vector<dataXYZ> &converted_data_vec)
{

    dataXYZ loc_conv_data;

    for(auto &loc_raw_data : raw_data_vec){

//TODO

        converted_data_vec.push_back(loc_conv_data);

    }


    return 0;

}