#include <cstdio>
#include <glob.h>
#include <iostream>
#include <limits>
#include <map>
#include <random>

#include "TString.h"
#include "include/ErrorCodesMap.hpp"
#include "src/cleanUVW.cpp"
#include "src/convertUVW_elitpc.cpp"
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

void viewUVWdata_elitpc(TString fileName) {

    int entry_nr = -1;
    int max_entries;

    auto goodFile = fileName;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    max_entries = good_data.returnNEntries();

    std::cout << "Number of entries is " << max_entries << "\n";

    convertUVW_elitpc loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0) {

        std::cerr << "Error at openSpecFile code: " << err << std::endl;

        return;
    }

    cleanUVW loc_clean_uvw;

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

        err = loc_conv_uvw.makeConversion(false, true);
        if (err != 0)
            std::cout << "Make conversion error code " << err << std::endl;

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << std::endl;

        int count_u = 0;
        int count_v = 0;
        int count_w = 0;
        for (const auto &data_entry : data_container.uvw_data) {

            if (data_entry.plane_val == 0) {
                count_u++;
            } else if (data_entry.plane_val == 1) {
                count_v++;
            } else if (data_entry.plane_val == 2) {
                count_w++;
            }
        }

        std::cout << count_u << "\n" << count_v << "\n" << count_w << "\n";
    }
}

// root -q "runmacro_elitpc.cpp(\"a\")"
void runmacro_elitpc(TString lin_arg) {

    viewUVWdata_elitpc("/media/gant/Expansion/elitpcraw/"
                       "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root");

    /* mass_create_raw_images_elitpc(
        "/media/gant/Expansion/elitpcraw/"
        "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"); */
}