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

namespace funcUtil {

/// @brief
/// @param fileName
/// @param opt_verbose
/// @return
std::vector<generalDataStorage> getAllEntries(TString fileName,
                                              bool opt_verbose = true,
                                              bool opt_norm = false) {

    int nr_entries = -1;

    auto goodFile = fileName;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    nr_entries = good_data.returnNEntries();

    if (opt_verbose) {

        std::cout << "Number of entries is " << nr_entries << "\n\n";
    }

    convertUVW_elitpc loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0) {

        std::cerr << "Error at openSpecFile code: " << err << std::endl;

        throw std::runtime_error("Could not open the spec file.\n");
    }

    uint32_t entry_nr = 0;

    std::vector<generalDataStorage> data_vec;

    // Save some time by reserving before.
    data_vec.reserve(nr_entries);

    while (entry_nr < nr_entries) {

        generalDataStorage data_container;

        err = good_data.decodeData(entry_nr, true);
        if (err != 0) {

            std::cout << "Error decode data code " << err << " at entry "
                      << entry_nr << err << std::endl;

            throw std::runtime_error("Could not decode the data.\n");
        }

        data_container.root_raw_data = good_data.returnRawData();

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion(opt_norm, false);
        if (err != 0) {

            std::cout << "Make conversion error code " << err << " at entry "
                      << entry_nr << std::endl;

            throw std::runtime_error("Could not make the conversion.\n");
        }

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        data_container.event_id = data_container.uvw_data[0].event_id;

        //###TEMPORARY###
        //###############

        for (auto &data_el : data_container.uvw_data) {

            std::fill(data_el.signal_val.begin(),
                      data_el.signal_val.begin() + 10, 0);

            std::fill(data_el.signal_val.begin() + 500,
                      data_el.signal_val.begin() + 512, 0);
        }

        //###############
        //###############

        // Delete useless data to save memory.
        std::vector<rawData>().swap(data_container.root_raw_data);

        data_vec.push_back(data_container);

        if (opt_verbose && (entry_nr % (nr_entries / 10) == 0)) {
            std::cout << "At entry " << entry_nr << "\n";
        }

        entry_nr++;
    }

    if (opt_verbose) {

        std::cout << "\nThe data vector has size: " << data_vec.size()
                  << std::endl;
    }

    return data_vec;
}

/// @brief
/// @param entries_vec
std::vector<generalDataStorage>
groupEntriesByEvent(std::vector<generalDataStorage> &&entries_vec) {

    std::multimap<int, std::vector<dataUVW>> event_entry_group;

    for (auto &&entry : entries_vec) {

        event_entry_group.insert({entry.event_id, std::move(entry.uvw_data)});
    }

    // Free as much memory as possible to avoid crashes.
    std::vector<generalDataStorage>().swap(entries_vec);

    std::vector<generalDataStorage>
        result;   // Result vector to store merged data

    generalDataStorage loc_data;

    int prev_id = -1;
    for (auto &event : event_entry_group) {

        if (event.first != prev_id) {

            if (prev_id != -1) {

                result.push_back(std::move(loc_data));
            }
            loc_data.uvw_data.clear();

            loc_data.event_id = event.first;
        }

        loc_data.uvw_data.insert(loc_data.uvw_data.end(),
                                 std::make_move_iterator(event.second.begin()),
                                 std::make_move_iterator(event.second.end()));

        prev_id = event.first;
    }

    return result;
}

};   // namespace funcUtil

/// @brief
/// @param fileName
void viewUVWdata_elitpc(TString fileName) {

    std::vector<generalDataStorage> data_vec;

    try {
        data_vec = funcUtil::getAllEntries(fileName, true, true);
    } catch (const std::exception &e) {

        std::cout << "Encountered error when getting all entries." << std::endl;
        return;
    }

    data_vec = funcUtil::groupEntriesByEvent(std::move(data_vec));

    std::cout << "New data size is " << data_vec.size() << "\n\n";

    int nr_events = data_vec.size();

    auto loc_canv =
        new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();

    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;
    TH1D *c_hist = nullptr;

    int event_nr = -1;

    uint32_t loop_nr = 0;

    while (1) {

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

            if (event_nr < nr_events) {
                event_nr++;
            }

            std::cout << "Entry number is " << event_nr << "\n\n" << std::endl;

        } else if (key_val == 'a') {

            if (event_nr > 0) {
                event_nr--;
            }

            std::cout << "Entry number is " << event_nr << "\n\n" << std::endl;

        } else if (key_val == 'i') {

            std::cout << "Please enter the entry number.\n";

            std::cin >> event_nr;

            if (event_nr < 0 || event_nr > (nr_events - 1))
                event_nr = 0;

            std::cout << "Entry number is " << event_nr << "\n\n" << std::endl;
        }

        if (event_nr < 0) {
            event_nr = 0;
        }

        // Reset hists

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        if (c_hist != nullptr)
            c_hist->Reset();

        // Create new hists

        u_hists = new TH2D(Form("u_hists_%d", event_nr),
                           Form("Histogram for U entry %d", event_nr), 512, 1,
                           513, 250, 1, 251);
        v_hists = new TH2D(Form("v_hists_%d", event_nr),
                           Form("Histogram for V entry %d", event_nr), 512, 1,
                           513, 250, 1, 251);
        w_hists = new TH2D(Form("w_hists_%d", event_nr),
                           Form("Histogram for W entry %d", event_nr), 512, 1,
                           513, 250, 1, 251);

        // Fill the hists

        int bin = 0;

        std::array<double, 512> ch_array = {0};

        std::cout << "\n\nViewing event with Id: "
                  << data_vec[event_nr].event_id << " and size "
                  << data_vec[event_nr].uvw_data.size() << "\n\n";

        for (const auto &iter : data_vec[event_nr].uvw_data) {

            // All signals should have size 512;
            if (iter.signal_val.size() == 512) {

                for (auto i = 0; i < 512; ++i) {

                    ch_array[i] += iter.signal_val[i];
                }
            }

            bin = 0;

            if (iter.plane_val == 0 &&
                (iter.strip_section == 0 || iter.strip_section == 1)) {

                for (const auto &sig_iter : iter.signal_val) {

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 1 &&
                (iter.strip_section == 0 || iter.strip_section == 1)) {

                for (const auto &sig_iter : iter.signal_val) {

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 2 &&
                (iter.strip_section == 0 || iter.strip_section == 1)) {

                for (const auto &sig_iter : iter.signal_val) {

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }
        }

        // Draw

        loc_pad->cd(1);
        u_hists->Draw("COLZ");
        loc_pad->cd(2);
        v_hists->Draw("COLZ");
        loc_pad->cd(3);
        w_hists->Draw("COLZ");

        loc_canv->Update();
    }

    loc_canv->WaitPrimitive();
}

void testConvertUVW(TString fileName) {
    int nr_entries = -1;

    auto goodFile = fileName;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    nr_entries = good_data.returnNEntries();

    std::cout << "Number of entries is " << nr_entries << "\n\n";

    convertUVW_elitpc loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0) {

        std::cerr << "Error at openSpecFile code: " << err << std::endl;

        throw std::runtime_error("Could not open the spec file.\n");
    }

    int entry_nr = -1;

    uint32_t loop_nr = 0;

    while (1) {

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

            if (entry_nr < nr_entries) {
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

            if (entry_nr < 0 || entry_nr > (nr_entries - 1))
                entry_nr = 0;

            std::cout << "Entry number is " << entry_nr << "\n\n" << std::endl;
        }

        if (entry_nr < 0) {
            entry_nr = 0;
        }

        generalDataStorage data_container;

        err = good_data.decodeData(entry_nr, false);
        if (err != 0) {

            std::cout << "Error decode data code " << err << " at entry "
                      << entry_nr << err << std::endl;

            throw std::runtime_error("Could not decode the data.\n");
        }

        data_container.root_raw_data = good_data.returnRawData();

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion(false, true);
        if (err != 0) {

            std::cout << "Make conversion error code " << err << " at entry "
                      << entry_nr << std::endl;

            throw std::runtime_error("Could not make the conversion.\n");
        }

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW size: " << data_container.uvw_data.size()
                  << std::endl;
    }
}

// root -q "runmacro_elitpc.cpp(\"a\")"
void runmacro_elitpc(TString lin_arg) {

    viewUVWdata_elitpc("/media/gant/Expansion/elitpcraw/"
                       "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root");

    /* testConvertUVW("/media/gant/Expansion/elitpcraw/"
                   "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"); */

    /* mass_create_raw_images_elitpc(
        "/media/gant/Expansion/elitpcraw/"
        "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"); */
}