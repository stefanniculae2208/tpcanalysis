#include<iostream>
#include<vector>
#include<string>
#include<unistd.h>
#include<csignal>





#include "TFile.h"
#include "TTree.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TSystem.h"



#include "../include/convMatData.hpp"
#include "../include/dataXYZ.hpp"
#include "../include/viewGraph.hpp"
#include "../include/loadData.hpp"



// From GET
#include "dict/include/GDataSample.h"
#include "dict/include/GDataChannel.h"
#include "dict/include/GFrameHeader.h"
#include "dict/include/GDataFrame.h"











void alarm_handl(int sig_num)
{
    std::cout<<"Program is taking too long\n############\nClosing.\n############"<<std::endl;
    _exit(EXIT_FAILURE);
}





int main(int argc, char **argv)
{



    //Load file
    TApplication rApp("Root app", &argc, argv);

/*     auto getLib = "dict/build/libMyLib.so";
    gSystem->Load(getLib); */

    loadData("a", "b");







  

}