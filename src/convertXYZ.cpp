#include "../include/convertXYZ.hpp"


/**
 * @brief Construct a new convert X Y Z::convert X Y Z object
 * 
 * @param hit_data the hitPoints type vector that is converted using the other functions
 */
convertXYZ::convertXYZ(std::vector<hitPoints> hit_data)
{
    m_hit_data = hit_data;
}



/**
 * @brief Get a new vector that is to be converted by the other functions. The old vector is lost.
 * 
 * @param hit_data the new vector to be used
 * @return int error codes
 */
int convertXYZ::getNewVector(std::vector<hitPoints> hit_data)
{

    if(hit_data.size() == 0)
        return -3;

    m_hit_data = hit_data;

    return 0;

}









/**
 * @brief Makes the conversion from UVW planes to XYZ coordinate system.
 * 
 * @return int error codes
 */
int convertXYZ::makeConversionXYZ()
{

    if(m_hit_data.size() == 0)
        return -3;



    sortHitData();


    compareXY();




    return 0;

}








/**
 * @brief Sorts the data based on the position of the peak on the x axis
 * right now it is not useful, but can be used when optimising the code.
 * 
 */
void convertXYZ::sortHitData()
{

    struct{
        bool operator()(hitPoints a, hitPoints b)const{return a.peak_x < b.peak_x;}
    }lessHPI;






    std::sort(m_hit_data.begin(), m_hit_data.end(), lessHPI);

}




/**
 * @brief This function does the actual conversion.
 * First we iterate through all the hits first for U, then for V, and finally for W.
 * We check if the hits happen at the same time. If yes, we calculate X and Y using 3 methods and if the values are the same
 * we calculate the Z value from the time information and add the point to our converted vector.
 * 
 */
void convertXYZ::compareXY()
{

        //is not good but it should work for now
    //first we only care for u plane
    for(auto i = 0; i < m_hit_data.size(); i++){

        //continue if not u plane
        if(m_hit_data.at(i).plane != 0)
            continue;





        //iterate through v plane
        for(auto j = 0; j < m_hit_data.size(); j++){


            //continue if not j
            if(m_hit_data.at(j).plane != 1)
                continue;


            //see if the peaks are at the same location, continue if not
            if((m_hit_data.at(j).peak_x < (m_hit_data.at(i).peak_x - m_hit_data.at(i).fwhm / 2.355)) ||
             (m_hit_data.at(j).peak_x > (m_hit_data.at(i).peak_x + m_hit_data.at(i).fwhm / 2.355))||
             (m_hit_data.at(j).plane == m_hit_data.at(i).plane)){

                continue;

            }



            //w plane
            for(auto k = 0; k < m_hit_data.size(); k++){


                //continue if not w
                if(m_hit_data.at(k).plane != 2)
                    continue;


                //see if this hit is also at the same location
                if((m_hit_data.at(k).peak_x < (m_hit_data.at(j).peak_x - m_hit_data.at(j).fwhm / 2.355)) ||
                    (m_hit_data.at(k).peak_x > (m_hit_data.at(j).peak_x + m_hit_data.at(j).fwhm / 2.355))||
                    (m_hit_data.at(k).plane == m_hit_data.at(j).plane)){

                        continue;

                }

                std::pair<double, double> xy_from_uv;
                std::pair<double, double> xy_from_vw;
                std::pair<double, double> xy_from_uw;

                xy_from_uv = calculateXYfromUV(m_hit_data.at(i).strip, m_hit_data.at(j).strip);
                xy_from_vw = calculateXYfromVW(m_hit_data.at(j).strip, m_hit_data.at(k).strip);
                xy_from_uw = calculateXYfromUW(m_hit_data.at(i).strip, m_hit_data.at(k).strip);


                if(evaluatePointsEquality(xy_from_uv, xy_from_vw, xy_from_uw)){

                    dataXYZ loc_xyz;
                    loc_xyz.data_x = xy_from_uv.first;
                    loc_xyz.data_y = xy_from_uv.second;
                    loc_xyz.data_z = drift_vel * time_unit * m_hit_data.at(i).peak_x;
                    loc_xyz.data_charge = m_hit_data.at(i).peak_y;//only the charge from u for now

                    m_points_xyz.push_back(loc_xyz);

                }



            }
        }
    }




}



/**
 * @brief 
 * 
 * @param strip_u the strip values from the U plane
 * @param strip_v the strip values from the V plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromUV(int strip_u, int strip_v)
{

    std::pair<double, double> xy_from_uv;

    double y_part_from_v = 4.33 + (strip_v - 1) * (1.5 / cos(M_PI/6));

    //106.5 is the width on x of the plate. 1.5 is the distance between strips
    //since the strips are between 1 and 72 we substract 1 so the first strip is at 106.5 and the last is at 0
    xy_from_uv.first = 106.5 - ((strip_u - 1) * 1.5);
    xy_from_uv.second = tan(-M_PI/6) * xy_from_uv.first + y_part_from_v;



    return xy_from_uv;

}



/**
 * @brief 
 * 
 * @param strip_v the strip values from the V plane
 * @param strip_w the strip values from the W plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromVW(int strip_v, int strip_w)
{
    
    std::pair<double, double> xy_from_vw;

    double y_part_from_v = 4.33 + (strip_v - 1) * (1.5 / cos(M_PI/6));
    double y_part_from_w = 100.459 - (strip_w - 1) * (1.5 / cos(M_PI/6));


    xy_from_vw.first = (y_part_from_w - y_part_from_v) / (tan(-M_PI/6) - tan(M_PI/6));
    xy_from_vw.second = tan(-M_PI/6) * xy_from_vw.first + y_part_from_v;
    


    return xy_from_vw;

}



/**
 * @brief 
 * 
 * @param strip_u the strip values from the U plane
 * @param strip_w the strip values from the W plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromUW(int strip_u, int strip_w)
{

    std::pair<double, double> xy_from_uw;


    double y_part_from_w = 100.459 - (strip_w - 1) * (1.5 / cos(M_PI/6));


    xy_from_uw.first = 106.5 - ((strip_u - 1) * 1.5);

    xy_from_uw.second = tan(M_PI/6) * xy_from_uw.first + y_part_from_w;



    return xy_from_uw;

}





int convertXYZ::evaluatePointsEquality(std::pair<double, double> xy_from_uv, std::pair<double, double> xy_from_vw, std::pair<double, double> xy_from_uw)
{

    const double calib_variable = 3;

    if((xy_from_uv.first > (xy_from_vw.first - calib_variable)) && (xy_from_uv.first < (xy_from_vw.first + calib_variable)) &&
        (xy_from_uv.second > (xy_from_vw.second - calib_variable)) && (xy_from_uv.second < (xy_from_vw.second + calib_variable)) &&
        (xy_from_uv.first > (xy_from_uw.first - calib_variable)) && (xy_from_uv.first < (xy_from_uw.first + calib_variable)) &&
        (xy_from_uv.second > (xy_from_uw.second - calib_variable)) && (xy_from_uv.second < (xy_from_uw.second + calib_variable)))
        return 1;




    return 0;
}




std::vector<dataXYZ> convertXYZ::returnXYZ()
{

    return m_points_xyz;

}
