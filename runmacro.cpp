#include <cstdio>
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

/**
 * @brief Creates the .csv file used to normalize the channels.
 *
 */
void createNormCSV() {

    TString filename = "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
                       "CoBo_2018-06-20T10-35-30.853_0005.root";

    auto goodTree = "tree";

    loadData good_data(filename, goodTree);

    auto err = good_data.openFile();

    err = good_data.readData();

    auto max_entries = good_data.returnNEntries();

    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    generalDataStorage data_container;

    // we always use 20 entries
    std::array<std::array<double, 20>, 3> entry_mean = {{}};

    std::array<double, 3> total_mean = {};

    std::array<int, 20> entry_nrs = {7,  15,  22,  36,  37,  40, 49,
                                     52, 53,  55,  56,  61,  65, 68,
                                     78, 102, 103, 143, 152, 161};

    for (auto i = 0; i < 20; i++) {

        auto err = good_data.decodeData(entry_nrs.at(i));
        if (err != 0) {
            std::cout << "Error decode data code " << err << std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << std::endl;

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << std::endl;

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        int count_u = 0;
        int count_v = 0;
        int count_w = 0;

        for (const auto &uvw_entry : data_container.uvw_data) {

            if (uvw_entry.plane_val == 0) {

                entry_mean[0][i] +=
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                count_u++;

            } else if (uvw_entry.plane_val == 1) {

                entry_mean[1][i] +=
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                count_v++;

            } else if (uvw_entry.plane_val == 2) {

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

    std::array<std::array<std::array<double, 20>, 93>, 3> entry_ch_ratio = {
        {{}}};

    std::array<std::array<double, 93>, 3> total_ch_ratio = {{}};

    for (auto i = 0; i < 20; i++) {

        auto err = good_data.decodeData(entry_nrs.at(i));

        data_container.root_raw_data = good_data.returnRawData();

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        for (const auto &uvw_entry : data_container.uvw_data) {

            if (uvw_entry.plane_val == 0) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[0][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[0];

            } else if (uvw_entry.plane_val == 1) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[1][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[1];

            } else if (uvw_entry.plane_val == 2) {

                double loc_mean =
                    std::accumulate(uvw_entry.signal_val.begin(),
                                    uvw_entry.signal_val.end(), 0.0) /
                    uvw_entry.signal_val.size();

                entry_ch_ratio[2][uvw_entry.strip_nr][i] =
                    loc_mean / total_mean[2];
            }
        }
    }

    std::ofstream out_file("./utils/ch_norm_ratios.csv");

    out_file << "plane,strip,ratio\n";

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 93; j++) {
            double sum = std::accumulate(entry_ch_ratio[i][j].begin(),
                                         entry_ch_ratio[i][j].end(), 0.0);
            total_ch_ratio[i][j] = sum / entry_ch_ratio[i][j].size();

            out_file << i << "," << j << "," << total_ch_ratio[i][j] << "\n";
        }
    }

    out_file.close();
}

/**
 * @brief Tools used to reorder the channels to the correct order.
 *
 */
namespace reorderCh {
/**
 * @brief Contains constants used for the U plane.
 *
 */
struct planeInfo_U {

    static const int plane_nr = 0;
    static const int plane_size = 72;
    static const int cfg_size = 2;
    static const int right_angle = 1;
    inline static const std::string plane_file = "./utils/channelorder_u.txt";
    inline static const std::string cfg_file = "./utils/cfg_entries_u.txt";
};

/**
 * @brief Contains constants used for the V plane.
 *
 */
struct planeInfo_V {

    static const int plane_nr = 1;
    static const int plane_size = 92;
    static const int cfg_size = 1;
    static const int right_angle = 0;
    inline static const std::string plane_file = "./utils/channelorder_v.txt";
    inline static const std::string cfg_file = "./utils/cfg_entries_v.txt";
};

/**
 * @brief Contains constants used for the W plane.
 *
 */
struct planeInfo_W {

    static const int plane_nr = 2;
    static const int plane_size = 92;
    static const int cfg_size = 1;
    static const int right_angle = 1;
    inline static const std::string plane_file = "./utils/channelorder_w.txt";
    inline static const std::string cfg_file = "./utils/cfg_entries_w.txt";
};

/**
 * @brief Uses the channelorder_*.txt files from ./utils/ to reorder the
 * channels to the correct order. Works for 1 plane at a time.
 *
 * @tparam pI One of the planeInfo_U, planeInfo_V or planeInfo_W structs,
 * depending on the plane to be reordered.
 * @param uvw_vec The vector to be reordered.
 */
template <typename pI> void reorderChfromFile(std::vector<dataUVW> &uvw_vec) {

    std::ifstream file(pI::plane_file.c_str());

    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << pI::plane_file
                  << std::endl;
        return;
    }

    int index = 1;

    std::string line;

    std::map<int, int> ch_map;

    while (std::getline(file, line)) {

        if (ch_map.count(std::stoi(line)) == 0) {
            ch_map.insert({std::stoi(line), index});
        } else {
            std::cerr
                << "Error: key already exists in map at reorderChfromFile."
                << std::endl;
        }

        ++index;
    }

    for (auto &uvw_data : uvw_vec) {

        if (uvw_data.plane_val == pI::plane_nr)
            uvw_data.strip_nr = ch_map.at(uvw_data.strip_nr);
    }

    file.close();
}

/**
 * @brief Calculates the order in which the channels need to be reordered. Only
 * use this if you want to find a better order. The file used is taken as an
 * argument and the entries used are taken from ./utils/cfg_entries_*.txt. The
 * number of entries need to be changed in the planeInfo_* structs.
 *
 * @tparam pI One of the planeInfo_U, planeInfo_V or planeInfo_W structs,
 * depending on the plane to be reordered.
 * @param fileName The .root file used for the reorder.
 */
template <typename pI> void calculateReorder(TString fileName) {

    auto goodFile = fileName;

    auto goodTree = "tree";

    generalDataStorage data_container;

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    auto max_entries = good_data.returnNEntries();

    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    cleanUVW loc_clean_uvw;

    std::map<int, std::array<int, pI::cfg_size>> strip_order_map;

    std::ifstream file(pI::cfg_file.c_str());

    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << pI::cfg_file << std::endl;
        return;
    }

    int index = 0;

    std::string line;

    while (std::getline(file, line)) {

        int entry_nr = std::stoi(line);

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();

        // use norm_opt to set normalization on or off
        err = loc_conv_uvw.normalizeChannels();
        if (err != 0)
            std::cout << "Normalize channels error code " << err << std::endl;

        /* err = loc_conv_uvw.substractBl();
        if (err != 0)
            std::cout << "Substractbl error code " << err << std::endl; */

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        // std::cout<<"UVW data size is
        // "<<data_container.uvw_data.size()<<std::endl;

        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        data_container.uvw_data = loc_clean_uvw.returnDataUVW();

        // Order by plane and strip
        std::sort(data_container.uvw_data.begin(),
                  data_container.uvw_data.end(),
                  [](const dataUVW &a, const dataUVW &b) {
                      return 1e6 * a.plane_val + a.strip_nr <
                             1e6 * b.plane_val + b.strip_nr;
                  });

        std::array<std::array<std::size_t, 93>, 3> max_index;

        for (const auto &uvw_el : data_container.uvw_data) {

            auto max_it = std::max_element(uvw_el.signal_val.begin(),
                                           uvw_el.signal_val.end());
            max_index[uvw_el.plane_val][uvw_el.strip_nr] =
                std::distance(uvw_el.signal_val.begin(), max_it);
        }

        if (pI::right_angle) {
            std::sort(data_container.uvw_data.begin(),
                      data_container.uvw_data.end(),
                      [max_index](const dataUVW &a, const dataUVW &b) {
                          return max_index[a.plane_val][a.strip_nr] <
                                 max_index[b.plane_val][b.strip_nr];
                      });
        } else {
            std::sort(data_container.uvw_data.begin(),
                      data_container.uvw_data.end(),
                      [max_index](const dataUVW &a, const dataUVW &b) {
                          return max_index[a.plane_val][a.strip_nr] >
                                 max_index[b.plane_val][b.strip_nr];
                      });
        }

        int val_counter = 0;

        for (auto i = 0; i < data_container.uvw_data.size(); i++) {

            if (data_container.uvw_data.at(i).plane_val == pI::plane_nr) {

                ++val_counter;

                strip_order_map[data_container.uvw_data.at(i).strip_nr][index] =
                    val_counter;
            }
        }

        ++index;
    }

    file.close();

    std::map<int, int> strip_order;

    for (const auto &map_entry : strip_order_map) {

        auto &strip_values = map_entry.second;

        // Create a map to count the frequency of each value.
        std::unordered_map<int, int> frequency_map;
        for (int value : strip_values) {
            frequency_map[value]++;
        }

        std::vector<std::pair<int, int>> frequency_pairs(frequency_map.begin(),
                                                         frequency_map.end());
        std::sort(
            frequency_pairs.begin(), frequency_pairs.end(),
            [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                return a.second > b.second;
            });

        int most_frequent_value = 0;
        for (const auto &pair : frequency_pairs) {
            if (strip_order.count(pair.first) == 0) {
                most_frequent_value = pair.first;
                break;
            }
        }

        if (strip_order.count(most_frequent_value) == 0) {
            strip_order.insert({most_frequent_value, map_entry.first});
        } else {
            std::cerr << "Error: key " << most_frequent_value
                      << " already exists in map at calculateReorder."
                      << std::endl;
        }
    }

    std::ofstream out_file(pI::plane_file);

    for (const auto &map_entry : strip_order) {

        out_file << map_entry.second << "\n";
    }

    out_file.close();
}

}   // namespace reorderCh

namespace removeBackground {

struct planeInfoU {
    static const int plane_nr = 0;
    static const int plane_size = 72;
    inline static const std::string plane_hist_name = "Charge_hist_plane_u";
};

struct planeInfoV {
    static const int plane_nr = 1;
    static const int plane_size = 92;
    inline static const std::string plane_hist_name = "Charge_hist_plane_v";
};

struct planeInfoW {
    static const int plane_nr = 2;
    static const int plane_size = 92;
    inline static const std::string plane_hist_name = "Charge_hist_plane_w";
};

double calculateBaseline(const std::array<double, 512> &charge_val) {

    double baseline = std::numeric_limits<double>::max();

    // Calculate the baseline as the smallest non 0 element.

    auto start_iter = std::next(charge_val.begin(), 5);
    baseline =
        *std::min_element(start_iter, charge_val.end(), [](int a, int b) {
            return (a > 0 && b > 0) ? (a < b) : (a > b);
        });

    return baseline;
}

template <typename pI>
void calculateChargeHist(std::vector<dataUVW> &uvw_vec, TH1D *&charge_hist,
                         const int extr_opt = 0) {

    std::array<double, 512> charge_val;

    std::vector<dataUVW> filtered_data;
    std::copy_if(
        uvw_vec.begin(), uvw_vec.end(), std::back_inserter(filtered_data),
        [](const dataUVW &data) { return data.plane_val == pI::plane_nr; });

    charge_val.fill(0);

    for (std::size_t i = 0; i < charge_val.size(); i++) {
        charge_val.at(i) =
            std::accumulate(filtered_data.begin(), filtered_data.end(), 0.0,
                            [i](double acc, const dataUVW &el) {
                                return acc + el.signal_val.at(i);
                            });
    }

    if (extr_opt) {

        auto baseline = calculateBaseline(charge_val);

        std::transform(charge_val.begin(), charge_val.end(), charge_val.begin(),
                       [baseline](const double &sig_el) {
                           return std::max(0.0, (sig_el - baseline));
                       });

        baseline /= pI::plane_size;

        for (auto &data_el : uvw_vec) {

            data_el.baseline_val += baseline;

            // Extract the baseline from the signal. Make any elements lower
            // than 0 equal to 10 so we don't have negative signal values.
            // I didn't make them 0 because I have a problem with empty vectors.
            std::transform(data_el.signal_val.begin(), data_el.signal_val.end(),
                           data_el.signal_val.begin(),
                           [baseline](const double &sig_el) {
                               return std::max(10.0, (sig_el - baseline));
                           });
        }
    }

    charge_hist =
        new TH1D(pI::plane_hist_name.c_str(), "Charge histogram", 512, 1, 512);

    charge_hist->SetContent(charge_val.data());
}

void viewChargeHist(std::vector<dataUVW> &uvw_vec, const int extr_opt = 0) {

    auto *charge_canv = new TCanvas("Charge_canvas", "Charge_canvas");
    auto loc_pad = new TPad("charge_pad", "Charge pad", 0, 0, 1, 1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();

    TH1D *charge_u;
    TH1D *charge_v;
    TH1D *charge_w;

    calculateChargeHist<planeInfoU>(uvw_vec, charge_u, extr_opt);
    calculateChargeHist<planeInfoV>(uvw_vec, charge_v, extr_opt);
    calculateChargeHist<planeInfoW>(uvw_vec, charge_w, extr_opt);

    loc_pad->cd(1);
    charge_u->Draw();
    loc_pad->cd(2);
    charge_v->Draw();
    loc_pad->cd(3);
    charge_w->Draw();

    charge_canv->Update();
}

}   // namespace removeBackground

/**
 * @brief Allows the viewing of the raw data from the given file.
 *
 * @param fileName The file to viewed.
 * @param norm_opt The normalization option. 1 if you want the channels to be
 * normalized, else 0.
 */
void view_raw_data(TString fileName, int norm_opt = 1) {

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

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    auto loc_canv =
        new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();

    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;

    uint32_t loop_nr = 0;

    while (1) {

        generalDataStorage data_container;

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

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << std::endl;

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << std::endl;

        // use norm_opt to set normalization on or off
        if (norm_opt) {

            err = loc_conv_uvw.normalizeChannels();
            if (err != 0)
                std::cout << "Normalize channels error code " << err
                          << std::endl;
        }

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << std::endl;

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

        int bin = 0;

        for (const auto &iter : data_container.uvw_data) {

            bin = 0;

            if (iter.plane_val == 0) {

                for (auto sig_iter : iter.signal_val) {

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 1) {

                for (auto sig_iter : iter.signal_val) {

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 2) {

                for (auto sig_iter : iter.signal_val) {

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }
        }

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

/**
 * @brief Create a pdf file containing the entries from the .root file.
 *
 * @param source_file The source .root file.
 * @param destination_file The name and loation of the pdf file to be created.
 * @param read_entries The number of entries read.
 * @param norm_opt The normalization option. 1 if you want the channels to be
 * normalized, else 0.
 */
void create_entries_pdf(TString source_file, TString destination_file,
                        int read_entries, int norm_opt = 1) {

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

    if (err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    cleanUVW loc_clean_uvw;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;

    gROOT->SetBatch(kTRUE);

    auto loc_canv = new TCanvas("entries.pdf", "PDF", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
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

    // only read the number of entries wanted
    max_entries = std::min(max_entries, read_entries);

    while (entry_nr < max_entries) {

        generalDataStorage data_container;

        std::cout << "\n\n\nNow at entry: " << entry_nr << " of " << max_entries
                  << " for " << destination_file << "\n";

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << "\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << "\n";

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << "\n";

        // use norm_opt to set normalization on or off
        if (norm_opt) {

            err = loc_conv_uvw.normalizeChannels();
            if (err != 0)
                std::cout << "Normalize channels error code " << err
                          << std::endl;
        }

        /* err = loc_conv_uvw.substractBl();

        if (err != 0)
            std::cout << "Substractbl error code " << err << "\n"; */

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << "\n";

        reorderCh::reorderChfromFile<reorderCh::planeInfo_U>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_V>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_W>(
            data_container.uvw_data);

        loc_clean_uvw.setUVWData(data_container.uvw_data);

        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        data_container.uvw_data = loc_clean_uvw.returnDataUVW();

        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if (err != 0)
            std::cout << "Error set UVW data code " << err << "\n";

        err = loc_convert_hit.getHitInfo();
        if (err != 0)
            std::cout << "Error get hit info code " << err << "\n";

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout << "Hit data size " << data_container.hit_data.size() << "\n";
        std::cout << "Hist data size "
                  << data_container.raw_hist_container.size() << "\n";

        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if (err != 0)
            std::cout << "Error get new vector code " << err << "\n";

        err = loc_conv_xyz.makeConversionXYZ();
        if (err != 0)
            std::cout << "Error make conversion XYZ code " << err << "\n";

        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout << "XYZ vector size " << data_container.xyz_data.size()
                  << "\n";

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        if (u_graph != nullptr)
            u_graph->Delete();

        if (v_graph != nullptr)
            v_graph->Delete();

        if (w_graph != nullptr)
            w_graph->Delete();

        if (p_graph != nullptr)
            p_graph->Delete();

        if (p_graph3d != nullptr)
            p_graph3d->Delete();

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

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

        loc_pad->cd(1);
        u_hists->Draw("COLZ");
        loc_pad->cd(2);
        v_hists->Draw("COLZ");
        loc_pad->cd(3);
        w_hists->Draw("COLZ");

        std::vector<double> x_u;
        std::vector<double> y_u;
        std::vector<double> c_u;

        std::vector<double> x_v;
        std::vector<double> y_v;
        std::vector<double> c_v;

        std::vector<double> x_w;
        std::vector<double> y_w;
        std::vector<double> c_w;

        for (const auto &hit_iter : data_container.hit_data) {

            if (hit_iter.plane == 0) {

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);
                c_u.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 1) {

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);
                c_v.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 2) {

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);
                c_w.push_back(hit_iter.peak_y + hit_iter.base_line);
            }
        }

        if (data_container.hit_data.size() != 0) {

            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            // u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            // Set marker colors Pink > Red > Green > Blue
            if (c_u.size() != 0) {

                auto min_val_c_u = 0;
                auto max_val_c_u = 2000;

                std::transform(c_u.begin(), c_u.end(), c_u.begin(),
                               [min_val_c_u, max_val_c_u](const double x) {
                                   return (x - min_val_c_u) /
                                          (max_val_c_u - min_val_c_u);
                               });

                for (auto vec_i = 0; vec_i < c_u.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    u_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_u.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_u.at(vec_i));
                    } else if (c_u.at(vec_i) >= 0.33 && c_u.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_u.at(vec_i) / 2, 0);
                    } else if (c_u.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_u.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    u_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(4);
            u_graph->Draw("AP");

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            // v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            if (c_v.size() != 0) {

                auto min_val_c_v = 0;
                auto max_val_c_v = 2000;

                std::transform(c_v.begin(), c_v.end(), c_v.begin(),
                               [min_val_c_v, max_val_c_v](const double x) {
                                   return (x - min_val_c_v) /
                                          (max_val_c_v - min_val_c_v);
                               });

                for (auto vec_i = 0; vec_i < c_v.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    v_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_v.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_v.at(vec_i));
                    } else if (c_v.at(vec_i) >= 0.33 && c_v.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_v.at(vec_i) / 2, 0);
                    } else if (c_v.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_v.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    v_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(5);
            v_graph->Draw("AP");

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            // w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            if (c_w.size() != 0) {

                auto min_val_c_w = 0;
                auto max_val_c_w = 2000;

                std::transform(c_w.begin(), c_w.end(), c_w.begin(),
                               [min_val_c_w, max_val_c_w](const double x) {
                                   return (x - min_val_c_w) /
                                          (max_val_c_w - min_val_c_w);
                               });

                for (auto vec_i = 0; vec_i < c_w.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    w_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_w.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_w.at(vec_i));
                    } else if (c_w.at(vec_i) >= 0.33 && c_w.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_w.at(vec_i) / 2, 0);
                    } else if (c_w.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_w.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    w_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(6);
            w_graph->Draw("AP");

            loc_pad->Modified();
            loc_pad->Update();

        } else {
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }

        if (data_container.xyz_data.size() != 0) {

            std::vector<double> x, y, z, chg;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
            chg.push_back(0.0);

            for (const auto &point_iter : data_container.xyz_data) {

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
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle("XY coordinates projection; X axis; Y axis");
            if (chg.size() != 0) {

                auto min_val_chg = 0;
                auto max_val_chg = 6000;

                std::transform(chg.begin(), chg.end(), chg.begin(),
                               [min_val_chg, max_val_chg](const double x) {
                                   return (x - min_val_chg) /
                                          (max_val_chg - min_val_chg);
                               });

                for (auto vec_i = 0; vec_i < chg.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    p_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (chg.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * chg.at(vec_i));
                    } else if (chg.at(vec_i) >= 0.33 && chg.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * chg.at(vec_i) / 2, 0);
                    } else if (chg.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(chg.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    p_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(7);
            p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X "
                                "axis; Y axis; Z axis");
            loc_pad->cd(8);
            p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();

        } else {
            p_graph = nullptr;
            p_graph3d = nullptr;
        }

        entry_nr++;

        loc_pad->Update();
        loc_canv->Update();
        loc_canv->Print(destination_file);
    }

    gErrorIgnoreLevel = kPrint;
    loc_canv->Print(destination_file + "]");

    loc_canv->Close();

    if (loc_canv) {
        delete (loc_canv);
        loc_canv = nullptr;
    }

    gROOT->SetBatch(kFALSE);

    std::cout << "\n\n\nDONE!!!!!\n\n\n" << std::endl;
}

/**
 * @brief Create a pdf file with the raw data entries/
 *
 * @param source_file The source .root file.
 * @param destination_file The name and loation of the pdf file to be created.
 * @param read_entries The number of entries read.
 */
void create_raw_pdf(TString source_file, TString destination_file,
                    int read_entries) {

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

    if (err != 0)
        return;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;

    gROOT->SetBatch(kTRUE);

    auto loc_canv = new TCanvas("entries.pdf", "PDF", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
    loc_pad->Divide(3, 1);
    loc_pad->Draw();

    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;

    loc_canv->Print(destination_file + "[");
    gErrorIgnoreLevel = kWarning;

    // only read the number of entries wanted
    max_entries = std::min(max_entries, read_entries);

    while (entry_nr < max_entries) {

        generalDataStorage data_container;

        std::cout << "\n\n\nNow at entry: " << entry_nr << " of " << max_entries
                  << " for " << destination_file << "\n";

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << "\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << "\n";

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << "\n";

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << "\n";

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

        int bin = 0;
        // int strip = 1;

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

        loc_pad->cd(1);
        u_hists->Draw("COLZ");
        loc_pad->cd(2);
        v_hists->Draw("COLZ");
        loc_pad->cd(3);
        w_hists->Draw("COLZ");

        entry_nr++;

        loc_pad->Update();
        loc_canv->Update();
        loc_canv->Print(destination_file);
    }

    gErrorIgnoreLevel = kPrint;
    loc_canv->Print(destination_file + "]");

    loc_canv->Close();

    if (loc_canv) {
        delete (loc_canv);
        loc_canv = nullptr;
    }

    gROOT->SetBatch(kFALSE);

    std::cout << "\n\n\nDONE!!!!!\n\n\n" << std::endl;
}

/**
 * @brief Writes the calculated XYZ values from the given entry from the given
 * file to a .csv file.
 *
 * @param filename The name of the .root file.
 * @param entry_nr The entry number to be converted.
 */
void writeXYZcvs(TString filename = "./rootdata/data2.root",
                 int entry_nr = 429) {

    generalDataStorage data_container;

    auto goodFile = filename;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_conv_uvw(data_container.root_raw_data);

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if (err != 0)
        std::cout << "Make conversion error code " << err << std::endl;

    /* err = loc_conv_uvw.substractBl();

    if (err != 0)
        std::cout << "Substractbl error code " << err << std::endl; */

    /* err = loc_conv_uvw.drawChargeHist();

    if (err != 0)
        std::cout << "Draw Charge Hist error code " << err << std::endl; */

    data_container.uvw_data = loc_conv_uvw.returnDataUVW();

    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if (err != 0)
        std::cout << "Error get hit info code " << err << std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout << "Hit data size " << data_container.hit_data.size()
              << std::endl;
    std::cout << "Hist data size " << data_container.raw_hist_container.size()
              << std::endl;

    convertXYZ loc_conv_xyz(data_container.hit_data);

    err = loc_conv_xyz.makeConversionXYZ();

    data_container.xyz_data = loc_conv_xyz.returnXYZ();

    std::ofstream out_file("./converteddata/test_xyz.csv");

    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open file for output" << std::endl;
    }

    // header
    out_file << "x,y,z\n";

    for (const auto &data_entry : data_container.xyz_data) {

        out_file << data_entry.data_x << "," << data_entry.data_y << ","
                 << data_entry.data_z;

        out_file << "\n";
    }

    out_file.close();
}

/**
 * @brief Converts all of the entries to XYZ and writes them in a .csv file.
 *
 * @param filename The name of the .root file.
 */
void writeFullXYZCSV(TString filename = "./rootdata/data2.root") {

    auto goodFile = filename;

    auto goodTree = "tree";

    std::ofstream out_file("./converteddata/data2allentries.csv");

    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open file for output" << std::endl;
        return;
    }

    loadData good_data(goodFile, goodTree);

    convertUVW loc_conv_uvw;

    cleanUVW loc_clean_uvw;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;

    auto err = good_data.openFile();
    err = good_data.readData();

    err = loc_conv_uvw.openSpecFile();

    auto max_entries = good_data.returnNEntries();

    // header
    out_file << "x,y,z,entry_nr\n";

    int entry_nr = 0;

    while (entry_nr < max_entries) {

        std::cout << "\n\n\nNow at entry: " << entry_nr << " of " << max_entries
                  << "\n";

        generalDataStorage data_container;

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << "\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << "\n";

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << "\n";

        /* err = loc_conv_uvw.substractBl();

        if (err != 0)
            std::cout << "Substractbl error code " << err << "\n"; */

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << "\n";

        reorderCh::reorderChfromFile<reorderCh::planeInfo_U>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_V>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_W>(
            data_container.uvw_data);

        loc_clean_uvw.setUVWData(data_container.uvw_data);

        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        data_container.uvw_data = loc_clean_uvw.returnDataUVW();

        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if (err != 0)
            std::cout << "Error set UVW data code " << err << "\n";

        err = loc_convert_hit.getHitInfo();
        if (err != 0)
            std::cout << "Error get hit info code " << err << "\n";

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout << "Hit data size " << data_container.hit_data.size() << "\n";
        std::cout << "Hist data size "
                  << data_container.raw_hist_container.size() << "\n";

        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if (err != 0)
            std::cout << "Error get new vector code " << err << "\n";

        err = loc_conv_xyz.makeConversionXYZ();
        if (err != 0)
            std::cout << "Error make conversion XYZ code " << err << "\n";

        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout << "XYZ vector size " << data_container.xyz_data.size()
                  << "\n";

        for (const auto &data_entry : data_container.xyz_data) {

            out_file << data_entry.data_x << "," << data_entry.data_y << ","
                     << data_entry.data_z << "," << entry_nr;

            out_file << "\n";
        }

        entry_nr++;
    }

    out_file.close();
}

/**
 * @brief Draws an XY image of the entry from the .root file specified.
 *
 * @param filename The name of the .root file.
 * @param entry_nr The entry number to be converted.
 */
void drawXYimage(TString filename = "./rootdata/data2.root",
                 int entry_nr = 429) {

    generalDataStorage data_container;

    auto goodFile = filename;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_conv_uvw(data_container.root_raw_data);

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if (err != 0)
        std::cout << "Make conversion error code " << err << std::endl;

    /* err = loc_conv_uvw.substractBl();

    if (err != 0)
        std::cout << "Substractbl error code " << err << std::endl;

    err = loc_conv_uvw.drawChargeHist();

    if (err != 0)
        std::cout << "Draw Charge Hist error code " << err << std::endl; */

    data_container.uvw_data = loc_conv_uvw.returnDataUVW();

    convertHitData loc_convert_hit(data_container.uvw_data);

    err = loc_convert_hit.getHitInfo();
    if (err != 0)
        std::cout << "Error get hit info code " << err << std::endl;

    data_container.hit_data = loc_convert_hit.returnHitData();
    data_container.raw_hist_container = loc_convert_hit.returnHistData();

    std::cout << "Hit data size " << data_container.hit_data.size()
              << std::endl;
    std::cout << "Hist data size " << data_container.raw_hist_container.size()
              << std::endl;

    convertXYZ loc_conv_xyz(data_container.hit_data);

    err = loc_conv_xyz.makeConversionXYZ();

    data_container.xyz_data = loc_conv_xyz.returnXYZ();

    std::vector<double> x, y, z;

    for (const auto &point_iter : data_container.xyz_data) {

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

/**
 * @brief Allows the viewing of the data from the .root file in 3 formats:
 * - UVW with the baseline extracted.
 * - Hits detected for each plane.
 * - The reconstructed XYZ points.
 *
 * @param fileName The name of the .root file.
 * @param norm_opt The normalization option. 1 if you want the channels to be
 * normalized, else 0.
 */
void view_data_entries(TString fileName, int norm_opt = 1) {

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

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    cleanUVW loc_clean_uvw;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;

    auto loc_canv =
        new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
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

    while (1) {

        generalDataStorage data_container;

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

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << std::endl;
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << std::endl;

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << std::endl;

        // use norm_opt to set normalization on or off
        if (norm_opt) {

            err = loc_conv_uvw.normalizeChannels();
            if (err != 0)
                std::cout << "Normalize channels error code " << err
                          << std::endl;
        }

        /* err = loc_conv_uvw.substractBl();

        if (err != 0)
            std::cout << "Substractbl error code " << err << std::endl; */

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << std::endl;

        reorderCh::reorderChfromFile<reorderCh::planeInfo_U>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_V>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_W>(
            data_container.uvw_data);

        // removeBackground::viewChargeHist(data_container.uvw_data, 1);

        loc_clean_uvw.setUVWData(data_container.uvw_data);

        auto *charge_canv = new TCanvas("Charge_canvas", "Charge_canvas");
        auto charge_pad = new TPad("charge_pad", "Charge pad", 0, 0, 1, 1);
        charge_pad->Divide(3, 1);
        charge_pad->Draw();

        TH1D *charge_u;
        TH1D *charge_v;
        TH1D *charge_w;

        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.getChargeHist<cleanUVW::planeInfoU>(charge_u);
        if (err != 0)
            std::cerr << "Error getChargeHist code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.getChargeHist<cleanUVW::planeInfoV>(charge_v);
        if (err != 0)
            std::cerr << "Error getChargeHist code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.getChargeHist<cleanUVW::planeInfoW>(charge_w);
        if (err != 0)
            std::cerr << "Error getChargeHist code " << err << std::endl;

        charge_pad->cd(1);
        charge_u->Draw();
        charge_pad->cd(2);
        charge_v->Draw();
        charge_pad->cd(3);
        charge_w->Draw();

        charge_canv->Update();

        data_container.uvw_data = loc_clean_uvw.returnDataUVW();

        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if (err != 0)
            std::cout << "Error set UVW data code " << err << std::endl;

        err = loc_convert_hit.getHitInfo();
        if (err != 0)
            std::cout << "Error get hit info code " << err << std::endl;

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout << "Hit data size " << data_container.hit_data.size()
                  << std::endl;
        std::cout << "Hist data size "
                  << data_container.raw_hist_container.size() << std::endl;

        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if (err != 0)
            std::cout << "Error get new vector code " << err << std::endl;

        err = loc_conv_xyz.makeConversionXYZ();
        if (err != 0)
            std::cout << "Error make conversion XYZ code " << err << std::endl;

        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout << "XYZ vector size " << data_container.xyz_data.size()
                  << std::endl;

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        if (u_graph != nullptr)
            u_graph->Delete();

        if (v_graph != nullptr)
            v_graph->Delete();

        if (w_graph != nullptr)
            w_graph->Delete();

        if (p_graph != nullptr)
            p_graph->Delete();

        if (p_graph3d != nullptr)
            p_graph3d->Delete();

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

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

        loc_pad->cd(1);
        u_hists->Draw("COLZ");
        loc_pad->cd(2);
        v_hists->Draw("COLZ");
        loc_pad->cd(3);
        w_hists->Draw("COLZ");

        std::vector<double> x_u;
        std::vector<double> y_u;
        std::vector<double> c_u;

        std::vector<double> x_v;
        std::vector<double> y_v;
        std::vector<double> c_v;

        std::vector<double> x_w;
        std::vector<double> y_w;
        std::vector<double> c_w;

        for (const auto &hit_iter : data_container.hit_data) {

            if (hit_iter.plane == 0) {

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);
                c_u.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 1) {

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);
                c_v.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 2) {

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);
                c_w.push_back(hit_iter.peak_y + hit_iter.base_line);
            }
        }

        if (data_container.hit_data.size() != 0) {

            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            // u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            // Set marker colors Pink > Red > Green > Blue
            if (c_u.size() != 0) {

                auto min_val_c_u = 0;
                auto max_val_c_u = 2000;

                std::transform(c_u.begin(), c_u.end(), c_u.begin(),
                               [min_val_c_u, max_val_c_u](const double x) {
                                   return (x - min_val_c_u) /
                                          (max_val_c_u - min_val_c_u);
                               });

                for (auto vec_i = 0; vec_i < c_u.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    u_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_u.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_u.at(vec_i));
                    } else if (c_u.at(vec_i) >= 0.33 && c_u.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_u.at(vec_i) / 2, 0);
                    } else if (c_u.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_u.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    u_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(4);
            u_graph->Draw("AP");

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            // v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            if (c_v.size() != 0) {

                auto min_val_c_v = 0;
                auto max_val_c_v = 2000;

                std::transform(c_v.begin(), c_v.end(), c_v.begin(),
                               [min_val_c_v, max_val_c_v](const double x) {
                                   return (x - min_val_c_v) /
                                          (max_val_c_v - min_val_c_v);
                               });

                for (auto vec_i = 0; vec_i < c_v.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    v_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_v.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_v.at(vec_i));
                    } else if (c_v.at(vec_i) >= 0.33 && c_v.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_v.at(vec_i) / 2, 0);
                    } else if (c_v.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_v.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    v_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(5);
            v_graph->Draw("AP");

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            // w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            if (c_w.size() != 0) {

                auto min_val_c_w = 0;
                auto max_val_c_w = 2000;

                std::transform(c_w.begin(), c_w.end(), c_w.begin(),
                               [min_val_c_w, max_val_c_w](const double x) {
                                   return (x - min_val_c_w) /
                                          (max_val_c_w - min_val_c_w);
                               });

                for (auto vec_i = 0; vec_i < c_w.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    w_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_w.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_w.at(vec_i));
                    } else if (c_w.at(vec_i) >= 0.33 && c_w.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_w.at(vec_i) / 2, 0);
                    } else if (c_w.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_w.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    w_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(6);
            w_graph->Draw("AP");

            loc_pad->Modified();
            loc_pad->Update();

        } else {
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }

        if (data_container.xyz_data.size() != 0) {

            std::vector<double> x, y, z, chg;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
            chg.push_back(0.0);

            for (const auto &point_iter : data_container.xyz_data) {

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
            // p_graph->SetMarkerColor(kBlack);
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle("XY coordinates projection; X axis; Y axis");
            if (chg.size() != 0) {

                auto min_val_chg = 0;
                auto max_val_chg = 6000;

                std::transform(chg.begin(), chg.end(), chg.begin(),
                               [min_val_chg, max_val_chg](const double x) {
                                   return (x - min_val_chg) /
                                          (max_val_chg - min_val_chg);
                               });

                for (auto vec_i = 0; vec_i < chg.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    p_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (chg.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * chg.at(vec_i));
                    } else if (chg.at(vec_i) >= 0.33 && chg.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * chg.at(vec_i) / 2, 0);
                    } else if (chg.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(chg.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    p_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(7);
            p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X "
                                "axis; Y axis; Z axis");
            loc_pad->cd(8);
            p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();

        } else {
            p_graph = nullptr;
            p_graph3d = nullptr;
        }

        loc_canv->Update();
    }

    loc_canv->WaitPrimitive();
}

/**
 * @brief Draws an UVW image of the entry from the .root file specified.
 *
 * @param filename The name of the .root file.
 * @param entry_nr The entry number to be converted.
 */
void drawUVWimage(TString filename = "./rootdata/data2.root",
                  int entry_nr = 429) {

    generalDataStorage data_container;

    auto goodFile = filename;

    auto goodTree = "tree";

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    err = good_data.decodeData(entry_nr);
    data_container.root_raw_data = good_data.returnRawData();

    convertUVW loc_conv_uvw(data_container.root_raw_data);

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    err = loc_conv_uvw.makeConversion();
    if (err != 0)
        std::cout << "Make conversion error code " << err << std::endl;

    /* err = loc_conv_uvw.substractBl();

    if (err != 0)
        std::cout << "Substractbl error code " << err << std::endl;

    err = loc_conv_uvw.drawChargeHist();

    if (err != 0)
        std::cout << "Draw Charge Hist error code " << err << std::endl; */

    data_container.uvw_data = loc_conv_uvw.returnDataUVW();

    auto loc_canv = new TCanvas("uvw format", "UVW hists", 800, 600);
    loc_canv->SetFrameLineColor(0);
    loc_canv->SetFrameLineWidth(0);
    loc_canv->SetBottomMargin(0);
    loc_canv->SetTopMargin(0);
    loc_canv->SetLeftMargin(0);
    loc_canv->SetRightMargin(0);

    gStyle->SetOptStat(0);

    auto u_hists =
        new TH2D(Form("u_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);
    auto v_hists =
        new TH2D(Form("v_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);
    auto w_hists =
        new TH2D(Form("w_hists_%d", entry_nr), "", 512, 1, 513, 100, 1, 101);

    u_hists->SetTickLength(0);
    TAxis *u_axis = u_hists->GetYaxis();
    u_axis->SetTickLength(0);
    u_hists->SetLabelSize(0);
    u_axis->SetLabelSize(0);

    v_hists->SetTickLength(0);
    TAxis *v_axis = v_hists->GetYaxis();
    v_axis->SetTickLength(0);
    v_hists->SetLabelSize(0);
    v_axis->SetLabelSize(0);

    w_hists->SetTickLength(0);
    TAxis *w_axis = w_hists->GetYaxis();
    w_axis->SetTickLength(0);
    w_hists->SetLabelSize(0);
    w_axis->SetLabelSize(0);

    int bin = 0;
    // int strip = 1;

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

/**
 * @brief Hepls with the mass conversion to pdf. Uses the create_entries_pdf()
 * function.
 *
 * @param lin_arg The file to be converted, taken from the command line
 * argument.
 */
void mass_convertPDF(TString lin_arg) {

    TString dirandfileName = lin_arg;

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index + 1, dir.Sizeof());

    index = fileName.Last('/');   // remove path

    fileName.Remove(0, index + 1);
    fileName.Remove(fileName.Sizeof() - 6, fileName.Sizeof());   // remove .root

    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append("pdffiles");

    gROOT->ProcessLine(mkdirCommand);

    TString pdfName = dir + "pdffiles/" + fileName + ".pdf";

    create_entries_pdf(dirandfileName, pdfName, 50);
}

/**
 * @brief Hepls with the mass conversion to pdf. Uses the create_raw_pdf()
 * function.
 *
 * @param lin_arg The file to be converted, taken from the command line
 * argument.
 * @param nr_entries THe number of entries to be converted.
 */
void mass_convertRawPDF(TString lin_arg, int nr_entries = 10000) {

    TString dirandfileName = lin_arg;

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index + 1, dir.Sizeof());

    index = fileName.Last('/');   // remove path

    fileName.Remove(0, index + 1);
    fileName.Remove(fileName.Sizeof() - 6, fileName.Sizeof());   // remove .root

    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append("rawpdf");

    gROOT->ProcessLine(mkdirCommand);

    TString pdfName = dir + "rawpdf/" + fileName + ".pdf";

    create_raw_pdf(dirandfileName, pdfName, nr_entries);
}

/**
 * @brief For testing purposes only. Helps view the reordered channels.
 *
 * @param fileName The name of the .root file.
 * @param entry_nr The entry number to be viewed.
 * @param plane The plane to be viewed.
 * @param norm_opt The normalization option. 1 if you want the channels to be
 * normalized, else 0.s
 */
void printReorderedChannels(TString fileName, int entry_nr, int plane,
                            int norm_opt = 1) {

    auto goodFile = fileName;

    auto goodTree = "tree";

    generalDataStorage data_container;

    loadData good_data(goodFile, goodTree);

    auto err = good_data.openFile();
    err = good_data.readData();
    auto max_entries = good_data.returnNEntries();

    convertUVW loc_conv_uvw;

    err = loc_conv_uvw.openSpecFile();

    if (err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    err = good_data.decodeData(entry_nr);
    if (err != 0) {
        std::cout << "Error decode data code " << err << std::endl;
    }

    data_container.root_raw_data = good_data.returnRawData();

    std::cout << "RAW data size is " << data_container.root_raw_data.size()
              << std::endl;

    loc_conv_uvw.setRawData(data_container.root_raw_data);

    err = loc_conv_uvw.makeConversion();
    if (err != 0)
        std::cout << "Make conversion error code " << err << std::endl;

    generalDataStorage data_container_raw;

    data_container_raw.uvw_data = loc_conv_uvw.returnDataUVW();

    // use norm_opt to set normalization on or off
    if (norm_opt) {

        err = loc_conv_uvw.normalizeChannels();
        if (err != 0)
            std::cout << "Normalize channels error code " << err << std::endl;
    }

    /* err = loc_conv_uvw.substractBl();

    if (err != 0)
        std::cout << "Substractbl error code " << err << std::endl; */

    data_container.uvw_data = loc_conv_uvw.returnDataUVW();

    std::cout << "UVW data size is " << data_container.uvw_data.size()
              << std::endl;

    auto loc_canv =
        new TCanvas("View entries canvas", "View entries", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
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

    u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                       Form("Histogram for U entry %d", entry_nr), 512, 1, 513,
                       100, 1, 101);
    v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                       Form("Histogram for V entry %d", entry_nr), 512, 1, 513,
                       100, 1, 101);
    w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                       Form("Histogram for W entry %d", entry_nr), 512, 1, 513,
                       100, 1, 101);

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

    loc_pad->cd(4);
    u_hists->Draw("COLZ");
    loc_pad->cd(5);
    v_hists->Draw("COLZ");
    loc_pad->cd(6);
    w_hists->Draw("COLZ");

    generalDataStorage data_container_ord = data_container;

    reorderCh::reorderChfromFile<reorderCh::planeInfo_U>(
        data_container_ord.uvw_data);
    reorderCh::reorderChfromFile<reorderCh::planeInfo_V>(
        data_container_ord.uvw_data);
    reorderCh::reorderChfromFile<reorderCh::planeInfo_W>(
        data_container_ord.uvw_data);

    u_hists_ord = new TH2D(Form("u_ord_hists_%d", entry_nr),
                           Form("Histogram ord for U entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);
    v_hists_ord = new TH2D(Form("v_ord_hists_%d", entry_nr),
                           Form("Histogram ord for V entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);
    w_hists_ord = new TH2D(Form("w_ord_hists_%d", entry_nr),
                           Form("Histogram ord for W entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);

    for (const auto &iter : data_container_ord.uvw_data) {

        bin = 0;

        if (iter.plane_val == 0) {

            for (const auto &sig_iter : iter.signal_val) {

                u_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }

        if (iter.plane_val == 1) {

            for (const auto &sig_iter : iter.signal_val) {

                v_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }

        if (iter.plane_val == 2) {

            for (const auto &sig_iter : iter.signal_val) {

                w_hists_ord->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }
    }

    loc_pad->cd(7);
    u_hists_ord->Draw("COLZ");
    loc_pad->cd(8);
    v_hists_ord->Draw("COLZ");
    loc_pad->cd(9);
    w_hists_ord->Draw("COLZ");

    loc_conv_uvw.setUVWData(data_container_ord.uvw_data);

    u_hists_raw = new TH2D(Form("u_raw_hists_%d", entry_nr),
                           Form("Histogram raw for U entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);
    v_hists_raw = new TH2D(Form("v_raw_hists_%d", entry_nr),
                           Form("Histogram raw for V entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);
    w_hists_raw = new TH2D(Form("w_raw_hists_%d", entry_nr),
                           Form("Histogram raw for W entry %d", entry_nr), 512,
                           1, 513, 100, 1, 101);

    for (const auto &iter : data_container_raw.uvw_data) {

        bin = 0;

        if (iter.plane_val == 0) {

            for (const auto &sig_iter : iter.signal_val) {

                u_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }

        if (iter.plane_val == 1) {

            for (const auto &sig_iter : iter.signal_val) {

                v_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }

        if (iter.plane_val == 2) {

            for (const auto &sig_iter : iter.signal_val) {

                w_hists_raw->SetBinContent(++bin, iter.strip_nr, sig_iter);
            }
        }
    }

    loc_pad->cd(1);
    u_hists_raw->Draw("COLZ");
    loc_pad->cd(2);
    v_hists_raw->Draw("COLZ");
    loc_pad->cd(3);
    w_hists_raw->Draw("COLZ");

    loc_canv->Update();

    loc_canv->WaitPrimitive();
}

void create_labeled_pdf(TString source_file, TString destination_file,
                        int read_entries, int norm_opt = 1) {

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

    if (err != 0)
        return;

    err = loc_conv_uvw.buildNormalizationMap();
    if (err != 0)
        return;

    cleanUVW loc_clean_uvw;

    convertHitData loc_convert_hit;

    convertXYZ loc_conv_xyz;

    filterEventsXY loc_filter_xy;

    gROOT->SetBatch(kTRUE);

    auto loc_canv = new TCanvas("entries.pdf", "PDF", 1500, 1000);
    auto loc_pad = new TPad("pad name", "pad title", 0, 0, 1, 1);
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

    // only read the number of entries wanted
    max_entries = std::min(max_entries, read_entries);

    while (entry_nr < max_entries) {

        generalDataStorage data_container;

        std::cout << "\n\n\nNow at entry: " << entry_nr << " of " << max_entries
                  << " for " << destination_file << "\n";

        err = good_data.decodeData(entry_nr);
        if (err != 0) {
            std::cout << "Error decode data code " << err << "\n";
        }

        data_container.root_raw_data = good_data.returnRawData();

        std::cout << "RAW data size is " << data_container.root_raw_data.size()
                  << "\n";

        loc_conv_uvw.setRawData(data_container.root_raw_data);

        err = loc_conv_uvw.makeConversion();
        if (err != 0)
            std::cout << "Make conversion error code " << err << "\n";

        // use norm_opt to set normalization on or off
        if (norm_opt) {

            err = loc_conv_uvw.normalizeChannels();
            if (err != 0)
                std::cout << "Normalize channels error code " << err
                          << std::endl;
        }

        /* err = loc_conv_uvw.substractBl();

        if (err != 0)
            std::cout << "Substractbl error code " << err << "\n"; */

        data_container.uvw_data = loc_conv_uvw.returnDataUVW();

        std::cout << "UVW data size is " << data_container.uvw_data.size()
                  << "\n";

        reorderCh::reorderChfromFile<reorderCh::planeInfo_U>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_V>(
            data_container.uvw_data);
        reorderCh::reorderChfromFile<reorderCh::planeInfo_W>(
            data_container.uvw_data);

        loc_clean_uvw.setUVWData(data_container.uvw_data);

        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoU>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoV>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;
        err = loc_clean_uvw.substractBl<cleanUVW::planeInfoW>();
        if (err != 0)
            std::cerr << "Error substractBl code " << err << std::endl;

        data_container.uvw_data = loc_clean_uvw.returnDataUVW();

        err = loc_convert_hit.setUVWData(data_container.uvw_data);
        if (err != 0)
            std::cout << "Error set UVW data code " << err << "\n";

        err = loc_convert_hit.getHitInfo();
        if (err != 0)
            std::cout << "Error get hit info code " << err << "\n";

        data_container.hit_data = loc_convert_hit.returnHitData();
        data_container.raw_hist_container = loc_convert_hit.returnHistData();

        std::cout << "Hit data size " << data_container.hit_data.size() << "\n";
        std::cout << "Hist data size "
                  << data_container.raw_hist_container.size() << "\n";

        err = loc_conv_xyz.getNewVector(data_container.hit_data);
        if (err != 0)
            std::cout << "Error get new vector code " << err << "\n";

        err = loc_conv_xyz.makeConversionXYZ();
        if (err != 0)
            std::cout << "Error make conversion XYZ code " << err << "\n";

        data_container.xyz_data = loc_conv_xyz.returnXYZ();

        std::cout << "XYZ vector size " << data_container.xyz_data.size()
                  << "\n";

        data_container.n_entry = entry_nr;

        err = loc_filter_xy.filterAndPush(data_container);
        if (err != 0)
            std::cerr << "Error filter and push code " << err << "\n";

        entry_nr++;
    }

    loc_filter_xy.assignClass();
    // loc_filter_xy.assignClass_threaded();

    auto event_vec = loc_filter_xy.returnEventVector();

    for (const auto &curr_event : event_vec) {

        entry_nr = curr_event.n_entry;

        if (u_hists != nullptr)
            u_hists->Reset();

        if (v_hists != nullptr)
            v_hists->Reset();

        if (w_hists != nullptr)
            w_hists->Reset();

        if (u_graph != nullptr)
            u_graph->Delete();

        if (v_graph != nullptr)
            v_graph->Delete();

        if (w_graph != nullptr)
            w_graph->Delete();

        if (p_graph != nullptr)
            p_graph->Delete();

        if (p_graph3d != nullptr)
            p_graph3d->Delete();

        u_hists = new TH2D(Form("u_hists_%d", entry_nr),
                           Form("Histogram for U entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        v_hists = new TH2D(Form("v_hists_%d", entry_nr),
                           Form("Histogram for V entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);
        w_hists = new TH2D(Form("w_hists_%d", entry_nr),
                           Form("Histogram for W entry %d", entry_nr), 512, 1,
                           513, 100, 1, 101);

        int bin = 0;

        for (const auto &iter : curr_event.uvw_data) {

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

        loc_pad->cd(1);
        u_hists->Draw("COLZ");
        loc_pad->cd(2);
        v_hists->Draw("COLZ");
        loc_pad->cd(3);
        w_hists->Draw("COLZ");

        std::vector<double> x_u;
        std::vector<double> y_u;
        std::vector<double> c_u;

        std::vector<double> x_v;
        std::vector<double> y_v;
        std::vector<double> c_v;

        std::vector<double> x_w;
        std::vector<double> y_w;
        std::vector<double> c_w;

        for (const auto &hit_iter : curr_event.hit_data) {

            if (hit_iter.plane == 0) {

                x_u.push_back(hit_iter.peak_x);
                y_u.push_back(hit_iter.strip);
                c_u.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 1) {

                x_v.push_back(hit_iter.peak_x);
                y_v.push_back(hit_iter.strip);
                c_v.push_back(hit_iter.peak_y + hit_iter.base_line);

            } else if (hit_iter.plane == 2) {

                x_w.push_back(hit_iter.peak_x);
                y_w.push_back(hit_iter.strip);
                c_w.push_back(hit_iter.peak_y + hit_iter.base_line);
            }
        }

        if (curr_event.hit_data.size() != 0) {

            u_graph = new TGraph(x_u.size(), x_u.data(), y_u.data());
            u_graph->GetXaxis()->SetLimits(-1, 512);
            u_graph->GetHistogram()->SetMaximum(73);
            u_graph->GetHistogram()->SetMinimum(0);
            // u_graph->SetMarkerColor(kBlack);
            u_graph->SetMarkerStyle(kFullCircle);
            u_graph->SetTitle("Hits detected on U plane; Time; Strip");
            // Set marker colors Pink > Red > Green > Blue
            if (c_u.size() != 0) {

                auto min_val_c_u = 0;
                auto max_val_c_u = 2000;

                std::transform(c_u.begin(), c_u.end(), c_u.begin(),
                               [min_val_c_u, max_val_c_u](const double x) {
                                   return (x - min_val_c_u) /
                                          (max_val_c_u - min_val_c_u);
                               });

                for (auto vec_i = 0; vec_i < c_u.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    u_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_u.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_u.at(vec_i));
                    } else if (c_u.at(vec_i) >= 0.33 && c_u.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_u.at(vec_i) / 2, 0);
                    } else if (c_u.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_u.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    u_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(4);
            u_graph->Draw("AP");

            v_graph = new TGraph(x_v.size(), x_v.data(), y_v.data());
            v_graph->GetXaxis()->SetLimits(-1, 512);
            v_graph->GetHistogram()->SetMaximum(93);
            v_graph->GetHistogram()->SetMinimum(0);
            // v_graph->SetMarkerColor(kBlack);
            v_graph->SetMarkerStyle(kFullCircle);
            v_graph->SetTitle("Hits detected on V plane; Time; Strip");
            if (c_v.size() != 0) {

                auto min_val_c_v = 0;
                auto max_val_c_v = 2000;

                std::transform(c_v.begin(), c_v.end(), c_v.begin(),
                               [min_val_c_v, max_val_c_v](const double x) {
                                   return (x - min_val_c_v) /
                                          (max_val_c_v - min_val_c_v);
                               });

                for (auto vec_i = 0; vec_i < c_v.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    v_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_v.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_v.at(vec_i));
                    } else if (c_v.at(vec_i) >= 0.33 && c_v.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_v.at(vec_i) / 2, 0);
                    } else if (c_v.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_v.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    v_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(5);
            v_graph->Draw("AP");

            w_graph = new TGraph(x_w.size(), x_w.data(), y_w.data());
            w_graph->GetXaxis()->SetLimits(-1, 512);
            w_graph->GetHistogram()->SetMaximum(93);
            w_graph->GetHistogram()->SetMinimum(0);
            // w_graph->SetMarkerColor(kBlack);
            w_graph->SetMarkerStyle(kFullCircle);
            w_graph->SetTitle("Hits detected on W plane; Time; Strip");
            if (c_w.size() != 0) {

                auto min_val_c_w = 0;
                auto max_val_c_w = 2000;

                std::transform(c_w.begin(), c_w.end(), c_w.begin(),
                               [min_val_c_w, max_val_c_w](const double x) {
                                   return (x - min_val_c_w) /
                                          (max_val_c_w - min_val_c_w);
                               });

                for (auto vec_i = 0; vec_i < c_w.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    w_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (c_w.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * c_w.at(vec_i));
                    } else if (c_w.at(vec_i) >= 0.33 && c_w.at(vec_i) < 0.66) {
                        TColor *loc_color =
                            new TColor(0, 3 * c_w.at(vec_i) / 2, 0);
                    } else if (c_w.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(c_w.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    w_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(6);
            w_graph->Draw("AP");

            loc_pad->Modified();
            loc_pad->Update();

        } else {
            u_graph = nullptr;
            v_graph = nullptr;
            w_graph = nullptr;
        }

        if (curr_event.xyz_data.size() != 0) {

            std::vector<double> x, y, z, chg;

            x.push_back(-10.0);
            y.push_back(-10.0);
            z.push_back(-10.0);
            chg.push_back(0.0);

            for (const auto &point_iter : curr_event.xyz_data) {

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
            p_graph->SetMarkerStyle(kFullCircle);
            p_graph->SetTitle(
                Form("XY coordinates projection with size %d and MSE %f; "
                     "LABEL: %d; X axis; Y axis",
                     curr_event.xyz_data.size(), curr_event.mse_value,
                     curr_event.filter_label));
            if (chg.size() != 0) {

                auto min_val_chg = 0;
                auto max_val_chg = 6000;

                std::transform(chg.begin(), chg.end(), chg.begin(),
                               [min_val_chg, max_val_chg](const double x) {
                                   return (x - min_val_chg) /
                                          (max_val_chg - min_val_chg);
                               });

                for (auto vec_i = 0; vec_i < chg.size(); vec_i++) {

                    Double_t loc_x, loc_y;

                    p_graph->GetPoint(vec_i, loc_x, loc_y);

                    Int_t ci = TColor::GetFreeColorIndex();

                    if (chg.at(vec_i) < 0.33) {
                        TColor *loc_color = new TColor(0, 0, 3 * chg.at(vec_i));
                    } else if (chg.at(vec_i) >= 0.33 && chg.at(vec_i) < 0.66) {
                        TColor *loc_color = new TColor(0, 3 * chg.at(vec_i), 0);
                    } else if (chg.at(vec_i) > 1) {
                        TColor *loc_color = new TColor(1, 0, 1);
                    } else {
                        TColor *loc_color = new TColor(chg.at(vec_i), 0, 0);
                    }

                    TMarker *loc_marker = new TMarker(loc_x, loc_y, 20);
                    loc_marker->SetMarkerColor(ci);
                    p_graph->GetListOfFunctions()->Add(loc_marker);
                }
            }
            loc_pad->cd(7);
            p_graph->Draw("AP");
            loc_pad->Modified();
            loc_pad->Update();

            p_graph3d = new TGraph2D(x.size(), x.data(), y.data(), z.data());
            p_graph3d->SetMarkerColor(kBlue);
            p_graph3d->SetMarkerStyle(kFullCircle);
            p_graph3d->SetTitle("Reconstructed data in XYZ coordinates; X "
                                "axis; Y axis; Z axis");
            loc_pad->cd(8);
            p_graph3d->Draw("P0");
            loc_pad->Modified();
            loc_pad->Update();

        } else {
            p_graph = nullptr;
            p_graph3d = nullptr;
        }

        std::cout << "Written entry " << entry_nr << "\n\n";

        loc_pad->Update();
        loc_canv->Update();
        loc_canv->Print(destination_file);
    }

    gErrorIgnoreLevel = kPrint;
    loc_canv->Print(destination_file + "]");

    loc_canv->Close();

    if (loc_canv) {
        delete (loc_canv);
        loc_canv = nullptr;
    }

    gROOT->SetBatch(kFALSE);

    std::cout << "\n\n\nDONE!!!!!\n\n\n" << std::endl;
}

void runmacro(TString lin_arg) {

    /* view_data_entries("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
                      "CoBo_2018-06-20T10-51-39.459_0000.root",
                      1); */

    /* view_raw_data("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
                  "CoBo_2018-06-20T10-51-39.459_0000.root",
                  1); */

    /* create_entries_pdf("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
                       "CoBo_2018-06-20T10-51-39.459_0000.root",
                       "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/finalpdf/"
                       "CoBo_2018-06-20T10-51-39.459_0000.pdf",
                       10000, 1); */

    /* create_labeled_pdf(
        "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/"
        "CoBo_2018-06-20T10-51-39.459_0000.root",
        "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/labeledpdf/"
        "CoBo_2018-06-20T10-51-39.459_0000.pdf",
        100, 1); */

    create_labeled_pdf(
        "./rootdata/data2.root",
        "/media/gant/Expansion/tpc_root_raw/DATA_ROOT/labeledpdf/"
        "data2.pdf",
        200, 1);

    /* create_raw_pdf("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root",
                         "/media/gant/Expansion/tpcanalcsv/CoBo_2018-06-20T16-20-10.736_0000.pdf",
       10000); */

    // writeXYZcvs(429);

    // drawXYimage(429);

    // writeFullXYZCSV();

    // drawUVWimage(429);

    // mass_convertPDF(lin_arg);

    // mass_convertRawPDF(lin_arg, 10000);

    // createNormCSV();

    // printPeaksByChannel("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root",
    // 424, 0, 1);

    /* reorderCh::calculateReorder<reorderCh::planeInfo_U>
    ("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root");
    reorderCh::calculateReorder<reorderCh::planeInfo_V>
    ("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root");
    reorderCh::calculateReorder<reorderCh::planeInfo_W>
    ("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root");

    printReorderedChannels("/media/gant/Expansion/tpc_root_raw/DATA_ROOT/CoBo_2018-06-20T10-35-30.853_0005.root",
    134, 0, 1); */
}