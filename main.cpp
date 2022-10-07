#include<iostream>
#include<vector>
#include<string>

#include "TCanvas.h"



#include "../include/convMatData.hpp"
#include "../include/dataXYZ.hpp"
#include "../include/viewGraph.hpp"



int main(int argc, char **argv)
{

    convMatData matrix_data;

    constexpr double driftVelocity = 0.724;

    viewGraph graph_draw;

    std::vector<dataXYZ> data_xyz;

    std::shared_ptr<TCanvas> canvas_xyz;




    matrix_data.setMatrix(driftVelocity);

    matrix_data.printMatrix();


    //TODO
    //canvas_xyz = graph_draw.drawGraph(data_xyz);





    std::cout<<"Press Enter to continue "<<std::endl;
	getchar();


    return 0;

}