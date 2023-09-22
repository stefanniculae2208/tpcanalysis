#include <cstdio>
#include <glob.h>
#include <iostream>
#include <limits>
#include <map>
#include <random>

#include "TString.h"
#include "include/ErrorCodesMap.hpp"
#include "src/cleanUVW.cpp"
#include "src/convertHitData.cpp"
#include "src/convertUVW.cpp"
#include "src/convertXYZ.cpp"
#include "src/filterEventsXY.cpp"
#include "src/loadData.cpp"

#include "include/generalDataStorage.hpp"

#include "TCanvas.h"
#include "TColor.h"
#include "TError.h"
#include "TF1.h"
#include "TH2D.h"
#include "TMarker.h"
#include "TROOT.h"
#include "TSpectrum.h"
#include "TStyle.h"
#include "TSystem.h"

#include "dict/src/GDataChannel.cpp"
#include "dict/src/GDataFrame.cpp"
#include "dict/src/GDataSample.cpp"
#include "dict/src/GFrameHeader.cpp"

void viewUVWdata_mini(TString fileName) {

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

    if (err != 0)
        return;

    cleanUVW loc_clean_uvw;

    auto loc_canv =
        new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
    loc_pad->Divide(3, 2);
    loc_pad->Draw();

    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;

    TH2D *u_hists_clean = nullptr;
    TH2D *v_hists_clean = nullptr;
    TH2D *w_hists_clean = nullptr;

    uint32_t loop_nr = 0;

    while (1) {

        generalDataStorage data_container;

        generalDataStorage data_container_clean;

        std::cout
            << "Press 'e' to exit, 'd' for next and 'a' for previous and i "
               "to input the entry number. Loop "
            << loop_nr << std::endl;

        loop_nr++;

        char key_val = getchar();

        // Clear the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (key_val == 'e') {

            break;

        } else if (key_val == 'd') {

            if (entry_nr < max_entries) {
                entry_nr++;
            }

            std::cout << "Entry number is " << entry_nr << "\n\n" << std::endl;

        } else if (key_val == 'a') {

            if (entry_nr > 0) {
                entry_nr--;
            }

            std::cout << "Entry number is " << entry_nr << "\n\n" << std::endl;

        } else if (key_val == 'i') {

            std::cout << "Please enter the entry number.\n";

            std::cin >> entry_nr;

            if (entry_nr < 0 || entry_nr > (max_entries - 1))
                entry_nr = 0;

            std::cout << "Entry number is " << entry_nr << "\n\n" << std::endl;
        }

        if (entry_nr < 0) {
            entry_nr = 0;
        }

        err = good_data.decodeData(entry_nr, false);
        if (err != 0) {
            std::cout << "Error decode data code " << err << std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << std::endl;

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion(true, false);
        if (err != 0)
            std::cout << "Make conversion error code " << err << std::endl;

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << std::endl;

        loc_clean_uvw.setUVWData(data_container.uvw_data);

        err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        data_container_clean.uvw_data = loc_clean_uvw.returnDataUVW();

        // Reset hists

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        if (u_hists_clean != nullptr)
            u_hists_clean->Reset();

        if (v_hists_clean != nullptr)
            v_hists_clean->Reset();

        if (w_hists_clean != nullptr)
            w_hists_clean->Reset();

        // Crete new hists

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

        u_hists_clean =
            new TH2D(Form("u_hists_clean_%d", entry_nr),
                     Form("Clean histogram for U entry %d", entry_nr), 512, 1,
                     513, 100, 1, 101);
        v_hists_clean =
            new TH2D(Form("v_hists_clean_%d", entry_nr),
                     Form("Clean histogram for V entry %d", entry_nr), 512, 1,
                     513, 100, 1, 101);
        w_hists_clean =
            new TH2D(Form("w_hists_clean_%d", entry_nr),
                     Form("Clean histogram for W entry %d", entry_nr), 512, 1,
                     513, 100, 1, 101);

        // Set the color scale

        gStyle->SetPalette(kGreyScale);

        u_hists->SetMinimum(-100);
        u_hists->SetMaximum(1000);

        v_hists->SetMinimum(-100);
        v_hists->SetMaximum(1000);

        w_hists->SetMinimum(-100);
        w_hists->SetMaximum(1000);

        u_hists_clean->SetMinimum(-100);
        u_hists_clean->SetMaximum(1000);

        v_hists_clean->SetMinimum(-100);
        v_hists_clean->SetMaximum(1000);

        w_hists_clean->SetMinimum(-100);
        w_hists_clean->SetMaximum(1000);

        // Fill the hists

        int bin = 0;

        for (const auto &iter : data_container.uvw_data) {

            bin = 0;

            if (iter.plane_val == 0) {

                for (const auto &sig_iter : iter.signal_val) {

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 1) {

                for (const auto &sig_iter : iter.signal_val) {

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 2) {

                for (const auto &sig_iter : iter.signal_val) {

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }
        }

        bin = 0;

        for (const auto &iter : data_container_clean.uvw_data) {

            bin = 0;

            if (iter.plane_val == 0) {

                for (const auto &sig_iter : iter.signal_val) {

                    u_hists_clean->SetBinContent(++bin, iter.strip_nr,
                                                 sig_iter);
                }
            }

            if (iter.plane_val == 1) {

                for (const auto &sig_iter : iter.signal_val) {

                    v_hists_clean->SetBinContent(++bin, iter.strip_nr,
                                                 sig_iter);
                }
            }

            if (iter.plane_val == 2) {

                for (const auto &sig_iter : iter.signal_val) {

                    w_hists_clean->SetBinContent(++bin, iter.strip_nr,
                                                 sig_iter);
                }
            }
        }

        // Draw the hists.

        loc_pad->cd(1);
        u_hists->Draw("CONT4Z");
        loc_pad->cd(2);
        v_hists->Draw("CONT4Z");
        loc_pad->cd(3);
        w_hists->Draw("CONT4Z");   // CONT4Z or COLZ

        loc_pad->cd(4);
        u_hists_clean->Draw("CONT4Z");
        loc_pad->cd(5);
        v_hists_clean->Draw("CONT4Z");
        loc_pad->cd(6);
        w_hists_clean->Draw("CONT4Z");

        loc_canv->Update();
    }

    loc_canv->WaitPrimitive();
}

void runmacro2(TString lin_arg) {

    viewUVWdata_mini("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
                     "CoBo_2018-06-20T10-51-39.459_0000.root");
}