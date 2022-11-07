#ifndef convertXYZ_hpp
#define convertXYZ_hpp 1

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>



#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif




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


    int buildArray();
    int makeConversionXY();
    int makeConversionXYZ();



    

    private:
    const double drift_vel = 0.724;
    double m_matrix_params[4][3];
    std::vector<hitPoints> m_hit_data;
    std::vector<dataXYZ> m_xyz_data;
    std::vector<std::vector<hitPoints>> m_group_data;
    std::vector<std::vector<dataXYZ>> m_group_xyz;


    std::map<std::pair<int, int>, int> relationVW_U;




    void sortHitData();
    void groupHitData();
    void buildMap();
    void calculateXY();



};





#endif