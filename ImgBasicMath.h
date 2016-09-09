#ifndef IMGBASICMATH_H
#define IMGBASICMATH_H

class GDALDataset;

class ImgBasicMath
{
private:
    int imgWidth;
    int imgHeight;
    int imgBandCount;
    GDALDataset* dataset;
public:
    ImgBasicMath ( GDALDataset* dataset );
    virtual ~ImgBasicMath ();

    // calculate correlation matrix
    double* calCorrMat();

    // calculate convariance matrix
    double* calCovmat();



};




#endif
