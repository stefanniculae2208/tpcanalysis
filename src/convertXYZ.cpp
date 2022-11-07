#include "../include/convertXYZ.hpp"



convertXYZ::convertXYZ(std::vector<hitPoints> hit_data)
{
    m_hit_data = hit_data;
}






/**
* Builds the array used for conversion.
* The array holds the parameters for each element in the matrix.
*/
int convertXYZ::buildArray()
{

    m_matrix_params[0][0] = cos(0);

    m_matrix_params[0][1] = sin(0);

    m_matrix_params[0][2] = 0;


    m_matrix_params[1][0] = cos((-1) * 2 * M_PI /3);

    m_matrix_params[1][1] = sin((-1) * 2 * M_PI /3);

    m_matrix_params[1][2] = 0;


    m_matrix_params[2][0] = cos((-1) * M_PI /3);

    m_matrix_params[2][1] = sin((-1) * M_PI /3);

    m_matrix_params[2][2] = 0;


    m_matrix_params[3][0] = 0;

    m_matrix_params[3][1] = 0;

    m_matrix_params[3][2] = 1 / drift_vel;


    return 0;

}


int convertXYZ::makeConversionXY()
{





    struct{
        bool operator()(hitPoints a, hitPoints b)const{return a.peak_x < b.peak_x;}
    }lessHPI;






    std::sort(m_hit_data.begin(), m_hit_data.end(), lessHPI);


    for(auto hit_iter : m_hit_data){

        std::cout << "The x is " << hit_iter.peak_x << std::endl;


    }


    return 0;

}




