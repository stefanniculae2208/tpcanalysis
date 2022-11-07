#ifndef hitPoints_hpp
#define hitPoints_hpp 1


#include <vector>
#include "TROOT.h"



class hitPoints
{
    public:
    hitPoints(){};
    ~hitPoints(){};


    //int npeaks = 0;//number of peaks
    int strip;
    int plane;
    int base_line;
    //std::vector<hitPeakInfo> peaks_info;//each element holds info about one peak
    Double_t peak_x;
    Double_t peak_y;
    Double_t fwhm;

};







#endif