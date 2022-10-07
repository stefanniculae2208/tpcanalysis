#ifndef viewGraph_hpp
#define viewGraph_hpp 

#include "dataXYZ.hpp"
#include <vector>
#include "TGraph2D.h"
#include "TCanvas.h"


class viewGraph
{

    public:
    
    viewGraph();
    ~viewGraph();


    std::shared_ptr<TCanvas> drawGraph(std::vector<dataXYZ> &data_xyz);




};














#endif