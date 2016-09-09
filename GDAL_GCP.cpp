//////////////////////////////////////////////////////////////////////////////

#include "gdal.h"

class CRectifier  
{
public:
 CRectifier();
 CRectifier(CString filename);
 virtual ~CRectifier();

private:
 int m_Count;   //控制点个数 
 GDAL_GCP* m_Gcps; //控制点对
 CString m_ScrFileName; //原始文件名
 CString m_DestFileName; //目标文件名

public:
 // 功能：添加控制点
 // 参数：gcps-控制点对；nCount-控制点个数
 VOID AddGCPS(double* gcps,int nCount);
 
 // 功能：执行纠正操作
 BOOL Transform();

 // 功能：设置取源文件
 VOID SetScrFileName(CString filename);
 // 功能：获取目标文件
 CString GetDestFileName();
};

///////////////////////////////////////////////////////

#include "stdafx.h"
#include "RasterRectifier.h"
#include "CRectifier.h"

#include "./include/cpl_conv.h"
#include "./include/cpl_string.h"
#include "./include/ogrsf_frmts.h"
#include "./include/gdalwarper.h"

#pragma comment(lib,"lib//gdal_i.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRectifier::CRectifier()
{
 
}

CRectifier::CRectifier(CString filename)
{
 SetScrFileName(filename);
}

CRectifier::~CRectifier()
{

}

VOID CRectifier::AddGCPS(double* gcps,int nCount)
{
 m_Count = nCount;

 m_Gcps = new GDAL_GCP[m_Count];

 for (int i=0;i<m_Count;i++)
 {
  char pszId[5];
  sprintf(pszId,"GCP-%d",i);

  m_Gcps[i].pszId = pszId;
  m_Gcps[i].pszInfo="";

  m_Gcps[i].dfGCPPixel = gcps[4*i];
  m_Gcps[i].dfGCPLine = gcps[4*i+1];
  m_Gcps[i].dfGCPX = gcps[4*i+2];
  m_Gcps[i].dfGCPY = gcps[4*i+3];
  m_Gcps[i].dfGCPZ = 0;
 }
}
 
VOID CRectifier:: SetScrFileName(CString filename)
{
 m_ScrFileName = filename;
 int index = m_ScrFileName.ReverseFind('.') + 1;
 m_DestFileName = m_ScrFileName.Left(index) + "_Dest";
 m_DestFileName += m_ScrFileName.Right(m_ScrFileName.GetLength() - index );
}

CString CRectifier::GetDestFileName()
{
 return m_DestFileName;
}


BOOL CRectifier::Transform()
{
    GDALDatasetH  hSrcDS, hDstDS;
 GDALDriverH hDriver;
 GDALDataType eDT;
   
 try
    {
  GDALAllRegister();
  
  // Open the source file.

  hSrcDS = GDALOpen( m_ScrFileName, GA_ReadOnly );
  CPLAssert( hSrcDS != NULL );
  GDALSetGCPs(hSrcDS,m_Count,m_Gcps,NULL);

     // Create output with same datatype as first input band. 
  
  eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));
  
  // Get output driver (GeoTIFF format)

  hDriver = GDALGetDriverByName( "GTiff" );
  CPLAssert( hDriver != NULL );

  // Get Source coordinate system.

   const char *pszSrcWKT = NULL;
// 
//   pszSrcWKT = GDALGetProjectionRef( hSrcDS );
//   CPLAssert( pszSrcWKT != NULL && strlen(pszSrcWKT) > 0 );
// 
//   // Setup output coordinate system that is input by user. 
// 
   char *pszDstWKT = NULL;
// 
//   oTargetSRS.exportToWkt( &pszDstWKT );
//   CPLAssert( pszDstWKT != NULL && strlen(pszDstWKT) > 0 );


  // Create a transformer that maps from source pixel/line coordinates
  // to destination georeferenced coordinates (not destination 
  // pixel line).  We do that by omitting the destination dataset
  // handle (setting it to NULL).

  void *hTransformArg;

  hTransformArg = 
   GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT, NULL, pszDstWKT, 
            FALSE, 0, 1 );
  CPLAssert( hTransformArg != NULL );

  // Get approximate output georeferenced bounds and resolution for file.

  double adfDstGeoTransform[6];
  int nPixels=0, nLines=0;
  CPLErr eErr;

  eErr = GDALSuggestedWarpOutput( hSrcDS, 
          GDALGenImgProjTransform, hTransformArg, 
          adfDstGeoTransform, &nPixels, &nLines );
  CPLAssert( eErr == CE_None );

  GDALDestroyGenImgProjTransformer( hTransformArg );

  // 获得波段数目

  int nBandCount = GDALGetRasterCount(hSrcDS);

  // Create the output file. 

  hDstDS = GDALCreate( hDriver, m_DestFileName, nPixels, nLines, 
        nBandCount, eDT, NULL );
    
  CPLAssert( hDstDS != NULL );

  // Write out the projection definition.

  GDALSetProjection( hDstDS, pszDstWKT );

  GDALSetGeoTransform( hSrcDS, adfDstGeoTransform );
  GDALSetGeoTransform( hDstDS, adfDstGeoTransform );

  // 逐个波段获取颜色表
  GDALColorTableH hCT;
  
  for (int i=1;i<=nBandCount;i++)//band从1开始算
  {
   hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,i) );
   if( hCT != NULL )
    GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,i), hCT );
  }
  
  // Setup warp options.
    
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;

  char** papszWarpOptions = NULL;
        papszWarpOptions = CSLSetNameValue(papszWarpOptions,
                                               "TFW", "YES");

  psWarpOptions->papszWarpOptions = papszWarpOptions;

  psWarpOptions->nBandCount = nBandCount;
  
  // 申请内存

  psWarpOptions->panSrcBands = 
   (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );  
  psWarpOptions->panDstBands = 
   (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  
  for (int j=0; j<nBandCount; j++)
  {
   psWarpOptions->panSrcBands[j] = j + 1;  
   psWarpOptions->panDstBands[j] = j + 1;
  }

  psWarpOptions->pfnProgress = GDALTermProgress;  

  // Establish reprojection transformer.

  psWarpOptions->pTransformerArg = 
   GDALCreateGenImgProjTransformer( hSrcDS, 
            GDALGetProjectionRef(hSrcDS), 
            hDstDS,
            GDALGetProjectionRef(hDstDS), 
            FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.

  GDALWarpOperation oOperation;

  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0, 
           GDALGetRasterXSize( hDstDS ), 
           GDALGetRasterYSize( hDstDS ) );

  CSLDestroy( papszWarpOptions );

  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );


  GDALClose( hDstDS );
  GDALClose( hSrcDS );

  return true;
 }
    catch(...)
    {
        return false; 
    }
}

 

////////////////////////////////////////////////////////////////////////////////////////