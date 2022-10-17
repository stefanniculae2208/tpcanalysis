#ifndef viewGraph_hpp
#define viewGraph_hpp 

#include "dataXYZ.hpp"
#include <vector>
#include "TGraph2D.h"
#include "TCanvas.h"
#include "TPolyMarker3D.h"


//This class will be used to draw the graphs needed and everything related to this topic

class viewGraph
{

    public:
    
    viewGraph();
    ~viewGraph();


    std::shared_ptr<TCanvas> drawTrack(std::vector<dataXYZ> &data_xyz);




};














#endif