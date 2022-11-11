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


    sortHitData();

    groupHitData();

    buildMap();

    calculateXY();


    return 0;

}





void convertXYZ::sortHitData()
{

    struct{
        bool operator()(hitPoints a, hitPoints b)const{return a.peak_x < b.peak_x;}
    }lessHPI;






    std::sort(m_hit_data.begin(), m_hit_data.end(), lessHPI);

}


void convertXYZ::groupHitData()
{


    for(auto i = 0; i < m_hit_data.size(); i++){

        if(m_hit_data.at(i).plane != 0)
            continue;

        std::vector<hitPoints> loc_group;

        for(auto j = 0; j < m_hit_data.size(); j++){

            if(i == j)
                continue;

            if((m_hit_data.at(j).peak_x < (m_hit_data.at(i).peak_x - m_hit_data.at(i).fwhm / 2.355)) ||
             (m_hit_data.at(j).peak_x > (m_hit_data.at(i).peak_x + m_hit_data.at(i).fwhm / 2.355))||
             (m_hit_data.at(j).plane == m_hit_data.at(i).plane)){

                continue;

            }

            
            loc_group.push_back(m_hit_data.at(j));

            

        }

        loc_group.push_back(m_hit_data.at(i));
        m_group_data.push_back(loc_group);

    }


    int nr_grp = 0;

    for(auto grp_iter : m_group_data){

        std::cout<<"Group number  "<<nr_grp<<": "<<std::endl;

        for(auto hit_iter : grp_iter){

            std::cout<<"    Hit is at x "<<hit_iter.peak_x<<" plane "<<hit_iter.plane<<" strip "<<hit_iter.strip<<std::endl;

        }

        nr_grp++;

    }



}



void convertXYZ::calculateXY()
{


    for(auto grp_iter : m_group_data){

        std::vector<dataXYZ> loc_data_xyz;


        for(auto i = 0; i < grp_iter.size(); i++){

            if(grp_iter.at(i).plane != 1)
                continue;


            for(auto j = 0; j < grp_iter.size(); j++){

                if(grp_iter.at(j).plane != 2)
                    continue;


                dataXYZ loc_xyz;

                loc_xyz.data_x = (grp_iter.at(i).strip + grp_iter.at(j).strip)/(tan(M_PI/6) - tan(-M_PI/6));

                loc_xyz.data_y = tan(M_PI/6) * loc_xyz.data_x + grp_iter.at(i).strip;

                loc_xyz.data_z = 0;

                loc_data_xyz.push_back(loc_xyz);


            }

            



        }



        m_group_xyz.push_back(loc_data_xyz);

    }


    int nr_point = 0;

    std::cout<<"\n\n\n\n\n\n\n";

    for(auto ptr_iter : m_group_xyz){

        std::cout<<"Point "<<nr_point<<std::endl;

        for(auto pos_iter : ptr_iter){
            std::cout<<"    X is "<<pos_iter.data_x<<" and Y is "<<pos_iter.data_y<<std::endl;

        }

        nr_point++;

    }
    




}


void convertXYZ::buildMap()
{

    //TODO

}





