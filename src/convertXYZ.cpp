#include "../include/convertXYZ.hpp"
#include "../try_build_coords.cpp"



convertXYZ::convertXYZ(std::vector<hitPoints> hit_data)
{
    m_hit_data = hit_data;
}









int convertXYZ::makeConversionXY()
{


    sortHitData();

    buildMap();


/*     for(auto map_iter : relationVW_U){

        for(auto vect_iter : map_iter.second){

            if(map_iter.first.first == 41)
                std::cout<<"{ "<<map_iter.first.first<<", "<<map_iter.first.second<<": "<<vect_iter<<" }\n";

        }

    } */




    groupHitData();

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


                //if all 3 are at the same location we check to see if the 3 strips actually intersect in the same point using the map
                try{
                    for(auto u_iter : relationVW_U.at({m_hit_data.at(j).strip, m_hit_data.at(k).strip})){

                        if(m_hit_data.at(i).strip == u_iter){
                            std::vector<hitPoints> loc_group;

                            loc_group.push_back(m_hit_data.at(i));
                            loc_group.push_back(m_hit_data.at(j));
                            loc_group.push_back(m_hit_data.at(k));

                            m_group_data.push_back(loc_group);
                        }


                    }

                }catch(...){
                    std::cout<<"Error in map for values v: "<<m_hit_data.at(j).strip<<" and w: "<<m_hit_data.at(k).strip<<"\n";
                }







            }








            
            //loc_group.push_back(m_hit_data.at(j));

            

        }

        //loc_group.push_back(m_hit_data.at(i));
        //m_group_data.push_back(loc_group);

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
    relationVW_U = try_build_coords();


}





