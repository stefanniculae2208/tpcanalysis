#include<iostream>
#include<vector>
#include<string>
#include<unistd.h>
#include<csignal>

#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TApplication.h"



#include "../include/convMatData.hpp"
#include "../include/dataXYZ.hpp"
#include "../include/viewGraph.hpp"
#include "../include/loadData.hpp"







void alarm_handl(int sig_num)
{
    std::cout<<"Program is taking too long\n############\nClosing.\n############"<<std::endl;
    _exit(EXIT_FAILURE);
}





int main(int argc, char **argv)
{



    //Load file
    TApplication rApp("Root app", &argc, argv);







  

}