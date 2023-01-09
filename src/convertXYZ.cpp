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

    std::vector<dataXYZ>().swap(m_points_xyz);

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




    //compareXY();

    calculateXYZ();




    return 0;

}













/**
 * @brief Sorts the hit data based on the plane value.
 * OLD: Sorts the data based on the position of the peak on the x axis. Right now it is not useful.
 * 
 */
void convertXYZ::sortHitData()
{

    /* struct{
        bool operator()(hitPoints &a, hitPoints &b)const{return a.peak_x < b.peak_x;}
    }lessHPI; */

    //std::sort(m_hit_data.begin(), m_hit_data.end(), lessHPI);




    std::sort(m_hit_data.begin(), m_hit_data.end(), [](const hitPoints &a, const hitPoints &b) { return a.plane< b.plane; });

}




/**
 * @brief This function does the actual conversion.
 * First we iterate through all the hits first for U, then for V, and finally for W.
 * We check if the hits happen at the same time. If yes, we calculate X and Y using 3 methods and if the values are the same
 * we calculate the Z value from the time information and add the point to our converted vector.
 * 
 */
void convertXYZ::calculateXYZ()
{

    //clear the vector
    std::vector<dataXYZ>().swap(m_points_xyz);

    auto start_u = std::lower_bound(m_hit_data.begin(), m_hit_data.end(), 0, [](const hitPoints& hp, const int v) { return hp.plane < v; });
    auto end_u = std::upper_bound(m_hit_data.begin(), m_hit_data.end(), 0, [](const int v, const hitPoints& hp) { return v < hp.plane; });

    auto start_v = std::lower_bound(m_hit_data.begin(), m_hit_data.end(), 1, [](const hitPoints& hp, const int v) { return hp.plane < v; });
    auto end_v = std::upper_bound(m_hit_data.begin(), m_hit_data.end(), 1, [](const int v, const hitPoints& hp) { return v < hp.plane; });

    auto start_w = std::lower_bound(m_hit_data.begin(), m_hit_data.end(), 2, [](const hitPoints& hp, const int v) { return hp.plane < v; });
    auto end_w = std::upper_bound(m_hit_data.begin(), m_hit_data.end(), 2, [](const int v, const hitPoints& hp) { return v < hp.plane; });


    for(auto it_u = start_u; it_u != end_u; ++it_u){

        if(it_u == m_hit_data.end()){

            std::cout<<"Iterator it_u hit end of vector."<<std::endl;
            break;

        }


        for(auto it_v = start_v; it_v != end_v; ++it_v){

            if(it_u == m_hit_data.end()){

                std::cout<<"Iterator it_v hit end of vector."<<std::endl;
                break;

            }

            //see if the peaks are at the same location, continue if not
            if(((*it_v).peak_x < ((*it_u).peak_x - (*it_u).fwhm / 2.355)) ||
             ((*it_v).peak_x > ((*it_u).peak_x + (*it_u).fwhm / 2.355)) ||
             ((*it_v).plane == (*it_u).plane)){

                continue;

            }



            for(auto it_w = start_w; it_v != end_w; ++it_w){

                if(it_w == m_hit_data.end()){

                    std::cout<<"Iterator it_w hit the end of the vector."<<std::endl;
                    break;

                }

                //see if this hit is also at the same location
                if(((*it_w).peak_x < ((*it_v).peak_x - (*it_v).fwhm / 2.355)) ||
                    ((*it_w).peak_x > ((*it_v).peak_x + (*it_v).fwhm / 2.355)) ||
                    ((*it_w).plane == (*it_v).plane)){

                        continue;

                }

                std::pair<double, double> xy_from_uv;
                std::pair<double, double> xy_from_vw;
                std::pair<double, double> xy_from_uw;

                xy_from_uv = calculateXYfromUV((*it_u).strip, (*it_v).strip);
                xy_from_vw = calculateXYfromVW((*it_v).strip, (*it_w).strip);
                xy_from_uw = calculateXYfromUW((*it_u).strip, (*it_w).strip);


                if(evaluatePointsEquality(xy_from_uv, xy_from_vw, xy_from_uw)){

                
                    

                    dataXYZ loc_xyz;
                    loc_xyz.data_x = (xy_from_uv.first + xy_from_vw.first + xy_from_uw.first)/3;
                    loc_xyz.data_y = (xy_from_uv.second + xy_from_vw.second + xy_from_uw.second)/3;
                    loc_xyz.data_z = drift_vel * time_unit * (*it_u).peak_x;
                    loc_xyz.data_charge = (*it_u).peak_y + (*it_v).peak_y + (*it_w).peak_y;

                    m_points_xyz.push_back(loc_xyz);


                    /* m_points_xyz.emplace_back(((xy_from_uv.first + xy_from_vw.first + xy_from_uw.first)/3), 
                                            ((xy_from_uv.second + xy_from_vw.second + xy_from_uw.second)/3),
                                            (drift_vel * time_unit * (*it_u).peak_x),
                                            ((*it_u).peak_y + (*it_v).peak_y + (*it_u).peak_y)); */


                }



            }



        }



    }




}











/* void convertXYZ::compareXY()
{

    //clear the vector
    std::vector<dataXYZ>().swap(m_points_xyz);

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

                
                    std::cout<<"U: "<<m_hit_data.at(i).strip<<" V: "<<m_hit_data.at(j).strip<<" W: "<<m_hit_data.at(k).strip;
                    std::cout<<" X: "<<xy_from_uv.first<<" Y: "<<xy_from_uv.second<<"\n";

                    dataXYZ loc_xyz;
                    loc_xyz.data_x = (xy_from_uv.first + xy_from_vw.first + xy_from_uw.first)/3;
                    loc_xyz.data_y = (xy_from_uv.second + xy_from_vw.second + xy_from_uw.second)/3;
                    loc_xyz.data_z = drift_vel * time_unit * m_hit_data.at(i).peak_x;
                    loc_xyz.data_charge = m_hit_data.at(i).peak_y + m_hit_data.at(j).peak_y + m_hit_data.at(k).peak_y;

                    m_points_xyz.push_back(loc_xyz);

                }



            }
        }
    }




} */



/**
 * @brief 
 * 
 * @param strip_u the strip values from the U plane
 * @param strip_v the strip values from the V plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromUV(const int strip_u, const int strip_v)
{

    std::pair<double, double> xy_from_uv;

    //0.866 is cos(M_PI/6)
    double y_part_from_v = 4.33 + (strip_v - 1) * (1.5 / COS_PIOVER6);

    //106.5 is the width on x of the plate. 1.5 is the distance between strips
    //since the strips are between 1 and 72 we substract 1 so the first strip is at 106.5 and the last is at 0
    xy_from_uv.first = 106.5 - ((strip_u - 1) * 1.5);


    //-0.577 is tan(-M_PI/6) which is -tan(M_PI/6)
    xy_from_uv.second = -TAN_PIOVER6 * xy_from_uv.first + y_part_from_v;



    return xy_from_uv;

}



/**
 * @brief 
 * 
 * @param strip_v the strip values from the V plane
 * @param strip_w the strip values from the W plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromVW(const int strip_v, const int strip_w)
{
    
    std::pair<double, double> xy_from_vw;


    //0.866 is cos(M_PI/6)
    double y_part_from_v = 4.33 + (strip_v - 1) * (1.5 / 0.866);
    double y_part_from_w = 100.459 - (strip_w - 1) * (1.5 / 0.866);

    //0.577 is tan(M_PI/6) and -0.577 is tan(-M_PI/6)
    xy_from_vw.first = (y_part_from_w - y_part_from_v) / (-TAN_PIOVER6 - TAN_PIOVER6);
    xy_from_vw.second = (-TAN_PIOVER6) * xy_from_vw.first + y_part_from_v;
    


    return xy_from_vw;

}



/**
 * @brief 
 * 
 * @param strip_u the strip values from the U plane
 * @param strip_w the strip values from the W plane
 * @return std::pair<double, double> the calculated X and Y values; the first element is Y and the second is Y
 */
std::pair<double, double> convertXYZ::calculateXYfromUW(const int strip_u, const int strip_w)
{

    std::pair<double, double> xy_from_uw;

    //0.866 is cos(M_PI/6) and cos(-M_PI/6)
    double y_part_from_w = 100.459 - (strip_w - 1) * (1.5 / COS_PIOVER6);


    xy_from_uw.first = 106.5 - ((strip_u - 1) * 1.5);

    //0.577 is tan(M_PI/6)
    xy_from_uw.second = TAN_PIOVER6 * xy_from_uw.first + y_part_from_w;



    return xy_from_uw;

}




/**
 * @brief This function checks to see if the 3 X and Y pairs calculated from the data of 2 planes are almost equal.
 * The calibration variable is used to set the range of the equality.
 * 
 * @param xy_from_uv x and y calculated from the u and v plane data
 * @param xy_from_vw x and y calculated from the v and w plane data
 * @param xy_from_uw x and y calculated from the u and w plane data
 * @return int 1 if true 0 if false
 */
int convertXYZ::evaluatePointsEquality(const std::pair<double, double> xy_from_uv, std::pair<double, double> xy_from_vw, std::pair<double, double> xy_from_uw)
{

    const double calib_variable = 3;


    return (xy_from_uv.first > (xy_from_vw.first - calib_variable) && xy_from_uv.first < (xy_from_vw.first + calib_variable) &&
        xy_from_uv.second > (xy_from_vw.second - calib_variable) && xy_from_uv.second < (xy_from_vw.second + calib_variable) &&
        xy_from_uv.first > (xy_from_uw.first - calib_variable) && xy_from_uv.first < (xy_from_uw.first + calib_variable) &&
        xy_from_uv.second > (xy_from_uw.second - calib_variable) && xy_from_uv.second < (xy_from_uw.second + calib_variable)/*  &&
        xy_from_vw.first > (xy_from_uw.first - calib_variable) && xy_from_vw.first < (xy_from_uw.first + calib_variable) &&
        xy_from_vw.second > (xy_from_uw.second - calib_variable) && xy_from_vw.second < (xy_from_uw.second + calib_variable) */ &&
        xy_from_uv.first > -3 && xy_from_uv.first < 110 && xy_from_uv.second > -3 && xy_from_uv.second < 110) ? 1 : 0;

    
}





/**
 * @brief Returns the vector contining the x, y and z values calculated.
 * 
 * @return std::vector<dataXYZ> the vector containing the x, y and z values of the points.
 */
std::vector<dataXYZ> convertXYZ::returnXYZ()
{

    return m_points_xyz;

}



/* 





std::pair<double, double> convertXYZ::calculateXYfromUV_V2(int strip_u, int strip_v)
{

    std::pair<double, double> xy_from_uv;


    const auto deg30 = 30. * TMath::Pi() / 180.;
    const auto slopeV = tan(-deg30);
    const auto pitchV = 1.5 / cos(deg30);
    const auto yV = 4.33 + (pitchV * strip_v);
    
    auto x = 106.5 - (strip_u * 1.5);
    auto y = x * slopeV + yV;
    xy_from_uv = {x, y};



    return xy_from_uv;

}

std::pair<double, double> convertXYZ::calculateXYfromVW_V2(int strip_v, int strip_w)
{
    
    std::pair<double, double> xy_from_vw;




    const auto deg30 = 30. * TMath::Pi() / 180.;
    const auto slopeV = tan(-deg30);
    const auto pitchV = 1.5 / cos(deg30);
    const auto yV = 4.33 + (pitchV * strip_v);
    
    const auto slopeW = tan(deg30);
    const auto pitchW = 1.5 / cos(deg30);
    const auto yW = 99.593 - (pitchW * strip_w);

    auto x = (yW - yV) / (slopeV - slopeW);
    auto y = x * slopeV + yV;
    
    xy_from_vw = {x, y};

    return xy_from_vw;

}




std::pair<double, double> convertXYZ::calculateXYfromUW_V2(int strip_u, int strip_w)
{

    std::pair<double, double> xy_from_uw;



    const auto deg30 = 30. * TMath::Pi() / 180.;
    const auto slopeW = tan(deg30);
    const auto pitchW = 1.5 / cos(deg30);
    const auto yW = 99.593 - (pitchW * strip_w);
    
    auto x = 106.5 - (strip_u * 1.5);
    auto y = x * slopeW + yW;
    xy_from_uw = {x, y};



    return xy_from_uw;

}



 */