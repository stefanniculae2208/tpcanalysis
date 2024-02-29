#include <cstdio>
#include <glob.h>
#include <iostream>
#include <limits>
#include <map>
#include <random>

#include "../src/cleanUVW.cpp"
#include "../src/convertUVW_elitpc.cpp"
#include "../src/loadData.cpp"
#include "TString.h"

#include "../include/generalDataStorage.hpp"

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

#include "../dict/src/GDataChannel.cpp"
#include "../dict/src/GDataFrame.cpp"
#include "../dict/src/GDataSample.cpp"
#include "../dict/src/GFrameHeader.cpp"

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

/**
 * @brief Creates the .csv file used to normalize the channels.
 * The file and channels used have been pre selected.
 *
 */
void createNormCSV_elitpc(bool lin_arg = true) {

    const bool opt_fpn = lin_arg;

    TString filename = "/media/gant/Expansion/elitpcraw/"
                       "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root";

    auto goodTree = "tree";

    loadData good_data(filename, goodTree);

    auto err = good_data.openFile();

    err = good_data.readData();

    auto max_entries = good_data.returnNEntries();

    convertUVW_elitpc loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    generalDataStorage data_container;

    // we always use 20 entries
    std::array<std::array<double, 20>, 3> entry_mean = {{}};

    std::array<double, 3> total_mean = {};

    // Entries have been manually selected and should be mostly noise.
    std::array<int, 20> entry_nrs = {3,  4,  7,  9,  10, 14, 16, 17, 20, 21,
                                     22, 23, 24, 25, 26, 27, 28, 31, 32, 33};

    std::vector<generalDataStorage> data_vec;

    try {
        data_vec = funcUtil::getAllEntries(filename, true, false);
    } catch (const std::exception &e) {

        std::cout << "Encountered error when getting all entries." << std::endl;
        return;
    }

    data_vec = funcUtil::groupEntriesByEvent(std::move(data_vec));

    std::cout << "New data size is " << data_vec.size() << "\n\n";

    int nr_events = data_vec.size();

    for (auto i = 0; i < 20; i++) {

        data_container = data_vec[entry_nrs[i]];

        int count_u = 0;
        int count_v = 0;
        int count_w = 0;

        for (const auto &uvw_entry : data_container.uvw_data) {

            if (uvw_entry.plane_val == 0 && (uvw_entry.strip_section == 0 ||
                                             uvw_entry.strip_section == 1)) {

                entry_mean[0][i] +=
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                count_u++;

            } else if (uvw_entry.plane_val == 1 &&
                       (uvw_entry.strip_section == 0 ||
                        uvw_entry.strip_section == 1)) {

                entry_mean[1][i] +=
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                count_v++;

            } else if (uvw_entry.plane_val == 2 &&
                       (uvw_entry.strip_section == 0 ||
                        uvw_entry.strip_section == 1)) {

                entry_mean[2][i] +=
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                count_w++;
            }
        }

        entry_mean[0][i] /= count_u;
        entry_mean[1][i] /= count_v;
        entry_mean[2][i] /= count_w;
    }

    total_mean[0] =
        std::accumulate(entry_mean[0].begin(), entry_mean[0].end(), 0.0) /
        entry_mean[0].size();
    total_mean[1] =
        std::accumulate(entry_mean[1].begin(), entry_mean[1].end(), 0.0) /
        entry_mean[1].size();
    total_mean[2] =
        std::accumulate(entry_mean[2].begin(), entry_mean[2].end(), 0.0) /
        entry_mean[2].size();

    std::array<std::array<std::array<double, 20>, 227>, 3> entry_ch_ratio = {
        {{}}};

    std::array<std::array<double, 227>, 3> total_ch_ratio = {{}};

    for (auto i = 0; i < 20; i++) {

        auto err = good_data.decodeData(entry_nrs.at(i), opt_fpn);

        data_container.root_raw_data = good_data.returnRawData();

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion(false);

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        // For now only take the first strip section until I know what to do
        // with the other.

        for (const auto &uvw_entry : data_container.uvw_data) {

            if (uvw_entry.plane_val == 0 && (uvw_entry.strip_section == 0 ||
                                             uvw_entry.strip_section == 1)) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[0][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[0];

            } else if (uvw_entry.plane_val == 1 &&
                       (uvw_entry.strip_section == 0 ||
                        uvw_entry.strip_section == 1)) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[1][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[1];

            } else if (uvw_entry.plane_val == 2 &&
                       (uvw_entry.strip_section == 0 ||
                        uvw_entry.strip_section == 1)) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[2][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[2];
            }
        }
    }

    std::ofstream out_file("./utils/ch_norm_ratios_elitpc.csv");

    out_file << "plane,strip,ratio\n";

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 227; j++) {
            double sum = std::accumulate(entry_ch_ratio[i][j].begin(),
                                         entry_ch_ratio[i][j].end(), 0.0);
            total_ch_ratio[i][j] = sum / entry_ch_ratio[i][j].size();

            out_file << i << "," << j << "," << total_ch_ratio[i][j] << "\n";
        }
    }

    out_file.close();
}