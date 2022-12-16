#ifndef hitPoints_hpp
#define hitPoints_hpp 1


#include <vector>
#include "TROOT.h"



class hitPoints
{
    public:
    hitPoints(){};
    ~hitPoints(){};



    int strip = -1;
    int plane = -1;
    int base_line = 0;
    int entry_nr = -1;
    Double_t peak_x = -1;
    Double_t peak_y = -1;
    Double_t fwhm = -1;







};







#endif