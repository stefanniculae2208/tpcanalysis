#include "../include/convMatData.hpp"


convMatData::convMatData()
{

    driftVel = 1;

}


convMatData::~convMatData(){};


//We set the data with the correct values

void convMatData::setMatrix(double newDriftVel)
{

    this->driftVel = newDriftVel;

    matrixData[0][0] = cos(0);

    matrixData[0][1] = sin(0);

    matrixData[0][2] = 0;


    matrixData[1][0] = cos((-1) * 2 * M_PI /3);

    matrixData[1][1] = sin((-1) * 2 * M_PI /3);

    matrixData[1][2] = 0;


    matrixData[2][0] = cos((-1) * M_PI /3);

    matrixData[2][1] = sin((-1) * M_PI /3);

    matrixData[2][2] = 0;


    matrixData[3][0] = 0;

    matrixData[3][1] = 0;

    matrixData[3][2] = 1 / this->driftVel;



}


//Print the matrix for testing purposes

void convMatData::printMatrix()
{

    for(int i = 0; i < 4; i++){

        for(int j = 0; j < 3; j++){

            std::cout<<matrixData[i][j]<<"    ";

        }

        std::cout<<std::endl;

    }
        

}



//Information about how the converison is made is found here http://172.18.7.158/index.php?plugin=attach&refer=Presentations&openfile=NTNPD2021_Fila.pdf page 4
//We basically solve the system of 4 equations with 3 unknowns

int convMatData::convertCoords(double data_u, double data_v, double data_w, double data_t, dataXYZ &data_xyz)
{

    data_xyz.data_x = data_u;

    std::cout<<"x is "<<data_xyz.data_x<<std::endl;




    data_xyz.data_y = (data_v + (-1) * matrixData[1][0] * data_xyz.data_x) / matrixData[1][1];

    double data_y_check = (data_w + (-1) * matrixData[2][0] * data_xyz.data_x) / matrixData[2][1];

    std::cout<<"From first eq y is "<<data_xyz.data_y<<std::endl;

    std::cout<<"From second eq y is "<<data_y_check<<std::endl;

    



    data_xyz.data_z = this->driftVel * data_t;

    std::cout<<"z is "<<data_xyz.data_z<<std::endl;






    //check if the difference between y we got from the first equation and the second equation is bigger than 30% of y
    //if so we return the error code -1
    if(abs((data_xyz.data_y - data_y_check)) > (0.3 * data_xyz.data_y))
        return -1;




    return 0;

}

