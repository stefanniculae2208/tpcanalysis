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
/// @param opt_norm
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

            // Remove the first and last few bins.
            std::fill(data_el.signal_val.begin(),
                      data_el.signal_val.begin() + 10, 0);

            std::fill(data_el.signal_val.begin() + 500,
                      data_el.signal_val.begin() + 512, 0);

            // Bring all values that are close to 0 to 1 in order to not have
            // empty bins.
            std::transform(
                data_el.signal_val.begin(), data_el.signal_val.end(),
                data_el.signal_val.begin(),
                [](const double &sig_el) { return std::max(10.0, sig_el); });
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

/// @brief
/// @param data_vec
/// @return
std::vector<dataUVW> mergeSplitStrips(std::vector<dataUVW> &&data_vec) {

    std::multimap<std::pair<int, int>, size_t> stripSignalMap;

    for (size_t i = 0; i < data_vec.size(); ++i) {
        const auto &data = data_vec[i];
        stripSignalMap.insert({{data.plane_val, data.strip_nr}, i});
    }

    std::vector<dataUVW> averaged_data;

    dataUVW averaged_entry;

    for (auto it = stripSignalMap.begin(); it != stripSignalMap.end();) {

        // Range of entries with the same strip_nr
        auto range = stripSignalMap.equal_range(it->first);

        size_t signal_size = data_vec[it->second].signal_val.size();

        std::vector<double> averaged_signal_val(
            data_vec.front().signal_val.size(), 0.0);

        int count = 0;

        for (auto iter = range.first; iter != range.second; ++iter) {

            size_t index = iter->second;
            const auto &signal_val = data_vec[index].signal_val;

            for (size_t i = 0; i < signal_size; ++i) {
                averaged_signal_val[i] += signal_val[i];
            }
            count++;
        }

        /* for (auto &val : averaged_signal_val) {

            val /= count;
        } */

        averaged_entry.signal_val = std::move(averaged_signal_val);
        averaged_entry.baseline_val = data_vec[it->second].baseline_val;
        averaged_entry.plane_val = data_vec[it->second].plane_val;
        averaged_entry.strip_nr = data_vec[it->second].strip_nr;
        averaged_entry.entry_nr = data_vec[it->second].entry_nr;
        averaged_entry.event_id = data_vec[it->second].event_id;
        averaged_entry.strip_section = 0;

        averaged_data.push_back(std::move(averaged_entry));

        it = range.second;
    }

    return averaged_data;
}

};   // namespace funcUtil

/// @brief
/// @param fileName
/// @param opt_norm
/// @param opt_clean
void viewUVWdata_elitpc(std::initializer_list<TString> fileName,
                        bool opt_norm = false, bool opt_clean = false) {

    std::vector<generalDataStorage> data_vec;

    // ELITPC CoBo files usually come in groups of 4, one for each AsAd, so we
    // have to combine them.
    for (const auto &file_entry : fileName) {
        try {

            // data_vec = funcUtil::getAllEntries(file_entry, true, true);
            std::vector<generalDataStorage> temp_data =
                funcUtil::getAllEntries(file_entry, true, opt_norm);
            data_vec.insert(data_vec.end(),
                            std::make_move_iterator(temp_data.begin()),
                            std::make_move_iterator(temp_data.end()));
        } catch (const std::exception &e) {

            std::cout << "Encountered error when getting all entries."
                      << std::endl;
            return;
        }
    }

    cleanUVW loc_clean_uvw;

    data_vec = funcUtil::groupEntriesByEvent(std::move(data_vec));

    std::cout << "New data size is " << data_vec.size() << "\n\n";

    for (auto &&event : data_vec) {

        event.uvw_data = funcUtil::mergeSplitStrips(std::move(event.uvw_data));
    }

    int nr_events = data_vec.size();

    std::cout << "There are " << nr_events << " events."
              << "\n\n";

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

            if (event_nr < (nr_events - 1)) {

                event_nr++;
            }

            std::cout << "Event number is " << event_nr << "\n\n" << std::endl;

        } else if (key_val == 'a') {

            if (event_nr > 0) {
                event_nr--;
            }

            std::cout << "Event number is " << event_nr << "\n\n" << std::endl;

        } else if (key_val == 'i') {

            std::cout << "Please enter the entry number.\n";

            std::cin >> event_nr;

            if (event_nr < 0 || event_nr > (nr_events - 1))
                event_nr = 0;

            std::cout << "Event number is " << event_nr << "\n\n" << std::endl;
        }

        if (event_nr < 0) {
            event_nr = 0;
        }

        if (opt_clean) {

            loc_clean_uvw.setUVWData(data_vec[event_nr].uvw_data);

            auto err =
                loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoU>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoV>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoW>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            data_vec[event_nr].uvw_data = loc_clean_uvw.returnDataUVW();
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

        std::cout << "\n\nViewing event " << event_nr
                  << " with Id: " << data_vec[event_nr].event_id << " and size "
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
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

                for (const auto &sig_iter : iter.signal_val) {

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 1 &&
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

                for (const auto &sig_iter : iter.signal_val) {

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 2 &&
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

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

/// @brief Creates images from the data from the specified root file.
/// @param fileName The root file. Must include path if needed.
/// @param outfolder The loction where the images are created.
/// @param opt_norm Enables the normalization of the channels.
/// @param opt_clean Enables further cleaning and smoothing of the signal.
void drawUVWimages_elitpc(std::initializer_list<TString> fileName,
                          TString outfolder, bool opt_norm = false,
                          bool opt_clean = false)

{

    gROOT->SetBatch(kTRUE);

    std::vector<generalDataStorage> data_vec;

    // ELITPC CoBo files usually come in groups of 4, one for each AsAd, so we
    // have to combine them.
    for (const auto &file_entry : fileName) {
        try {

            // data_vec = funcUtil::getAllEntries(file_entry, true, true);
            std::vector<generalDataStorage> temp_data =
                funcUtil::getAllEntries(file_entry, true, opt_norm);
            data_vec.insert(data_vec.end(),
                            std::make_move_iterator(temp_data.begin()),
                            std::make_move_iterator(temp_data.end()));
        } catch (const std::exception &e) {

            std::cout << "Encountered error when getting all entries."
                      << std::endl;
            return;
        }
    }

    cleanUVW loc_clean_uvw;

    data_vec = funcUtil::groupEntriesByEvent(std::move(data_vec));

    std::cout << "New data size is " << data_vec.size() << "\n\n";

    for (auto &&event : data_vec) {

        event.uvw_data = funcUtil::mergeSplitStrips(std::move(event.uvw_data));
    }

    int nr_events = data_vec.size();

    std::cout << "There are " << nr_events << " events."
              << "\n\n";

    auto loc_canv = new TCanvas("uvw format", "UVW hists", 800, 600);
    loc_canv->SetFrameLineColor(0);
    loc_canv->SetFrameLineWidth(0);
    loc_canv->SetBottomMargin(0);
    loc_canv->SetTopMargin(0);
    loc_canv->SetLeftMargin(0);
    loc_canv->SetRightMargin(0);

    gStyle->SetOptStat(0);
    // gStyle->SetPalette(kGreyScale);

    TH2D *u_hists = nullptr;
    TH2D *v_hists = nullptr;
    TH2D *w_hists = nullptr;
    TH1D *c_hist = nullptr;

    for (auto event_nr = 0; event_nr < nr_events; ++event_nr) {

        if (opt_clean) {

            loc_clean_uvw.setUVWData(data_vec[event_nr].uvw_data);

            auto err =
                loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoU>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoV>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            err = loc_clean_uvw.substractBl<cleanUVW::miniPlaneInfoW>(true);
            if (err != 0)
                std::cerr << "Error substractBl code " << err << std::endl;

            data_vec[event_nr].uvw_data = loc_clean_uvw.returnDataUVW();
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

        u_hists = new TH2D(Form("u_hists_%d", event_nr), "", 512, 1, 513, 132,
                           1, 133);
        v_hists = new TH2D(Form("v_hists_%d", event_nr), "", 512, 1, 513, 225,
                           1, 226);
        w_hists = new TH2D(Form("w_hists_%d", event_nr), "", 512, 1, 513, 226,
                           1, 227);

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

        u_hists->SetMinimum(-1);
        u_hists->SetMaximum(1000);

        v_hists->SetMinimum(-1);
        v_hists->SetMaximum(1000);

        w_hists->SetMinimum(-1);
        w_hists->SetMaximum(1000);

        // Fill the hists

        int bin = 0;

        std::array<double, 512> ch_array = {0};

        for (const auto &iter : data_vec[event_nr].uvw_data) {

            // All signals should have size 512;
            if (iter.signal_val.size() == 512) {

                for (auto i = 0; i < 512; ++i) {

                    ch_array[i] += iter.signal_val[i];
                }
            }

            bin = 0;

            if (iter.plane_val == 0 &&
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

                for (const auto &sig_iter : iter.signal_val) {

                    u_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 1 &&
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

                for (const auto &sig_iter : iter.signal_val) {

                    v_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }

            if (iter.plane_val == 2 &&
                (iter.strip_section == 0 /*  || iter.strip_section == 1 */)) {

                for (const auto &sig_iter : iter.signal_val) {

                    w_hists->SetBinContent(++bin, iter.strip_nr, sig_iter);
                }
            }
        }

        // Draw

        u_hists->Draw("COL");
        loc_canv->SetWindowSize(512, (3 * 132));
        loc_canv->Update();
        loc_canv->Print(outfolder + Form("%d", event_nr) + "_u" + ".png");

        v_hists->Draw("COL");
        loc_canv->SetWindowSize(512, (3 * 225));
        loc_canv->Update();
        loc_canv->Print(outfolder + Form("%d", event_nr) + "_v" + ".png");

        w_hists->Draw("COL");
        loc_canv->SetWindowSize(512, (3 * 226));
        loc_canv->Update();
        loc_canv->Print(outfolder + Form("%d", event_nr) + "_w" + ".png");
    }

    loc_canv->Close();

    gROOT->SetBatch(kFALSE);
}

/// @brief Creates clean images from the specified root files.
/// @param fileList The list of strings of the root files.
/// @param zip_opt Choose if you want to zip the images.
void mass_create_clean_images_elitpc(std::initializer_list<TString> fileList,
                                     bool zip_opt = false) {

    TString dirandfileName = *(fileList.begin());

    bool opt_norm = false;
    bool opt_clean = false;

    TString out_dir_name = "images_default";

    if (opt_clean && opt_norm) {

        out_dir_name = "images_aggressive";
    } else if (!opt_clean && opt_norm) {

        out_dir_name = "images_mild";
    } else if (!opt_clean && !opt_norm) {

        out_dir_name = "images_none";
    }

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index + 1, dir.Sizeof());

    index = fileName.Last('/');   // remove path

    fileName.Remove(0, index + 1);
    fileName.Remove(fileName.Sizeof() - 6, fileName.Sizeof());   // remove .root

    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append(out_dir_name);

    gROOT->ProcessLine(mkdirCommand);

    mkdirCommand.Append("/");
    mkdirCommand.Append(
        fileName);   // Create folder with the name of the root file.

    gROOT->ProcessLine(mkdirCommand);

    TString imagePath = dir + out_dir_name + "/" + fileName + "/";

    drawUVWimages_elitpc(fileList, imagePath, opt_norm, opt_clean);

    if (zip_opt) {

        TString zipCommand = ".! zip -j -r -m " + dir + out_dir_name + "/" +
                             fileName + ".zip " + imagePath;

        gROOT->ProcessLine(zipCommand);

        TString rmCommand = ".! rm -r " + imagePath;

        gROOT->ProcessLine(rmCommand);
    }
}

// root -q "runmacro_elitpc.cpp(\"a\")"
void runmacro_elitpc(TString lin_arg) {

    /* viewUVWdata_elitpc({"/media/gant/Expansion/elitpcraw/"
                        "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"});
     */

    viewUVWdata_elitpc({"/media/gant/Expansion/elitpcraw/"
                        "CoBo0_AsAd0_2022-04-12T08_03_44.531_0000.root",
                        "/media/gant/Expansion/elitpcraw/"
                        "CoBo0_AsAd1_2022-04-12T08_03_44.533_0000.root",
                        "/media/gant/Expansion/elitpcraw/"
                        "CoBo0_AsAd2_2022-04-12T08_03_44.536_0000.root",
                        "/media/gant/Expansion/elitpcraw/"
                        "CoBo0_AsAd3_2022-04-12T08_03_44.540_0000.root"},
                       false, false);

    /* mass_create_clean_images_elitpc(
        {"/media/gant/Expansion/elitpcraw/"
         "CoBo0_AsAd0_2022-04-12T08_03_44.531_0000.root",
         "/media/gant/Expansion/elitpcraw/"
         "CoBo0_AsAd1_2022-04-12T08_03_44.533_0000.root",
         "/media/gant/Expansion/elitpcraw/"
         "CoBo0_AsAd2_2022-04-12T08_03_44.536_0000.root",
         "/media/gant/Expansion/elitpcraw/"
         "CoBo0_AsAd3_2022-04-12T08_03_44.540_0000.root"},
        false); */

    /* testConvertUVW("/media/gant/Expansion/elitpcraw/"
                   "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"); */

    /* mass_create_raw_images_elitpc(
        "/media/gant/Expansion/elitpcraw/"
        "CoBo_ALL_AsAd_ALL_2021-07-12T11 36 07.452_0000.root"); */
}