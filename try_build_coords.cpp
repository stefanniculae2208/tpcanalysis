#include <array>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>



#include "TF1.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TH2.h"


struct padsInfo{

    int nr_pads;
    int strip_units;
    double pad_offset;

};

struct coordVal{
    int strip;
    int pad;

    double x;
    double y;


    bool operator==(const coordVal &a) const
    {

        return ((a.x < (x + 1)) && (a.x > (x - 1)) && (a.y < (y + 1)) && (a.y >     (y - 1)));


    }

};



std::vector<coordVal> buildUcoords(std::string file_name, bool verbose_opt)
{

    std::ofstream out_csv;

    std::vector<coordVal> loc_vect;


    std::array<std::array<std::pair<double, double>, 61>, 72> u_coord;

    std::array<padsInfo, 72> pads_arr;

    int total_pads = 0;

    for(auto i = 0; i < 72; i++){

        //number of strips away from ref point
        pads_arr[i].strip_units = 71-i;


        //number of pads in that line
        if(i < 6){

            pads_arr[i].nr_pads = 61 - (5-i);

        }else if(i > 66){

            pads_arr[i].nr_pads = 61 - (i - 66);

        }else{
            pads_arr[i].nr_pads = 61;
        }


        //the offset
        if(i < 4){

            pads_arr[i].pad_offset = i*(-0.5);

        }else if(i > 66){

            pads_arr[i].pad_offset = (-0.5)*(70-i);

        }else{

            pads_arr[i].pad_offset = (i%2)*(-0.5) - 2;

        }


        if(verbose_opt){
            std::cout<<"At i is "<<i+1<<" numberof pads is "<<pads_arr[i].nr_pads<<" at strips "<<pads_arr[i].strip_units
                <<" with offset "<<pads_arr[i].pad_offset<<std::endl;
        }


        total_pads += pads_arr[i].nr_pads;

    }

    std::cout<<"For plane U pads = "<<total_pads<<std::endl;


    out_csv.open(file_name);

    for(auto i = 0; i < 72; i++){


        for(auto j = 0; j < pads_arr[i].nr_pads; j++){

            coordVal loc_coord;

            u_coord[i][j] = {1.5*pads_arr[i].strip_units, 2*(2.5+pads_arr[i].pad_offset)*0.866 + 1.732*j};



            out_csv << i << "," << j << "," << u_coord[i][j].first << "," << u_coord[i][j].second << "\n";

            loc_coord.strip = i;
            loc_coord.pad = j;
            loc_coord.x = u_coord[i][j].first;
            loc_coord.y = u_coord[i][j].second;

            loc_vect.push_back(loc_coord);

        }

        
    }

    out_csv.close();


    return loc_vect;



}


//bad
std::vector<coordVal> buildVcoords(std::string file_name, bool verbose_opt)
{

    std::ofstream out_csv;

    std::vector<coordVal> loc_vect;


    std::array<padsInfo, 92> pads_arr;

    int total_pads = 0;

    for(auto i = 0; i < 92; i++){



        if(i >= 0 && i < 31){
            pads_arr[i].nr_pads = 6 + 2 * i;
        }else if(i >= 61 && i < 92){
            pads_arr[i].nr_pads = 6 + 2 * (91 - i);
        }else if(i >= 35 && i < 57){
            pads_arr[i].nr_pads = 71;
        }else if(i >= 31 && i < 35){
            pads_arr[i].nr_pads = pads_arr[i-1].nr_pads+1;
        }else if(i >= 57 && i < 61){
            pads_arr[i].nr_pads = pads_arr[i-1].nr_pads-1;
        }

        if(verbose_opt)
            std::cout<<"At i+1 "<<i+1<<" number of pads are "<<pads_arr[i].nr_pads<<std::endl;


        total_pads += pads_arr[i].nr_pads;

    }


    std::cout<<"For plane V pads = "<<total_pads<<std::endl;





    

    std::array<std::array<std::pair<double, double>, 72>, 92> v_coord;

 
    out_csv.open(file_name);

    for(auto v = 0; v < 92; v++){

        auto lineV = new TF1(Form("v_%02d", v), "pol1", -10, 120);

        lineV->SetParameters(4.33 + 2*(0.866 * v), tan(-M_PI/6));


        //need to find out the x_max for each strip
        double x_max = 0;

        if(v < 30){

            //lowest element of the first strip is at 8.25
            //the difference between the lowest elements of each strip sis 2*1.5 for the first 30 strips
            x_max = 8.250 + 2 * 1.5 * v;

        }else if(v > 29 && v < 35){

            //the 31st element is at 98.25 and the rest increase by 1.5 each time
            //we decrease by 59 because conter starts at 0
            x_max = 98.25 + 1.5 * (v - 29);

        }else{

            //all the elements here have the same x
            x_max = 105.75;

        }




        //std::cout<<"\n\n\nFor v "<<v<<": \n";

        //need to use the number of pads per strip and decrease x_max by 1.5 each time
        for(auto i = 0; i < pads_arr[v].nr_pads; i++){

            coordVal loc_coord;

            out_csv << std::fixed << std::setprecision(2) << v << "," << pads_arr[v].nr_pads - i - 1 << "," << x_max << "," << lineV->Eval(x_max) << "\n";




            loc_coord .strip = v;
            loc_coord.pad = pads_arr[v].nr_pads - i - 1;
            loc_coord.x = x_max;
            loc_coord.y = lineV->Eval(x_max);


/*             if(v > 50 && v < 60)
                std::cout<<"    For i "<<i<<" x is "<<x_max<<"\n"; */



            x_max -= 1.5;
            loc_vect.push_back(loc_coord);

        }




    }

    out_csv.close();




    return loc_vect;



}




//not sure if working
std::vector<coordVal> buildWcoords(std::string file_name, bool verbose_opt)
{

    std::ofstream out_csv;

    std::vector<coordVal> loc_vect;


    std::array<padsInfo, 92> pads_arr;


    int total_pads = 0;


    for(auto i = 0; i < 92; i++){



        if(i >= 0 && i < 31){
            pads_arr[i].nr_pads = 6 + 2 * i;
        }else if(i >= 61 && i < 92){
            pads_arr[i].nr_pads = 5 + 2 * (91 - i);
        }else if(i >= 35 && i < 56){
            pads_arr[i].nr_pads = 73;
        }else if(i >= 31 && i < 35){
            pads_arr[i].nr_pads = pads_arr[i-1].nr_pads+1;
        }else if(i > 56 && i < 61){
            pads_arr[i].nr_pads = pads_arr[i-1].nr_pads-1;
        }else if(i == 56){
            pads_arr[i].nr_pads = 71;
        }

        if(verbose_opt)
            std::cout<<"At i+1 "<<i+1<<" number of pads are "<<pads_arr[i].nr_pads<<std::endl;

        total_pads += pads_arr[i].nr_pads;

    }

    std::cout<<"For plane W pads = "<<total_pads<<std::endl;


    std::array<std::array<std::pair<double, double>, 72>, 92> v_coord;

 
    out_csv.open(file_name);


    for(auto w = 0; w < 92; w++){

        auto lineW = new TF1(Form("w%02d", w), "pol1", -10, 120);
        lineW->SetParameters(99.593 - 2*(0.866 * w), tan(M_PI/6));




        //need to find out the x_min for each strip
        double x_min = 0;

        if(w < 55){

            x_min = 0.75;

        }else if(w > 54 && w < 61){

            x_min = 0.75 + (w - 54) * 1.5;

        }else{

            x_min = 9.75 + (w - 60) * 3;

        }




        for(auto i = 0; i < pads_arr[w].nr_pads; i++){

            coordVal loc_coord;

            out_csv << std::fixed << std::setprecision(2) << w << "," << pads_arr[w].nr_pads - i - 1 << "," << x_min << "," << lineW->Eval(x_min) << "\n";





            loc_coord .strip = w;
            loc_coord.pad = pads_arr[w].nr_pads - i - 1;
            loc_coord.x = x_min;
            loc_coord.y = lineW->Eval(x_min);


            x_min += 1.5;
            loc_vect.push_back(loc_coord);

        }



    }






    out_csv.close();




    return loc_vect;


}









void drawLines()
{

    auto makeCanvas = new TF1("makeCanvas", "x", -1, 107);
    makeCanvas->SetTitle("Strips");
    makeCanvas->SetLineColor(kWhite);
    makeCanvas->Draw();


    for(auto u = 0; u < 72; u++) {
        auto y1 = 0.;
        auto y2 = 105.;
        auto x1 = 106.5 - (u * 1.5);
        auto x2 = x1;
        auto lineU = new TLine(x1, y1, x2, y2);
        lineU->SetLineColor(kBlue);
        lineU->Draw("SAME");
    }


    for(auto v = 0; v < 92; v++){

        auto lineV = new TF1(Form("v_%02d", v), "pol1", -10, 120);
        lineV->SetParameters(4.33 + 2*(0.866 * v), tan(-M_PI/6));
        lineV->SetLineColor(kRed);
        lineV->Draw("SAME");

    }

    for(auto w = 0; w < 92; w++) {
        auto lineW = new TF1(Form("w%02d", w), "pol1", -10, 120);
        lineW->SetParameters(99.593 - 2*(0.866 * w), tan(M_PI/6));
        lineW->SetLineColor(kGreen);
        lineW->Draw("SAME");
    }





}




void drawPoints(std::vector<coordVal> &val_u, std::vector<coordVal> &val_v, std::vector<coordVal> &val_w, bool opt_u, bool opt_v, bool opt_w)
{


    auto loc_hist_u = new TH2D("hist_name_u", "hist_title_u", 301, -100, 200, 301, -100, 200);


    if(opt_u){

        for(auto &u : val_u){

            loc_hist_u->Fill(u.x, u.y);

        }

        loc_hist_u->SetMarkerStyle(8);
        loc_hist_u->SetMarkerColor(kBlue); 
        loc_hist_u->Draw("SAME");

    }




    auto loc_hist_v = new TH2D("hist_name_v", "hist_title_v", 301, -100, 200, 301, -100, 200);

    if(opt_v){



        for(auto &v : val_v){

            loc_hist_v->Fill(v.x, v.y);

        }

        loc_hist_v->SetMarkerStyle(8);
        loc_hist_v->SetMarkerColor(kRed);
        loc_hist_v->Draw("SAME");


    }




    auto loc_hist_w = new TH2D("hist_name_w", "hist_title_w", 301, -100, 200, 301, -100, 200);


    if(opt_w){



        for(auto &w : val_w){

            loc_hist_w->Fill(w.x, w.y);

        }

        loc_hist_w->SetMarkerStyle(8);
        loc_hist_w->SetMarkerColor(kGreen);
        loc_hist_w->Draw("SAME");

    }







}



void makeMap(std::map<std::pair<int, int>, std::vector<int>> &relationVW_U, std::vector<coordVal> val_u, std::vector<coordVal> val_v, std::vector<coordVal> val_w, bool verbose)
{


    for(auto &u : val_u){

        for(auto &v : val_v){

            if(u == v){

                for(auto &w : val_w){

                    if(u == w && v == w){


                        if(relationVW_U.count({v.strip, w.strip}) == 0){

                            std::vector<int> loc_vector_u;
                            loc_vector_u.push_back(u.strip);
                            relationVW_U.insert({{v.strip, w.strip}, loc_vector_u});

                        }else{

                            if(verbose)
                                std::cerr << "Error, key already exists, " << relationVW_U.at({v.strip, w.strip}).size() << " values found at "
                                    <<v.strip<< " " << w.strip << "; current value for u is " << u.strip << std::endl;
                            relationVW_U.at({v.strip, w.strip}).push_back(u.strip);

                        }


                    }

                }


            }


        }

    }



}






std::map<std::pair<int, int>, std::vector<int>> try_build_coords()
{

    std::vector<coordVal> val_u;

    std::vector<coordVal> val_v;

    std::vector<coordVal> val_w;

    std::map<std::pair<int, int>, std::vector<int>>relationVW_U;

    val_u = buildUcoords("point_coords_u.csv", 0);


    val_v = buildVcoords("point_coords_v.csv", 0);

    val_w = buildWcoords("point_coords_w.csv", 0);



    std::cout<<"Vectors have the elements "<<val_u.size()<<"; "<<val_v.size()<<"; "<<val_w.size()<<";"<<std::endl;


    auto loc_canv = new TCanvas("name", "title");



    drawPoints(val_u, val_v, val_w, 1, 1, 1);

    loc_canv->Update();


    makeMap(relationVW_U, val_u, val_v, val_w, 0);


    //std::cout<<"Map size is "<<relationVW_U.size()<<std::endl;


    return relationVW_U;



}