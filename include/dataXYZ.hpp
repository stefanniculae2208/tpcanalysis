#ifndef dataXYZ_hpp
#define dataXYZ_hpp 1

/**
 * @brief Used for data storage.
 * The information is stored here in the XYZ format.
 *
 */
class dataXYZ {

  public:
    dataXYZ(){};
    ~dataXYZ(){};

    /// @brief The location on X.
    double data_x;

    /// @brief The location on Y.
    double data_y;

    /// @brief The location on Z.
    double data_z;

    /// @brief The value of the charge.
    double data_charge;
};

#endif