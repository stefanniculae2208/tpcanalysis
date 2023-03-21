#ifndef filterEventsXY
#define filterEventsXY_hpp 1

#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include "dataXYZ.hpp"
#include "generalDataStorage.hpp"

class filterEventsXY {
  public:
    filterEventsXY(){};
    ~filterEventsXY(){};

    int filterAndPush(generalDataStorage event_entry);
    int assignClass();
    int assignClass_threaded();

    std::vector<generalDataStorage> returnEventVector();
    std::vector<generalDataStorage> &&moveEventVector();

  private:
    std::vector<generalDataStorage> m_event_vec;
    static const std::size_t min_event_size = 50;
    static constexpr double mse_limit = 12;

    void assignClassToEvent(std::vector<generalDataStorage>::iterator start_it,
                            std::vector<generalDataStorage>::iterator end_it);
    void calculateLinRegCoeff(const std::vector<dataXYZ> &curr_event,
                              double &coeff_m, double &coeff_b);
    void calculateMSE(const std::vector<dataXYZ> &curr_event,
                      const double &coeff_m, const double &coeff_b,
                      double &mse_value);
};

#endif