#include "ImgBasicMath.h"

#include <gdal_priv.h>

ImgBasicMath::ImgBasicMath( GDALDataset* dataset )
    : dataset( dataset )
{
    GDALAllRegister();

    if ( dataset != NULL )
    {
        imgWidth = dataset->GetRasterXSize();
        imgHeight = dataset->GetRasterYSize();
        imgBandCount = dataset->GetRasterCount();
    }
}


double* ImgBasicMath::calCovmat()
{
    double* covMat = new double[imgBandCount * imgBandCount];

    GDALDataType eDT = dataset->GetRasterBand( 1 )->GetRasterDataType();

    // initialize sum array
    double** sum = new double*[this->imgBandCount];
    for ( int i = 0; i < imgBandCount; i++ )
    {
        sum[i] = new double[this->imgBandCount];
        for ( int j = 0; j < imgBandCount; j++ )
        {
            sum[i][j] = 0.0;
        }
    }

    int lineCount = 4;
    if ( imgWidth > 1000 ) { lineCount = 50; }
    if ( imgWidth > 10000 ) { lineCount = 5 ; }
    int currentLineCount = lineCount;
    int tileCount = imgHeight / lineCount;

    int numY;
    for ( numY = 0; numY < tileCount + 1; numY++ )
    {
        // deal with last piece of tile
        if ( numY == tileCount )
        {
            currentLineCount = imgHeight % lineCount;
            if ( currentLineCount == 0 ) { break; }
        }

        // read tile data out
        float** data = new float*[imgBandCount];
        for ( int band = 0; band < imgBandCount; band++ )
        {
            int bandList = {band + 1};
            data[band] = new float[currentLineCount * imgWidth];
            dataset->RasterIO( GF_Read, 0, numY * lineCount, imgWidth, currentLineCount, data[band], imgWidth,
                               currentLineCount, GDT_Float32, 1, &bandList, 0, 0, 0 );
            int noDataValue;
            GDALGetRasterNoDataValue( dataset->GetRasterBand( bandList ), &noDataValue );
            for ( int j = 0; j < this->imgWidth * currentLineCount; j++ )
            {
                if( data[band][j] == noDataValue ||
                        !_finite( data[band][j] ) )
                { data[band][j] = 0; }
            }
        }

        // calculate sum
        for ( int band1 = 0; band1 < imgBandCount; band1++ )
        {
            for ( int band2 = 0; band2 < imgBandCount; band2++ )
            {
                for ( int i = 0; i < currentLineCount * imgWidth; i++ )
                {
                    sum[band1][band2] += ( data[band1][i] ) * ( data[band2][i] );
                }
            }
        }

        for ( int i = 0; i < imgBandCount; i++ )
        {
            delete[] data[i];
        }
        delete[] data;

    }

    for ( int i = 0; i < imgBandCount; i++ )
    {
        for ( int j = 0; j < imgBandCount; j++ )
        {
            covMat[i * imgBandCount + j] = sum[i][j] * 1.0 / ( imgWidth * imgHeight );
        }
    }

    return covMat;
}

double* ImgBasicMath::calCovmat()
{
    double* corrMat = new double[imgBandCount * imgBandCount];

    GDALDataType eDT = dataset->GetRasterBand( 1 )->GetRasterDataType();

    // calculate band mean vector
    double* bandMean = new double[this->imgBandCount];
    memset( bandMean, 0, sizeof( double )*imgBandCount );
    for ( int i = 0; i < this->imgBandCount; i++ )
    {
        dataset->GetRasterBand( i + 1 )->ComputeStatistics( FALSE, NULL, NULL, bandMean + i, 0, NULL, NULL );
    }
    if ( bandMean == NULL ) { throw std::string( "calculate band mean failed." ); }

    // initialize sum array
    double** sum = new double*[this->imgBandCount];
    for ( int i = 0; i < imgBandCount; i++ )
    {
        sum[i] = new double[this->imgBandCount];
        for ( int j = 0; j < imgBandCount; j++ )
        {
            sum[i][j] = 0.0;
        }
    }

    int lineCount = 4;
    if ( imgWidth > 1000 ) { lineCount = 50; }
    if ( imgWidth > 10000 ) { lineCount = 5 ; }
    int currentLineCount = lineCount;
    int tileCount = imgHeight / lineCount;

    int numY;
    for ( numY = 0; numY < tileCount + 1; numY++ )
    {
        // deal with last piece of tile
        if ( numY == tileCount )
        {
            currentLineCount = imgHeight % lineCount;
            if ( currentLineCount == 0 ) { break; }
        }

        // read tile data out
        float** data = new float*[imgBandCount];
        for ( int band = 0; band < imgBandCount; band++ )
        {
            int bandList = {band + 1};
            data[band] = new float[currentLineCount * imgWidth];
            dataset->RasterIO( GF_Read, 0, numY * lineCount, imgWidth, currentLineCount, data[band], imgWidth,
                               currentLineCount, GDT_Float32, 1, &bandList, 0, 0, 0 );
            int noDataValue;
            GDALGetRasterNoDataValue( dataset->GetRasterBand( bandList ), &noDataValue );
            for ( int j = 0; j < this->imgWidth * currentLineCount; j++ )
            {
                if( data[band][j] == noDataValue ||
                        !_finite( data[band][j] ) )
                { data[band][j] = 0; }
            }
        }

        // calculate sum
        for ( int band1 = 0; band1 < imgBandCount; band1++ )
        {
            for ( int band2 = 0; band2 < imgBandCount; band2++ )
            {
                for ( int i = 0; i < currentLineCount * imgWidth; i++ )
                {
                    sum[band1][band2] += ( data[band1][i] - bandMean[band1] ) * ( data[band2][i] - bandMean[band2] );
                }
            }
        }

        for ( int i = 0; i < imgBandCount; i++ )
        {
            delete[] data[i];
        }
        delete[] data;

    }

    for ( int i = 0; i < imgBandCount; i++ )
    {
        for ( int j = 0; j < imgBandCount; j++ )
        {
            corrMat[i * imgBandCount + j] = sum[i][j] * 1.0 / ( imgWidth * imgHeight );
        }
    }

    return corrMat;
}
