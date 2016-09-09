/***************************************************************************
*
* Time: 2010-05-14
* Project: 遥感算法 
* Purpose: PCA变换
* Author:  李民录
* Copyright (c) 2010, liminlu0314@gmail.com
* Describe:提供PCA变换算法
*
****************************************************************************/
#ifndef PCATRANSFORM_H
#define PCATRANSFORM_H

/**
* \file PCATransform.h 
* @brief 图像PCA变换
*
* 用于图像PCA变换
*/

#include "MartixAlgo.h"	//用于矩阵求解特征值等
class CPCATransform
{
public:
	/**
	* @brief 构造函数，用于主成分变换
	* @param pszSrcFile 要变换的文件路径
	* @param pProcess	进度指针
	*/
	CPCATransform(const char* pszSrcFile, CProcessBase *pProcess = NULL);

	/**
	* @brief 析构函数
	*/
	~CPCATransform();

	/**
	* @brief PC变换
	* @param pszPCAFile			输出主成分变换后文件的路径
	* @param iBandCount			主成分变换后文件的的波段个数（默认为全部-1）
	* @param bIsCovariance		采用相关系数还是方差-协方差矩阵来计算，默认为协方差矩阵
	* @param bIsLikeEnvi		计算结果是否按照ENVI方式输出，即将所有的数据减去均值，使得每个波段的均值为0
	* @param pszFormat			输出文件格式，默认为GeoTiff格式
	* @return 返回代码
	*/
	int ExecutePCT(const char* pszPCAFile, int iBandCount = -1, bool bIsCovariance = true, 
		bool bIsLikeEnvi = true, const char* pszFormat = "GTiff");

	/**
	* @brief PC逆变换
	* @param pszPCAFile			输出主成分变换后文件的路径
	* @param pmMatrix			主成分变换的特征向量矩阵
	* @param pvMeanVector		原始图像的均值向量，可以为NULL
	* @param pszFormat			输出文件格式，默认为GeoTiff格式
	* @return 返回代码
	*/
	int ExecuteInversePCT(const char* pszPCAFile, const Matrix *pmMatrix,
		const Vector *pvMeanVector = NULL, const char* pszFormat = "GTiff");

	/**
	* @brief 获取PCA变换的变换矩阵和向量
	* @param mEigenVectors	特征向量矩阵
	* @param vMeanValues	均值向量
	*/
	void GetPCAMatrix(Matrix &mEigenVectors, Vector &vMeanValues);

private:

	/**
	* @brief 数据预处理，进行图像信息的统计等
	* @return 返回代码
	*/
	int PreProcessData();

	/**
	* @brief 计算协方差矩阵和相关系数矩阵R，第二步
	* @return 返回代码
	*/
	int CalcCovarianceMartix();

	/**
	* @brief 计算特征值和特征向量，第三步和计算贡献率以及累积贡献率，第四步
	*/
	void CalcEigenvalueAndEigenvector();

	/**
	* @brief 计算主成分得分，并写入到文件中，第五和第六步
	* @param pszPCAFile		输出主成分变换后文件的路径
	* @param iBandCount		主成分变换后文件的的波段个数（默认为全部-1）
	* @param pszFormat		输出文件格式
	* @return 返回代码
	*/
	int CreatePCAFile(const char* pszPCAFile, int iBandCount, const char* pszFormat);

	/**
	* @brief 计算主成分得分，并写入到文件中，第五和第六步
	* @param pszPCAFile 输出主成分变换后文件的路径
	* @return 返回代码
	*/
	int CalcSubAvg(const char* pszPCAFile);

private:			
	CProcessBase *m_pProcess;	/*<! 进度指针 */
	const char* m_pszSrcFile;		/*<! 要变换的文件路径 */
	bool m_bIsCovariance;			/*<! PCA变换方式，true为协方差，false为相关系数 */

	GDALDataset *m_pSrcDS;			/*<! 要变换的文件指针 */

	int m_iBandCount;			 	/*<! 波段个数 */	
	double *m_pBandMean;			/*<! 波段均值 */	
	double *m_pBandStad;			/*<! 波段标准差 */

	double *m_pRelativity;			/*<! 相关系数矩阵中的元素 */
	SymmMatrix m_RelMatrix;			/*<! 相关系数矩阵 */
	Vector m_EigenValues;			/*<! 相关系数矩阵的特征值 */
	Matrix m_EigenVectors;			/*<! 相关系数矩阵的特征向量 */
	Matrix m_MEV;					/*<! 构建选择后的特征向量矩阵 */
};

#endif /*PCATRANSFORM_H*/




//类的实现


/***************************************************************************
*
* Time: 2010-05-14
* Project: 遥感算法
* Purpose: PCA变换
* Author:  李民录
* Copyright (c) 2010, liminlu0314@gmail.com
* Describe:提供PCA变换算法
*
****************************************************************************/
#include "PCATransform.h"
#include "gdal_priv.h"	//只需要GDAL的头文件即可

CPCATransform::CPCATransform(const char* pszSrcFile, CProcessBase *pProcess)
{
	m_pBandMean		= NULL;
	m_pBandStad		= NULL;
	m_pRelativity	= NULL;
	m_pSrcDS		= NULL;

	m_bIsCovariance = true;
	m_pszSrcFile = pszSrcFile;
	m_pProcess = pProcess;
}

CPCATransform::~CPCATransform()
{
	if (m_pSrcDS != NULL)
		GDALClose( (GDALDatasetH) m_pSrcDS );

	RELEASE(m_pBandMean);
	RELEASE(m_pBandStad);
	RELEASE(m_pRelativity);
}

int CPCATransform::PreProcessData()
{
	GDALAllRegister();
	m_pSrcDS = (GDALDataset*) GDALOpen(m_pszSrcFile, GA_ReadOnly);
	if (m_pSrcDS == NULL)
	{
		if (m_pProcess != NULL)
			m_pProcess->SetMessage("输入文件不能打开！");

		return RE_FILENOTEXIST;
	}
	
	m_iBandCount = m_pSrcDS->GetRasterCount();
	m_pBandMean = new double [m_iBandCount];
	m_pBandStad = new double [m_iBandCount];

	for (int i=1; i<=m_iBandCount; i++)	//获取每个波段的均值和标准差
	{
		double dMaxValue, dMinValue;
		m_pSrcDS->GetRasterBand(i)->ComputeStatistics(FALSE, &dMinValue, &dMaxValue, 
			m_pBandMean+(i-1), m_pBandStad+(i-1), NULL, NULL);
	}

	return RE_SUCCESS;
}

int CPCATransform::CalcCovarianceMartix()
{
	if (m_pSrcDS == NULL)
	{
		if (m_pProcess != NULL)
			m_pProcess->SetMessage("输入文件不能打开！");

		return RE_FILENOTEXIST;
	}

	int iWidth = m_pSrcDS->GetRasterXSize();
	int iHeight = m_pSrcDS->GetRasterYSize();

	int iImageSize = iWidth * iHeight;

	int iElementNum = m_iBandCount*(m_iBandCount+1)/2;	//相关系数矩阵中的个数，只保存对角阵，因为是实对称阵
	int iElementIndex = 0;								//用于遍历相关系数矩阵中元素的索引
	m_pRelativity = new double [iElementNum];			//分配相关系数矩阵的大小

	SymmMatrix covMatrix(m_pRelativity, m_iBandCount, m_iBandCount);	//相关系数矩阵

	for (int i1=1; i1<=m_iBandCount; i1++)
	{
		for (int i2=1; i2<=m_iBandCount; i2++)
		{
			if (i2<i1)
				continue;

			if (i1 == i2)	//若是同一个波段，不用计算了
			{
				if (!m_bIsCovariance)
					m_pRelativity[iElementIndex] = 1.0;		//相关系数矩阵
				else
					m_pRelativity[iElementIndex] = m_pBandStad[i1-1]*m_pBandStad[i1-1];	//方差-协方差矩阵

				iElementIndex++;
				continue;
			}

			GDALRasterBand *ptrBandI1 = m_pSrcDS->GetRasterBand(i1);
			GDALRasterBand *ptrBandI2 = m_pSrcDS->GetRasterBand(i2);

			DT_64F *pBuff1 = new DT_64F[iWidth];
			DT_64F *pBuff2 = new DT_64F[iWidth];

			double dTemp = 0.0;
			for(int j=0; j<iHeight; j++)//行
			{
				ptrBandI1->RasterIO(GF_Read, 0, j, iWidth, 1, pBuff1, iWidth, 1, GDT_Float64, 0, 0);		//读取第一波段数据块
				ptrBandI2->RasterIO(GF_Read, 0, j, iWidth, 1, pBuff2, iWidth, 1, GDT_Float64, 0, 0);		//读取第二波段数据块

				for (int i=0; i<iWidth; i++)
					dTemp += ((pBuff1[i] - m_pBandMean[i1-1]) * (pBuff2[i] - m_pBandMean[i2-1]));
			}

			RELEASE(pBuff1);
			RELEASE(pBuff2);

			m_pRelativity[iElementIndex] = dTemp / iImageSize;	//方差-协方差矩阵

			if (!m_bIsCovariance)
				m_pRelativity[iElementIndex] = m_pRelativity[iElementIndex] / (m_pBandStad[i1-1]*m_pBandStad[i2-1]); //相关系数矩阵

			iElementIndex++;
		}
	}

	m_RelMatrix = covMatrix;

	CalcEigenvalueAndEigenvector();	//计算特征值和特征向量
	return RE_SUCCESS;
}

void CPCATransform::CalcEigenvalueAndEigenvector()
{
	/************************************************************************/
	/*               计算相关系数矩阵的特征值和特征向量                     */
	/************************************************************************/
	Matrix matrix(m_iBandCount, m_iBandCount);
	for(int i=0; i<m_iBandCount; i++)
	{
		for(int j=0; j<m_iBandCount; j++)
			matrix(i, j) = m_RelMatrix(i, j);
	}

	Vector EigenValues(m_iBandCount);
	Matrix EigenVectors(m_iBandCount, m_iBandCount);
	Vector Contribute(m_iBandCount);
	Vector AccContribute(m_iBandCount);
	GetMatrixEigen(matrix, EigenValues, EigenVectors, &Contribute, &AccContribute, 0.0001);

	m_EigenValues = EigenValues;
	m_EigenVectors = EigenVectors;
}

int CPCATransform::CreatePCAFile(const char* pszPCAFile, int iBandCount, const char* pszFormat)
{
	if (m_pSrcDS == NULL)
	{
		if (m_pProcess != NULL)
			m_pProcess->SetMessage("输入文件不能打开！");

		return RE_FILENOTEXIST;
	}

	int iWidth = m_pSrcDS->GetRasterXSize();
	int iHeight = m_pSrcDS->GetRasterYSize();
	
	int iNewBandCount = m_iBandCount;
	if (iBandCount>0)
		iNewBandCount = iBandCount;

	Matrix MEV(m_iBandCount, iNewBandCount);	//构建选择后的特征向量矩阵
	for (int it=0; it<iNewBandCount; it++)
	{
		for (int jt=0; jt<m_iBandCount; jt++)
			MEV(jt, it) = m_EigenVectors(jt, it);
	}

	m_MEV = MEV;	//构建选择后的特征向量矩阵
	
	return LinearCombination(m_pszSrcFile, pszPCAFile, &m_MEV, NULL, pszFormat, m_pProcess);
}

int CPCATransform::CalcSubAvg(const char* pszPCAFile)
{
	GDALAllRegister();
	GDALDataset *pDS = (GDALDataset*) GDALOpen(pszPCAFile, GA_Update);
	if (pDS == NULL)
	{
		if(m_pProcess != NULL)
			m_pProcess->SetMessage("文件不能打开！");

		return RE_FILENOTSUPPORT;
	}

	int iWidth = pDS->GetRasterXSize();
	int iHeight = pDS->GetRasterYSize();
	int iBandCount = pDS->GetRasterCount();

	if (m_pProcess != NULL)
		m_pProcess->SetStepCount(iHeight*iBandCount);

	DT_32F *pBuff = new DT_32F[iWidth];
	for (int it=1; it<=iBandCount; it++)
	{
		GDALRasterBand *pBand = pDS->GetRasterBand(it);
		double dfMin = 0.0, dfMax = 0.0, dfMean = 0.0, dfStdDev = 0.0;
		pBand->ComputeStatistics (FALSE, &dfMin, &dfMax, &dfMean, &dfStdDev, NULL, NULL);

		for(int i=0; i<iHeight; i++)
		{
			pBand->RasterIO(GF_Read, 0, i, iWidth, 1, pBuff, iWidth, 1, GDT_Float32, 0, 0);

			for (int j=0; j<iWidth; j++)
				pBuff[j] = pBuff[j] - (float)dfMean;

			pBand->RasterIO(GF_Write, 0, i, iWidth, 1, pBuff, iWidth, 1, GDT_Float32, 0, 0);

			if (m_pProcess != NULL)
				m_pProcess->StepIt();
		}//end
	}

	GDALClose((GDALDatasetH) pDS);
	RELEASE(pBuff);

	string str = string(pszPCAFile) + ".aux.xml";
	remove(str.c_str());

	return RE_SUCCESS;
}

int CPCATransform::ExecutePCT(const char* pszPCAFile, int iBandCount, bool bIsCovariance, 
							  bool bIsLikeEnvi, const char* pszFormat)
{
	m_bIsCovariance = bIsCovariance;

	if (m_pProcess != NULL)
		m_pProcess->SetMessage("开始执行主成分变换...");

	int iRev = PreProcessData();
	if(iRev != RE_SUCCESS)
		return iRev;

	iRev = CalcCovarianceMartix();
	if(iRev != RE_SUCCESS)
		return iRev;

	iRev = CreatePCAFile(pszPCAFile, iBandCount, pszFormat);
	if(iRev != RE_SUCCESS)
		return iRev;

	if(bIsLikeEnvi)
	{
		iRev = CalcSubAvg(pszPCAFile);
		if(iRev != RE_SUCCESS)
			return iRev;
	}

	if (m_pProcess != NULL)
		m_pProcess->SetMessage("计算完成！");

	return RE_SUCCESS;
}

int CPCATransform::ExecuteInversePCT(const char* pszPCAFile, const Matrix *pmMatrix, 
									 const Vector *pvMeanVector, const char* pszFormat)
{
	if (m_pProcess != NULL)
		m_pProcess->SetMessage("开始执行主成分逆变换...");

	if (pmMatrix == NULL || pszPCAFile == NULL)
	{
		if (m_pProcess != NULL)
			m_pProcess->SetMessage("指定文件名为空或者矩阵为空...");

		return RE_PARAMERROR;
	}

	Matrix invMatrix(pmMatrix->nrows(), pmMatrix->ncols());
	if(!InverseMatrix(*pmMatrix, invMatrix))
	{
		if (m_pProcess != NULL)
			m_pProcess->SetMessage("指定的矩阵没有逆矩阵...");
	}

	int iRev = LinearCombination(m_pszSrcFile, pszPCAFile, &invMatrix, pvMeanVector, pszFormat, m_pProcess);
	if(iRev != RE_SUCCESS)
		return iRev;

	if (m_pProcess != NULL)
		m_pProcess->SetMessage("计算完成！");

	return RE_SUCCESS;
}

void CPCATransform::GetPCAMatrix(Matrix &mEigenVectors, Vector &vMeanValues)
{
	mEigenVectors = m_EigenVectors;
	vMeanValues.resize(m_iBandCount, 0);

	for (int i=0; i<m_iBandCount; i++)
		vMeanValues[i] = m_pBandMean[i];
}