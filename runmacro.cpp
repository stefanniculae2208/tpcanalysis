#include <iostream>
#include <random>
#include <cstdio>
#include <limits>


#include "TString.h"
#include "src/loadData.cpp"
#include "src/convertUVW.cpp"
#include "src/convertXYZ.cpp"
#include "src/convertHitData.cpp"
#include "include/ErrorCodesMap.hpp"

#include "include/generalDataStorage.hpp"



#include "TSystem.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TError.h"
#include "TColor.h"
#include "TMarker.h"




#include "dict/src/GDataSample.cpp"
#include "dict/src/GDataChannel.cpp"
#include "dict/src/GFrameHeader.cpp"
#include "dict/src/GDataFrame.cpp"







void createNormCSV()
{

    TString filename = "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root";

    auto goodTree = "tree";


    loadData good_data(filename, goodTree);

    auto err = good_data.openFile();


    err = good_data.readData();


    auto max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;


    generalDataStorage data_container;







    //we always use 20 entries
    std::array<std::array<double, 20>, 3> entry_mean = {{}};
    
    std::array<double, 3> total_mean = {};

    std::array<int, 20> entry_nrs = {7, 15, 22, 36, 37, 40, 49, 52, 53, 55,
                                     56, 61, 65, 68, 78, 102, 103, 143, 152, 161};





    for(auto i = 0; i < 20; i++){

        auto err = good_data.decodeData(entry_nrs.at(i));
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<std::endl;








        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<std::endl;





        data_container.uvw_data = loc_conv_uvw.returnDataUVW();


        int count_u = 0;
        int count_v = 0;
        int count_w = 0;


        for(auto &uvw_entry : data_container.uvw_data){

            if(uvw_entry.plane_val == 0){

                entry_mean[0][i] += std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();
                                    
                count_u++;

            }else if(uvw_entry.plane_val == 1){

                entry_mean[1][i] += std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();

                count_v++;

            }
            else if(uvw_entry.plane_val == 2){

                entry_mean[2][i] += std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();

                count_w++;

            }

        }

        entry_mean[0][i] /= count_u;
        entry_mean[1][i] /= count_v;
        entry_mean[2][i] /= count_w;



    }


    total_mean[0] = std::accumulate(entry_mean[0].begin(), entry_mean[0].end(), 0.0) / entry_mean[0].size();
    total_mean[1] = std::accumulate(entry_mean[1].begin(), entry_mean[1].end(), 0.0) / entry_mean[1].size();
    total_mean[2] = std::accumulate(entry_mean[2].begin(), entry_mean[2].end(), 0.0) / entry_mean[2].size();





    std::array<std::array<std::array<double, 20>, 93>, 3> entry_ch_ratio = {{{}}};

    std::array<std::array<double, 93>, 3> total_ch_ratio = {{}};



    for(auto i = 0; i < 20; i++){

        auto err = good_data.decodeData(entry_nrs.at(i));
        

        data_container.root_raw_data = good_data.returnRawData();









        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        





        data_container.uvw_data = loc_conv_uvw.returnDataUVW();


        for(auto &uvw_entry : data_container.uvw_data){

            if(uvw_entry.plane_val == 0){

                double loc_mean = std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();

                entry_ch_ratio[0][uvw_entry.strip_nr][i] = loc_mean / total_mean[0];


            }else if(uvw_entry.plane_val == 1){

                double loc_mean = std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();

                entry_ch_ratio[1][uvw_entry.strip_nr][i] = loc_mean / total_mean[1];

            }
            else if(uvw_entry.plane_val == 2){

                double loc_mean = std::accumulate(uvw_entry.signal_val.begin(), uvw_entry.signal_val.end(), 0.0)
                                         / uvw_entry.signal_val.size();

                entry_ch_ratio[2][uvw_entry.strip_nr][i] = loc_mean / total_mean[2];
                
            }

        }

        



    }


    std::ofstream out_file("./ch_norm_ratios.csv");

    out_file<<"plane,strip,ratio\n";



    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 93; j++) {
            double sum = std::accumulate(entry_ch_ratio[i][j].begin(), entry_ch_ratio[i][j].end(), 0.0);
            total_ch_ratio[i][j] = sum / entry_ch_ratio[i][j].size();

            out_file<<i<<","<<j<<","<<total_ch_ratio[i][j]<<"\n";

        }
    }



    


    









    out_file.close();




}





void view_raw_data(TString fileName, int norm_opt = 0)
{





    int entry_nr = -1;
    int max_entries;




    auto goodFile = fileName;

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if(err != 0)
        return;

    


    



 













    auto loc_canv = new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0,0,1,1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();
    
    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;






    



    

    uint32_t loop_nr = 0;


    while(1){

        generalDataStorage data_container;
        

        std::cout<<"Press 'e' to exit, 'd' for next and 'a' for previous and i to input the entry number. Loop "<<loop_nr<<std::endl;

        loop_nr++;

        char key_val = getchar();

        // Clear the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if(key_val == 'e'){

            break;

        }else if(key_val == 'd'){

            if(entry_nr < max_entries){
                entry_nr++;
            }

            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }else if(key_val == 'a'){

            if(entry_nr > 0){
                entry_nr--;
            }

            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }else if(key_val == 'i'){

            std::cout<<"Please enter the entry number.\n";

            std::cin>>entry_nr;

            if(entry_nr < 0 || entry_nr > (max_entries - 1))
                entry_nr = 0;


            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }




        if(entry_nr < 0){
            entry_nr = 0;
        }



        






        err = good_data.decodeData(entry_nr);
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<std::endl;








        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<std::endl;


        //use norm_opt to set normalization on or off
        if(norm_opt){

            err = loc_conv_uvw.normalizeChannels();
            if(err != 0)
                std::cout<<"Normalize channels error code "<<err<<std::endl;

        }



        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<std::endl;







        if(u_hists != nullptr)
            u_hists->Reset();

        if(v_hists != nullptr)
            v_hists->Reset();

        if(w_hists != nullptr)
            w_hists->Reset();



        








        u_hists = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






        int bin = 0;


        for(auto iter : data_container.uvw_data){

            bin = 0;




            if(iter.plane_val == 0){

                for(auto sig_iter : iter.signal_val){

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 1){

                for(auto sig_iter : iter.signal_val){

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 2){

                for(auto sig_iter : iter.signal_val){

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


        }

        loc_pad->cd(1); u_hists->Draw("COLZ");
        loc_pad->cd(2); v_hists->Draw("COLZ");
        loc_pad->cd(3); w_hists->Draw("COLZ");






        loc_canv->Update();





    }






    loc_canv->WaitPrimitive();






}



void create_entries_pdf(TString source_file, TString destination_file, int read_entries)
{








    int entry_nr = 0;
    int max_entries;




    auto goodFile = source_file;

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    


    convertHitData loc_convert_hit;

    



    convertXYZ loc_conv_xyz;


    



 











    gROOT->SetBatch(kTRUE);

    auto loc_canv = new TCanvas("entries.pdf", "PDF", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0,0,1,1);
    loc_pad->Divide(3, 3);
    loc_pad->Draw();
    
    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;


    TGraph *u_graph = nullptr;
    TGraph *v_graph = nullptr;
    TGraph *w_graph = nullptr;
    


    TGraph *p_graph = nullptr;
    TGraph2D *p_graph3d = nullptr;


    


    loc_canv->Print(destination_file + "[");
    gErrorIgnoreLevel = kWarning;

    




    //only read the number of entries wanted
    max_entries = std::min(max_entries, read_entries);


    while(entry_nr < max_entries){

        generalDataStorage data_container;
        
        std::cout<<"\n\n\nNow at entry: "<<entry_nr<<" of "<<max_entries<<" for "<<destination_file<<"\n";




        


        

        err = good_data.decodeData(entry_nr);
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<"\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<"\n";






        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<"\n";



        err = loc_conv_uvw.substractBl();

        if(err != 0)
            std::cout<<"Substractbl error code "<<err<<"\n";


        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<"\n";


        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if(err != 0)
            std::cout<<"Error set UVW data code "<<err<<"\n";

        err = loc_convert_hit.getHitInfo();
        if(err != 0)
            std::cout<<"Error get hit info code "<<err<<"\n";

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout<<"Hit data size "<<data_container.hit_data.size()<<"\n";
        std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<"\n";




        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if(err != 0)
            std::cout<<"Error get new vector code "<<err<<"\n";

        err = loc_conv_xyz.makeConversionXYZ();
        if(err != 0)
            std::cout<<"Error make conversion XYZ code "<<err<<"\n";


        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout<<"XYZ vector size "<<data_container.xyz_data.size()<<"\n";













        if(u_hists != nullptr)
            u_hists->Reset();

        if(v_hists != nullptr)
            v_hists->Reset();

        if(w_hists != nullptr)
            w_hists->Reset();



        if(u_graph != nullptr)
            u_graph->Delete();

        if(v_graph != nullptr)
            v_graph->Delete();

        if(w_graph != nullptr)
            w_graph->Delete();

        




        if(p_graph != nullptr)
            p_graph->Delete();

        if(p_graph3d != nullptr)
            p_graph3d->Delete();

            

        








        u_hists = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






        int bin = 0;


        for(auto iter : data_container.uvw_data){

            bin = 0;




            if(iter.plane_val == 0){

                for(auto sig_iter : iter.signal_val){

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 1){

                for(auto sig_iter : iter.signal_val){

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 2){

                for(auto sig_iter : iter.signal_val){

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


        }

        loc_pad->cd(1); u_hists->Draw("COLZ");
        loc_pad->cd(2); v_hists->Draw("COLZ");
        loc_pad->cd(3); w_hists->Draw("COLZ");




        std::vector<double> x_u;
        std::vector<double> y_u;
        std::vector<double> c_u;

        std::vector<double> x_v;
        std::vector<double> y_v;
        std::vector<double> c_v;

        std::vector<double> x_w;
        std::vector<double> y_w;
        std::vector<double> c_w;

        

        for(auto hit_iter : data_container.hit_data){

            if(hit_iter.plane == 0){

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);
                c_u.push_back(hit_iter.peak_y + hit_iter.base_line);


            }else if(hit_iter.plane == 1){

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);
                c_v.push_back(hit_iter.peak_y + hit_iter.base_line);

            }else if(hit_iter.plane == 2){

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);
                c_w.push_back(hit_iter.peak_y + hit_iter.base_line);

            }

        }








        


        

        
        if(data_container.hit_data.size() != 0){
            


            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            //u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            //Set marker colors Pink > Red > Green > Blue
            if(c_u.size() != 0){

                auto min_val_c_u = 0;
                auto max_val_c_u = 2000;

                std::transform(c_u.begin(), c_u.end(), c_u.begin(),
                [min_val_c_u, max_val_c_u](const double x) { return (x - min_val_c_u) / (max_val_c_u - min_val_c_u); });

                for(auto vec_i = 0; vec_i < c_u.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    u_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_u.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_u.at(vec_i));
                    }else if(c_u.at(vec_i) >= 0.33 && c_u.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_u.at(vec_i) / 2, 0);
                    }else if(c_u.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_u.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    u_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(4);u_graph->Draw("AP");
            

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            //v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            if(c_v.size() != 0){

                auto min_val_c_v = 0;
                auto max_val_c_v = 2000;

                std::transform(c_v.begin(), c_v.end(), c_v.begin(),
                [min_val_c_v, max_val_c_v](const double x) { return (x - min_val_c_v) / (max_val_c_v - min_val_c_v); });

                for(auto vec_i = 0; vec_i < c_v.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    v_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_v.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_v.at(vec_i));
                    }else if(c_v.at(vec_i) >= 0.33 && c_v.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_v.at(vec_i) / 2, 0);
                    }else if(c_v.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_v.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    v_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(5);v_graph->Draw("AP");
            

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            //w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            if(c_w.size() != 0){

                auto min_val_c_w = 0;
                auto max_val_c_w = 2000;

                std::transform(c_w.begin(), c_w.end(), c_w.begin(),
                [min_val_c_w, max_val_c_w](const double x) { return (x - min_val_c_w) / (max_val_c_w - min_val_c_w); });

                for(auto vec_i = 0; vec_i < c_w.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    w_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_w.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_w.at(vec_i));
                    }else if(c_w.at(vec_i) >= 0.33 && c_w.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_w.at(vec_i) / 2, 0);
                    }else if(c_w.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_w.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    w_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(6);w_graph->Draw("AP");



            loc_pad->Modified();
            loc_pad->Update();

        }else{
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }


        if(data_container.xyz_data.size() != 0){

            std::vector<double> x, y, z, chg;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
            chg.push_back(0.0);
    


            for(auto point_iter : data_container.xyz_data){

                x.push_back(point_iter.data_x);
                y.push_back(point_iter.data_y);
                z.push_back(point_iter.data_z);
                chg.push_back(point_iter.data_charge);
        

            }

            x.push_back(150.0);
            y.push_back(150.0);
            z.push_back(150.0);
            chg.push_back(0.0);




            
            p_graph = new TGraph(x.size(), x.data(), y.data());
            p_graph->GetXaxis()->SetLimits(-10, 150);
            p_graph->GetHistogram()->SetMaximum(150);
            p_graph->GetHistogram()->SetMinimum(-10);
            //p_graph->SetMarkerColor(kBlack);
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle("XY coordinates projection; X axis; Y axis");
            if(chg.size() != 0){

                auto min_val_chg = 0;
                auto max_val_chg = 6000;

                std::transform(chg.begin(), chg.end(), chg.begin(),
                [min_val_chg, max_val_chg](const double x) { return (x - min_val_chg) / (max_val_chg - min_val_chg); });

                for(auto vec_i = 0; vec_i < chg.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    p_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(chg.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * chg.at(vec_i));
                    }else if(chg.at(vec_i) >= 0.33 && chg.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * chg.at(vec_i) / 2, 0);
                    }else if(chg.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(chg.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    p_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(7);p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            



            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X axis; Y axis; Z axis");
            loc_pad->cd(8); p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();



        }else{
            p_graph = nullptr;
            p_graph3d = nullptr;
        }

        entry_nr++;

        loc_pad->Update();
        loc_canv->Update();
        loc_canv->Print(destination_file);

    }



/* 
        std::vector<double> x_u;
        std::vector<double> y_u;

        std::vector<double> x_v;
        std::vector<double> y_v;

        std::vector<double> x_w;
        std::vector<double> y_w;

        

        for(auto hit_iter : data_container.hit_data){

            if(hit_iter.plane == 0){

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);


            }else if(hit_iter.plane == 1){

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);

            }else if(hit_iter.plane == 2){

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);

            }

        }








        


        

        
        if(data_container.hit_data.size() != 0){

        


            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            loc_pad->cd(4);u_graph->Draw("AP");
            

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            loc_pad->cd(5);v_graph->Draw("AP");
            

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            loc_pad->cd(6);w_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

        }else{
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }


        if(data_container.xyz_data.size() != 0){

            std::vector<double> x, y, z;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
    


            for(auto point_iter : data_container.xyz_data){

                x.push_back(point_iter.data_x);
                y.push_back(point_iter.data_y);
                z.push_back(point_iter.data_z);

        

            }

            x.push_back(150.0);
            y.push_back(150.0);
            z.push_back(150.0);




            
            p_graph = new TGraph(x.size(), x.data(), y.data());
            p_graph->GetXaxis()->SetLimits(-10, 150);
            p_graph->GetHistogram()->SetMaximum(150);
            p_graph->GetHistogram()->SetMinimum(-10);
            p_graph->SetMarkerColor(kBlack);
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle("XY coordinates projection; X axis; Y axis");
            loc_pad->cd(7);p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            



            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X axis; Y axis; Z axis");
            loc_pad->cd(8); p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();



        }else{
            p_graph = nullptr;
            p_graph3d = nullptr;
        }
        

























        entry_nr++;

        loc_canv->Update();
        loc_canv->Print(destination_file);





    } */



    

    gErrorIgnoreLevel = kPrint;
    loc_canv->Print(destination_file + "]");


    loc_canv->Close();

    if(loc_canv){
        delete(loc_canv);
        loc_canv = nullptr;
    }
        

    gROOT->SetBatch(kFALSE);


    std::cout<<"\n\n\nDONE!!!!!\n\n\n"<<std::endl;



}


void create_raw_pdf(TString source_file, TString destination_file, int read_entries)
{








    int entry_nr = 0;
    int max_entries;




    auto goodFile = source_file;

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    


    convertHitData loc_convert_hit;

    



    convertXYZ loc_conv_xyz;


    



 











    gROOT->SetBatch(kTRUE);

    auto loc_canv = new TCanvas("entries.pdf", "PDF", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0,0,1,1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();
    
    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;



    


    loc_canv->Print(destination_file + "[");
    gErrorIgnoreLevel = kWarning;

    



    //only read the number of entries wanted
    max_entries = std::min(max_entries, read_entries);


    while(entry_nr < max_entries){

        generalDataStorage data_container;
        
        std::cout<<"\n\n\nNow at entry: "<<entry_nr<<" of "<<max_entries<<" for "<<destination_file<<"\n";




        


        

        err = good_data.decodeData(entry_nr);
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<"\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<"\n";






        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<"\n";



        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<"\n";














        if(u_hists != nullptr)
            u_hists->Reset();

        if(v_hists != nullptr)
            v_hists->Reset();

        if(w_hists != nullptr)
            w_hists->Reset();

            

        








        u_hists = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






        int bin = 0;
        //int strip = 1;

        for(auto iter : data_container.uvw_data){

            bin = 0;




            if(iter.plane_val == 0){

                for(auto sig_iter : iter.signal_val){

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 1){

                for(auto sig_iter : iter.signal_val){

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 2){

                for(auto sig_iter : iter.signal_val){

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


        }

        loc_pad->cd(1); u_hists->Draw("COLZ");
        loc_pad->cd(2); v_hists->Draw("COLZ");
        loc_pad->cd(3); w_hists->Draw("COLZ");




        entry_nr++;

        loc_pad->Update();
        loc_canv->Update();
        loc_canv->Print(destination_file);

    }




    

    gErrorIgnoreLevel = kPrint;
    loc_canv->Print(destination_file + "]");


    loc_canv->Close();

    if(loc_canv){
        delete(loc_canv);
        loc_canv = nullptr;
    }
        

    gROOT->SetBatch(kFALSE);


    std::cout<<"\n\n\nDONE!!!!!\n\n\n"<<std::endl;



}






void writeXYZcvs(int entry_nr)
{

    generalDataStorage data_container;






    auto goodFile = "./rootdata/data2.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();


    convertUVW loc_conv_uvw(data_container.root_raw_data);


    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
        std::cout<<"Make conversion error code "<<err<<std::endl;



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    err = loc_conv_uvw.drawChargeHist();

    if(err != 0)
        std::cout<<"Draw Charge Hist error code "<<err<<std::endl;



    data_container.uvw_data = loc_conv_uvw.returnDataUVW();



    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if(err != 0)
        std::cout<<"Error get hit info code "<<err<<std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout<<"Hit data size "<<data_container.hit_data.size()<<std::endl;
    std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<std::endl;



    convertXYZ loc_conv_xyz(data_container.hit_data);


    err = loc_conv_xyz.makeConversionXYZ();


    data_container.xyz_data = loc_conv_xyz.returnXYZ();

    std::ofstream out_file("./converteddata/test_xyz.csv");

    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open file for output" << std::endl;
    }


    //header
    out_file << "x,y,z\n";


    for(auto &data_entry : data_container.xyz_data){

        out_file << data_entry.data_x << "," << data_entry.data_y << "," << data_entry.data_z;


        out_file << "\n";

    }





    out_file.close();




 





}



void writeFullCSV()
{

    auto goodFile = "./rootdata/data2.root";

    auto goodTree = "tree";



    std::ofstream out_file("./converteddata/data2allentries.csv");

    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open file for output" << std::endl;
        return;
    }






    loadData good_data(goodFile, goodTree);


    convertUVW loc_conv_uvw;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;


    auto err = good_data.openFile();
    err = good_data.readData();

    err = loc_conv_uvw.openSpecFile();


    auto max_entries = good_data.returnNEntries();


    //header
    out_file << "x,y,z,entry_nr\n";

    int entry_nr = 0;


    while(entry_nr < max_entries){

        std::cout<<"\n\n\nNow at entry: "<<entry_nr<<" of "<<max_entries<<"\n";


        generalDataStorage data_container;


        err = good_data.decodeData(entry_nr);
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<"\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<"\n";






        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<"\n";



        err = loc_conv_uvw.substractBl();

        if(err != 0)
            std::cout<<"Substractbl error code "<<err<<"\n";


        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<"\n";


        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if(err != 0)
            std::cout<<"Error set UVW data code "<<err<<"\n";

        err = loc_convert_hit.getHitInfo();
        if(err != 0)
            std::cout<<"Error get hit info code "<<err<<"\n";

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout<<"Hit data size "<<data_container.hit_data.size()<<"\n";
        std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<"\n";




        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if(err != 0)
            std::cout<<"Error get new vector code "<<err<<"\n";

        err = loc_conv_xyz.makeConversionXYZ();
        if(err != 0)
            std::cout<<"Error make conversion XYZ code "<<err<<"\n";


        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout<<"XYZ vector size "<<data_container.xyz_data.size()<<"\n";


        


        for(auto &data_entry : data_container.xyz_data){

            out_file << data_entry.data_x << "," << data_entry.data_y << "," << data_entry.data_z<< "," << entry_nr;


            out_file << "\n";

        }





        entry_nr++;


    }



    

    


    out_file.close();







}





void drawXYimage(int entry_nr)
{


    generalDataStorage data_container;






    auto goodFile = "./rootdata/data2.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();


    convertUVW loc_conv_uvw(data_container.root_raw_data);


    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
        std::cout<<"Make conversion error code "<<err<<std::endl;



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    err = loc_conv_uvw.drawChargeHist();

    if(err != 0)
        std::cout<<"Draw Charge Hist error code "<<err<<std::endl;



    data_container.uvw_data = loc_conv_uvw.returnDataUVW();



    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if(err != 0)
        std::cout<<"Error get hit info code "<<err<<std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout<<"Hit data size "<<data_container.hit_data.size()<<std::endl;
    std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<std::endl;



    convertXYZ loc_conv_xyz(data_container.hit_data);


    err = loc_conv_xyz.makeConversionXYZ();


    data_container.xyz_data = loc_conv_xyz.returnXYZ();


    std::vector<double> x, y, z;
    


    for(auto point_iter : data_container.xyz_data){

        x.push_back(point_iter.data_x);
        y.push_back(point_iter.data_y);
        z.push_back(point_iter.data_z);

 

    }



    auto loc_canv = new TCanvas("xy format", "", 800, 600);
    loc_canv->SetFrameLineColor(0);
    loc_canv->SetFrameLineWidth(0);
    loc_canv->SetBottomMargin(0);
    loc_canv->SetTopMargin(0);
    loc_canv->SetLeftMargin(0);
    loc_canv->SetRightMargin(0);

    
    gStyle->SetOptStat(0);






    auto p_graph = new TGraph(x.size(), x.data(), y.data());

    p_graph->GetXaxis()->SetTickLength(0);
    p_graph->GetXaxis()->SetLabelSize(0);
    p_graph->GetYaxis()->SetTickLength(0);
    p_graph->GetYaxis()->SetLabelSize(0);



    p_graph->GetXaxis()->SetLimits(-10, 150);
    p_graph->GetHistogram()->SetMaximum(150);
    p_graph->GetHistogram()->SetMinimum(-10);
    p_graph->SetMarkerColor(kBlue);
    p_graph->SetMarkerStyle(kFullCircle);
    p_graph->SetTitle("");
    p_graph->Draw("AP");





    loc_canv->Update();
    loc_canv->Print("./converteddata/test_xyz.png");

    loc_canv->Close();


}





void view_data_entries(TString fileName, int norm_opt = 0)
{





    int entry_nr = -1;
    int max_entries;




    auto goodFile = fileName;

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if(err != 0)
        return;


    


    convertHitData loc_convert_hit;

    



    convertXYZ loc_conv_xyz;


    



 













    auto loc_canv = new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0,0,1,1);
    loc_pad->Divide(3, 3);
    loc_pad->Draw();
    
    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;


    TGraph *u_graph = nullptr;
    TGraph *v_graph = nullptr;
    TGraph *w_graph = nullptr;


    TGraph *p_graph = nullptr;
    TGraph2D *p_graph3d = nullptr;




    



    

    uint32_t loop_nr = 0;


    while(1){

        generalDataStorage data_container;
        

        std::cout<<"Press 'e' to exit, 'd' for next and 'a' for previous and i to input the entry number. Loop "<<loop_nr<<std::endl;

        loop_nr++;

        char key_val = getchar();

        // Clear the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if(key_val == 'e'){

            break;

        }else if(key_val == 'd'){

            if(entry_nr < max_entries){
                entry_nr++;
            }

            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }else if(key_val == 'a'){

            if(entry_nr > 0){
                entry_nr--;
            }

            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }else if(key_val == 'i'){

            std::cout<<"Please enter the entry number.\n";

            std::cin>>entry_nr;

            if(entry_nr < 0 || entry_nr > (max_entries - 1))
                entry_nr = 0;


            std::cout<<"Entry number is "<<entry_nr<<"\n\n"<<std::endl;

        }




        if(entry_nr < 0){
            entry_nr = 0;
        }



        






        err = good_data.decodeData(entry_nr);
        if(err != 0){
            std::cout<<"Error decode data code "<<err<<std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<std::endl;








        
        loc_conv_uvw.setRawData(data_container.root_raw_data);

        

        err = loc_conv_uvw.makeConversion();
        if(err != 0)
            std::cout<<"Make conversion error code "<<err<<std::endl;

        
        //use norm_opt to set normalization on or off
        if(norm_opt){

            err = loc_conv_uvw.normalizeChannels();
            if(err != 0)
                std::cout<<"Normalize channels error code "<<err<<std::endl;

        }



        err = loc_conv_uvw.substractBl();

        if(err != 0)
            std::cout<<"Substractbl error code "<<err<<std::endl;


        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<std::endl;


        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if(err != 0)
            std::cout<<"Error set UVW data code "<<err<<std::endl;

        err = loc_convert_hit.getHitInfo();
        if(err != 0)
            std::cout<<"Error get hit info code "<<err<<std::endl;

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout<<"Hit data size "<<data_container.hit_data.size()<<std::endl;
        std::cout<<"Hist data size "<<data_container.raw_hist_container.size()<<std::endl;


        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if(err != 0)
            std::cout<<"Error get new vector code "<<err<<std::endl;

        err = loc_conv_xyz.makeConversionXYZ();
        if(err != 0)
            std::cout<<"Error make conversion XYZ code "<<err<<std::endl;


        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout<<"XYZ vector size "<<data_container.xyz_data.size()<<std::endl;












        if(u_hists != nullptr)
            u_hists->Reset();

        if(v_hists != nullptr)
            v_hists->Reset();

        if(w_hists != nullptr)
            w_hists->Reset();



        if(u_graph != nullptr)
            u_graph->Delete();

        if(v_graph != nullptr)
            v_graph->Delete();

        if(w_graph != nullptr)
            w_graph->Delete();

        if(p_graph != nullptr)
            p_graph->Delete();

        if(p_graph3d != nullptr)
            p_graph3d->Delete();

        








        u_hists = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






        int bin = 0;


        for(auto iter : data_container.uvw_data){

            bin = 0;




            if(iter.plane_val == 0){

                for(auto sig_iter : iter.signal_val){

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 1){

                for(auto sig_iter : iter.signal_val){

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


            if(iter.plane_val == 2){

                for(auto sig_iter : iter.signal_val){

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

            }


        }

        loc_pad->cd(1); u_hists->Draw("COLZ");
        loc_pad->cd(2); v_hists->Draw("COLZ");
        loc_pad->cd(3); w_hists->Draw("COLZ");




        std::vector<double> x_u;
        std::vector<double> y_u;
        std::vector<double> c_u;

        std::vector<double> x_v;
        std::vector<double> y_v;
        std::vector<double> c_v;

        std::vector<double> x_w;
        std::vector<double> y_w;
        std::vector<double> c_w;

        

        for(auto hit_iter : data_container.hit_data){

            if(hit_iter.plane == 0){

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);
                c_u.push_back(hit_iter.peak_y + hit_iter.base_line);


            }else if(hit_iter.plane == 1){

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);
                c_v.push_back(hit_iter.peak_y + hit_iter.base_line);

            }else if(hit_iter.plane == 2){

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);
                c_w.push_back(hit_iter.peak_y + hit_iter.base_line);

            }


        }








        


        

        
        if(data_container.hit_data.size() != 0){
            


            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            //u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            //Set marker colors Pink > Red > Green > Blue
            if(c_u.size() != 0){

                auto min_val_c_u = 0;
                auto max_val_c_u = 2000;

                std::transform(c_u.begin(), c_u.end(), c_u.begin(),
                [min_val_c_u, max_val_c_u](const double x) { return (x - min_val_c_u) / (max_val_c_u - min_val_c_u); });

                for(auto vec_i = 0; vec_i < c_u.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    u_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_u.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_u.at(vec_i));
                    }else if(c_u.at(vec_i) >= 0.33 && c_u.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_u.at(vec_i) / 2, 0);
                    }else if(c_u.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_u.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    u_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(4);u_graph->Draw("AP");
            

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            //v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            if(c_v.size() != 0){

                auto min_val_c_v = 0;
                auto max_val_c_v = 2000;

                std::transform(c_v.begin(), c_v.end(), c_v.begin(),
                [min_val_c_v, max_val_c_v](const double x) { return (x - min_val_c_v) / (max_val_c_v - min_val_c_v); });

                for(auto vec_i = 0; vec_i < c_v.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    v_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_v.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_v.at(vec_i));
                    }else if(c_v.at(vec_i) >= 0.33 && c_v.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_v.at(vec_i) / 2, 0);
                    }else if(c_v.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_v.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    v_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(5);v_graph->Draw("AP");
            

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            //w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            if(c_w.size() != 0){

                auto min_val_c_w = 0;
                auto max_val_c_w = 2000;

                std::transform(c_w.begin(), c_w.end(), c_w.begin(),
                [min_val_c_w, max_val_c_w](const double x) { return (x - min_val_c_w) / (max_val_c_w - min_val_c_w); });

                for(auto vec_i = 0; vec_i < c_w.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    w_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(c_w.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * c_w.at(vec_i));
                    }else if(c_w.at(vec_i) >= 0.33 && c_w.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * c_w.at(vec_i) / 2, 0);
                    }else if(c_w.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(c_w.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    w_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(6);w_graph->Draw("AP");



            loc_pad->Modified();
            loc_pad->Update();

        }else{
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }


        if(data_container.xyz_data.size() != 0){

            std::vector<double> x, y, z, chg;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
            chg.push_back(0.0);
    


            for(auto point_iter : data_container.xyz_data){

                x.push_back(point_iter.data_x);
                y.push_back(point_iter.data_y);
                z.push_back(point_iter.data_z);
                chg.push_back(point_iter.data_charge);
        

            }

            x.push_back(150.0);
            y.push_back(150.0);
            z.push_back(150.0);
            chg.push_back(0.0);




            
            p_graph = new TGraph(x.size(), x.data(), y.data());
            p_graph->GetXaxis()->SetLimits(-10, 150);
            p_graph->GetHistogram()->SetMaximum(150);
            p_graph->GetHistogram()->SetMinimum(-10);
            //p_graph->SetMarkerColor(kBlack);
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle("XY coordinates projection; X axis; Y axis");
            if(chg.size() != 0){

                auto min_val_chg = 0;
                auto max_val_chg = 6000;

                std::transform(chg.begin(), chg.end(), chg.begin(),
                [min_val_chg, max_val_chg](const double x) { return (x - min_val_chg) / (max_val_chg - min_val_chg); });

                for(auto vec_i = 0; vec_i < chg.size(); vec_i++){

                    Double_t loc_x, loc_y;

                    p_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if(chg.at(vec_i) < 0.33){
                        TColor *loc_color = new TColor(0, 0, 3 * chg.at(vec_i));
                    }else if(chg.at(vec_i) >= 0.33 && chg.at(vec_i) < 0.66){
                        TColor *loc_color = new TColor(0, 3 * chg.at(vec_i) / 2, 0);
                    }else if(chg.at(vec_i) > 1){
                        TColor *loc_color = new TColor(1, 0, 1);
                    }else{
                        TColor *loc_color = new TColor(chg.at(vec_i), 0, 0);
                    }


                    
                    
                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    p_graph->GetListOfFunctions()->Add(loc_marker);


                }

            }
            loc_pad->cd(7);p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            



            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X axis; Y axis; Z axis");
            loc_pad->cd(8); p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();



        }else{
            p_graph = nullptr;
            p_graph3d = nullptr;
        }
        



























        loc_canv->Update();





    }






    loc_canv->WaitPrimitive();






}


void drawUVWimage(int entry_nr)
{


    generalDataStorage data_container;






    auto goodFile = "./rootdata/data2.root";

    auto goodTree = "tree";


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();


    convertUVW loc_conv_uvw(data_container.root_raw_data);


    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
        std::cout<<"Make conversion error code "<<err<<std::endl;



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    err = loc_conv_uvw.drawChargeHist();

    if(err != 0)
        std::cout<<"Draw Charge Hist error code "<<err<<std::endl;



    data_container.uvw_data = loc_conv_uvw.returnDataUVW();



    



    auto loc_canv = new TCanvas("uvw format", "UVW hists", 800, 600);
    loc_canv->SetFrameLineColor(0);
    loc_canv->SetFrameLineWidth(0);
    loc_canv->SetBottomMargin(0);
    loc_canv->SetTopMargin(0);
    loc_canv->SetLeftMargin(0);
    loc_canv->SetRightMargin(0);

    
    gStyle->SetOptStat(0);
    

    auto u_hists = new TH2D(Form("u_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);
    auto v_hists = new TH2D(Form("v_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);
    auto w_hists = new TH2D(Form("w_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);

    u_hists->SetTickLength(0);
    TAxis* u_axis = u_hists->GetYaxis();
    u_axis->SetTickLength(0);
    u_hists->SetLabelSize(0);
    u_axis->SetLabelSize(0);

    v_hists->SetTickLength(0);
    TAxis* v_axis = v_hists->GetYaxis();
    v_axis->SetTickLength(0);
    v_hists->SetLabelSize(0);
    v_axis->SetLabelSize(0);

    w_hists->SetTickLength(0);
    TAxis* w_axis = w_hists->GetYaxis();
    w_axis->SetTickLength(0);
    w_hists->SetLabelSize(0);
    w_axis->SetLabelSize(0);

    

    




    int bin = 0;
    //int strip = 1;

    for(auto iter : data_container.uvw_data){

        bin = 0;




        if(iter.plane_val == 0){

            for(auto sig_iter : iter.signal_val){

                u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

                }

        }


        if(iter.plane_val == 1){

            for(auto sig_iter : iter.signal_val){

                v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 2){

            for(auto sig_iter : iter.signal_val){

                w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        }




    u_hists->Draw("COLA");
    loc_canv->Update();
    loc_canv->Print("./converteddata/test_hist_u.png");




    v_hists->Draw("COLA");
    loc_canv->Update();
    loc_canv->Print("./converteddata/test_hist_v.png");


    w_hists->Draw("COLA");
    loc_canv->Update();
    loc_canv->Print("./converteddata/test_hist_w.png");





    

    loc_canv->Close();



}





void mass_convertPDF(TString lin_arg)
{


    TString dirandfileName = lin_arg;

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index+1,dir.Sizeof());


    index = fileName.Last('/');//remove path



    fileName.Remove(0, index+1);
    fileName.Remove(fileName.Sizeof()-6, fileName.Sizeof());//remove .root
    









    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append("pdffiles");

    gROOT->ProcessLine(mkdirCommand);


    TString pdfName = dir + "pdffiles/" + fileName + ".pdf";



    create_entries_pdf(dirandfileName, pdfName, 50);





}


void mass_convertRawPDF(TString lin_arg, int nr_entries)
{


    TString dirandfileName = lin_arg;

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index+1,dir.Sizeof());


    index = fileName.Last('/');//remove path



    fileName.Remove(0, index+1);
    fileName.Remove(fileName.Sizeof()-6, fileName.Sizeof());//remove .root
    









    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append("rawpdf");

    gROOT->ProcessLine(mkdirCommand);


    TString pdfName = dir + "rawpdf/" + fileName + ".pdf";



    create_raw_pdf(dirandfileName, pdfName, nr_entries);





}






void printPeaksByChannel(TString fileName, int entry_nr, int plane, int norm_opt = 0)
{

    auto goodFile = fileName;

    auto goodTree = "tree";

    generalDataStorage data_container;


    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    auto max_entries = good_data.returnNEntries();


    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if(err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if(err != 0)
        return;


    


    //convertHitData loc_convert_hit;

    



    

    err = good_data.decodeData(entry_nr);
    if(err != 0){
        std::cout<<"Error decode data code "<<err<<std::endl;
    }

    data_container.root_raw_data = good_data.returnRawData();

    std::cout<<"RAW data size is "<<data_container.root_raw_data.size()<<std::endl;








        
    loc_conv_uvw.setRawData(data_container.root_raw_data);

        

    err = loc_conv_uvw.makeConversion();
    if(err != 0)
            std::cout<<"Make conversion error code "<<err<<std::endl;



    generalDataStorage data_container_raw;

    data_container_raw.uvw_data = loc_conv_uvw.returnDataUVW();



        
    //use norm_opt to set normalization on or off
    if(norm_opt){

        err = loc_conv_uvw.normalizeChannels();
        if(err != 0)
            std::cout<<"Normalize channels error code "<<err<<std::endl;

    }



    err = loc_conv_uvw.substractBl();

    if(err != 0)
        std::cout<<"Substractbl error code "<<err<<std::endl;


    data_container.uvw_data = loc_conv_uvw.returnDataUVW();

    std::cout<<"UVW data size is "<<data_container.uvw_data.size()<<std::endl;





 








    auto loc_canv = new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0,0,1,1);
    loc_pad->Divide(3, 3);
    loc_pad->Draw();
    
    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;

    TH2D *u_hists_ord = nullptr;
    TH2D *v_hists_ord = nullptr;
    TH2D *w_hists_ord = nullptr;

    TH2D *u_hists_raw = nullptr;
    TH2D *v_hists_raw = nullptr;
    TH2D *w_hists_raw = nullptr;







    u_hists = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    v_hists = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    w_hists = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






    int bin = 0;


    for(auto iter : data_container.uvw_data){

        bin = 0;




        if(iter.plane_val == 0){

            for(auto sig_iter : iter.signal_val){

                u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 1){

            for(auto sig_iter : iter.signal_val){

                v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 2){

            for(auto sig_iter : iter.signal_val){

                w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


    }

    loc_pad->cd(4); u_hists->Draw("COLZ");
    loc_pad->cd(5); v_hists->Draw("COLZ");
    loc_pad->cd(6); w_hists->Draw("COLZ");



    

    std::sort(data_container.uvw_data.begin(), data_container.uvw_data.end(), [](const dataUVW &a, const dataUVW &b)
                { return a.strip_nr < b.strip_nr; });

    
    std::array<std::array<std::size_t, 93>, 3> max_index;


    for(auto uvw_el : data_container.uvw_data){

        auto max_it = std::max_element(uvw_el.signal_val.begin(), uvw_el.signal_val.end());
        max_index[uvw_el.plane_val][uvw_el.strip_nr] = std::distance(uvw_el.signal_val.begin(), max_it);

    }




    std::sort(data_container.uvw_data.begin(), data_container.uvw_data.end(), [max_index](const dataUVW &a, const dataUVW &b)
                { return max_index[a.plane_val][a.strip_nr] < max_index[b.plane_val][b.strip_nr]; });









    generalDataStorage data_container_ord;

    

    std::array<int, 3> plane_counter = {};


    for(auto i = 0; i< data_container.uvw_data.size(); i++){

        data_container_ord.uvw_data.push_back(data_container.uvw_data.at(i));
        data_container_ord.uvw_data.at(i).strip_nr = plane_counter[data_container.uvw_data.at(i).plane_val];

        plane_counter[data_container.uvw_data.at(i).plane_val]++;

    }










    u_hists_ord = new TH2D(Form("u_ord_hists_%d", entry_nr), Form("Histogram ord for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    v_hists_ord = new TH2D(Form("v_ord_hists_%d", entry_nr), Form("Histogram ord for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    w_hists_ord = new TH2D(Form("w_ord_hists_%d", entry_nr), Form("Histogram ord for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);






    for(auto iter : data_container_ord.uvw_data){

        bin = 0;




        if(iter.plane_val == 0){

            for(auto sig_iter : iter.signal_val){

                u_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 1){

            for(auto sig_iter : iter.signal_val){

                v_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 2){

            for(auto sig_iter : iter.signal_val){

                w_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


    }

    
    loc_pad->cd(7); u_hists_ord->Draw("COLZ");
    loc_pad->cd(8); v_hists_ord->Draw("COLZ");
    loc_pad->cd(9); w_hists_ord->Draw("COLZ");


    loc_conv_uvw.setUVWData(data_container_ord.uvw_data);
    loc_conv_uvw.drawChargeHist();







    u_hists_raw = new TH2D(Form("u_hists_%d", entry_nr), Form("Histogram for U entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    v_hists_raw = new TH2D(Form("v_hists_%d", entry_nr), Form("Histogram for V entry %d", entry_nr), 512, 1, 513, 100, 1, 101);
    w_hists_raw = new TH2D(Form("w_hists_%d", entry_nr), Form("Histogram for W entry %d", entry_nr), 512, 1, 513, 100, 1, 101);








    for(auto iter : data_container_raw.uvw_data){

        bin = 0;




        if(iter.plane_val == 0){

            for(auto sig_iter : iter.signal_val){

                u_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 1){

            for(auto sig_iter : iter.signal_val){

                v_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


        if(iter.plane_val == 2){

            for(auto sig_iter : iter.signal_val){

                w_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);

            }

        }


    }

    loc_pad->cd(1); u_hists_raw->Draw("COLZ");
    loc_pad->cd(2); v_hists_raw->Draw("COLZ");
    loc_pad->cd(3); w_hists_raw->Draw("COLZ");

    



























    loc_canv->Update();

    loc_canv->WaitPrimitive();






 









}










void runmacro(TString lin_arg)
{

    //view_data_entries("./rootdata/data2.root", 1);

    //view_raw_data("./rootdata/data2.root", 1);

    //create_entries_pdf("/media/gant/Expansion/tpcanalcsv/data08.root", "/media/gant/Expansion/tpcanalcsv/data08.pdf", 10000);

    /* create_raw_pdf("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T16-20-10.736_0000.root",
                         "/media/gant/Expansion/tpcanalcsv/CoBo_2018-06-20T16-20-10.736_0000.pdf", 10000); */


    //writeXYZcvs(429);

    //drawXYimage(429);


    //writeFullCSV();

    //drawUVWimage(429);

    //mass_convertPDF(lin_arg);

    //mass_convertRawPDF(lin_arg, 10000);

    //createNormCSV();

    printPeaksByChannel("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root", 187, 0, 1);





}