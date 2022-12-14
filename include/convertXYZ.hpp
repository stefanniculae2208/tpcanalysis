#ifndef convertXYZ_hpp
#define convertXYZ_hpp 1

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <iostream>



#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


#ifndef COS_PIOVER6
#define COS_PIOVER6 (0.866)
#endif

#ifndef TAN_PIOVER6
#define TAN_PIOVER6 (0.577)
#endif



#include "TGraph.h"
#include "TGraph2D.h"


#include "dataUVW.hpp"
#include "hitPoints.hpp"
#include "dataXYZ.hpp"


//TODO
class convertXYZ
{
    public:
    convertXYZ(){};
    ~convertXYZ(){};
    convertXYZ(std::vector<hitPoints> hit_data);



    int makeConversionXYZ();
    std::vector<dataXYZ> returnXYZ();
    int getNewVector(std::vector<hitPoints> hit_data);



    

    private:
    const double drift_vel = 0.724 * 10000 * 1000;
    const double time_unit = 4e-8;



    std::vector<hitPoints> m_hit_data;
    std::vector<dataXYZ> m_points_xyz;

    std::map<int, int> m_plane_map;




    //may be needed when optimising
    void sortHitData();


    
    std::pair<double, double> calculateXYfromUV(const int strip_u, const int strip_v);
    std::pair<double, double> calculateXYfromVW(const int strip_v, const int strip_w);
    std::pair<double, double> calculateXYfromUW(const int strip_u, const int strip_w);
    int evaluatePointsEquality(const std::pair<double, double> xy_from_uv, std::pair<double, double> xy_from_vw, std::pair<double, double> xy_from_uw);

    void calculateXYZ();


    











/* 
    std::pair<double, double> calculateXYfromUV_V2(int strip_u, int strip_v);
    std::pair<double, double> calculateXYfromVW_V2(int strip_v, int strip_w);
    std::pair<double, double> calculateXYfromUW_V2(int strip_u, int strip_w); */



};





#endif