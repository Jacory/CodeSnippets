// hpscpca.cpp: implementation of the HpsCPCA class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\\hipas.h"
#include "hpscpca.h"
#include "HpsCStatistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
template<class TT>
void SortData(TT *pdata,int size,bool IsAscend = true)
{
	int i,j;
	TT tmp;
	if(IsAscend)
	{
		for(i=0;i<size;i++)
		{
			for(j=0;j<size-i-1;j++)
			{
				if(pdata[j]>pdata[j+1])
				{
					tmp = pdata[j];
					pdata[j] = pdata[j+1];
					pdata[j+1] = tmp;
				}

			}
		}
	}
	else
	{
		for(i=0;i<size;i++)
		{
			for(j=0;j<size-i-1;j++)
			{
				if(pdata[j]<pdata[j+1])
				{
					tmp = pdata[j];
					pdata[j] = pdata[j+1];
					pdata[j+1] = tmp;
				}

			}
		}
	}
}

matrix<double> *GetNormMatrix(DIMS &dims, HpsCFileManager *pmgr,double *mean)
{
	int PerPt = pmgr->GetBytesPerPt();
	int width = dims.Xend - dims.Xstart;
	int height = dims.Yend - dims.Ystart;
	int bands = GetDIMSBands(dims);
	matrix<double> *ppMatrix = new matrix<double>(bands,width*height); 
	int nums = pmgr->InitBlocks(dims);
	int onebks = nums/bands;
	int i,j,k;
	BYTE *pdata = NULL;
	int datasize;
	BYTE *b_data = NULL;
	unsigned short *s_data = NULL;
	float *f_data = NULL;
	double *d_data = NULL;
	for(i=0; i<bands;i++)
	{
		int tag = 0;
		for(j=i*onebks;j<(i+1)*onebks;j++)
		{
			pmgr->GetDIMSDataByBlock(dims,i,nums,&pdata,datasize);
			datasize /= PerPt;
			switch(PerPt)
			{
			case 1:
				b_data = pdata;
				for(k=0;k<datasize;k++)
				{
					(*ppMatrix)(i,tag+k) = (double)b_data[k]-mean[i];
				}
				tag += datasize;
				break;
			case 2:
				s_data = (unsigned short*)pdata;
				for(k=0;k<datasize;k++)
				{
					(*ppMatrix)(i,tag+k) = (double)s_data[k]-mean[i];
				}
				tag += datasize;
				break;
			case 4:
				f_data = (float*)pdata;
				for(k=0;k<datasize;k++)
				{
					(*ppMatrix)(i,tag+k) = (double)f_data[k]-mean[i];
				}
				tag += datasize;
				break;
			case 8:
				d_data = (double*)pdata;
				for(k=0;k<datasize;k++)
				{
					(*ppMatrix)(i,tag+k) = (double)d_data[k]-mean[i];
				}
				tag += datasize;
				break;
			}
			delete []pdata;
			pdata = NULL;
		}
	}
	return ppMatrix;
								
}
HpsCPCA::HpsCPCA(char * filename)
{
  	if(filename)
	{
		strcpy(m_sFileName,filename);
	}
	m_M = 1;
}

HpsCPCA::~HpsCPCA()
{

}

bool HpsCPCA::Process(DIMS &dims,HpsCFileManager *pmgr1,HpsCFileManager *pmgr2,CWnd *pwnd)
{

		int j;
		int bands = GetDIMSBands(dims);
		//生成头文件
		pmgr2->m_header = pmgr1->m_header;
		pmgr2->m_header.m_nSamples = dims.Xend - dims.Xstart;
		pmgr2->m_header.m_nLines = dims.Yend - dims.Ystart;
	
		int nCount =0;
			for (j=0; j<pmgr1->m_header.m_nBands;j++)
			{
				if(nCount == m_M)
					break;
				if (dims.m_pBand[j] ==1)
				{		
					//pmgr2->m_header.m_dWaveLength[nCount] = pmgr1->m_header.m_dWaveLength[j];
					pmgr2->m_header.m_dWaveLength[nCount] = nCount+1;
					nCount ++;
				}
				
			}
		
		//bands = MIN(bands,m_M);
		pmgr2->m_header.m_nBands = m_M;
		strcpy(pmgr2->m_header.m_cDescription ,"Hipas 生成的PCA图象");
		CTime t = CTime::GetCurrentTime(); //MFC
		CString s = t.Format( "%A, %B %d, %Y" );
		strcat(pmgr2->m_header.m_cDescription,s);
		strcpy(pmgr2->m_header.m_cDescription,pmgr1->m_header.m_cDescription);
		
		pmgr2->m_header.m_nDataType = DATATYPE;//pmgr1->m_header.m_nDataType;
	    //开始生成新的Manager
		BYTE *pp;
		bool IsMem = false;
		if(pmgr2->BoolIsImgInMem())
		{
			pmgr2->m_ImgInMem = new BYTE[pmgr2->m_header.m_nSamples*pmgr2->m_header.m_nLines*pmgr2->m_header.m_nBands*pmgr2->GetBytesPerPt()];
			IsMem = true;
			pp = (BYTE *)pmgr2->m_ImgInMem;   //内存图象
		}
		else
			pmgr2->HpsSaveImageAs(m_sFileName);
	
		//生成PCA图象
	
		
				//数据

		int PerPt = pmgr1->GetBytesPerPt();
		int nBytePPt = pmgr2->GetBytesPerPt();
		
		int N = pmgr1->m_header.m_nBands;
		double *mean = new double[N];
		double *max = new double[N];
		double *min = new double[N];

		matrix<double> *pM = NULL;
		matrix<double> *pVec = new matrix<double>(bands,bands);
		valarray<double> *pVecVal = new valarray<double>(bands);
		valarray<double> bb(bands);
		bool issucess =false;
	    switch(PerPt)
		{
			case 1:
			{	
				HpsCStatistics<byte> *stat = new HpsCStatistics<byte>();
				stat->m_Dims = dims;
				stat->m_nMgrID = m_nMgrID;
				stat->GetMaxMinMeanVal(max,min,mean,pwnd);
				pM = stat->GetCovariance(mean,pwnd);
				if(	stat->GetEigenVector(*pM,*pVec,*pVecVal,bb)>0)
				{	
					issucess = true;
				}
				delete stat;
				break;			
			}
			case 2:
			{	
				HpsCStatistics<unsigned short> *usstat = new HpsCStatistics<unsigned short>();
				usstat->m_Dims = dims;
				usstat->m_nMgrID = m_nMgrID;
				usstat->GetMaxMinMeanVal(max,min,mean,pwnd);
				pM = usstat->GetCovariance(mean,pwnd);
				if(	usstat->GetEigenVector(*pM,*pVec,*pVecVal,bb)>0)
				{
					issucess= true;
				}
				delete usstat;
				break;			
			}

			case 4:
			{	
				HpsCStatistics<float> *fstat = new HpsCStatistics<float>();
				fstat->m_Dims = dims;
				fstat->m_nMgrID = m_nMgrID;
				fstat->GetMaxMinMeanVal(max,min,mean,pwnd);
				pM = fstat->GetCovariance(mean,pwnd);
				if(	fstat->GetEigenVector(*pM,*pVec,*pVecVal,bb)>0)
				{
					issucess = true;
				}
				delete fstat;
				break;			
			}
			case 8:
			{	
				HpsCStatistics<double> *dstat = new HpsCStatistics<double>();
				dstat->m_Dims = dims;
				dstat->m_nMgrID = m_nMgrID;
				dstat->GetMaxMinMeanVal(max,min,mean,pwnd);	
				pM = dstat->GetCovariance(mean,pwnd);
				if(	dstat->GetEigenVector(*pM,*pVec,*pVecVal,bb)>0)
				{
					issucess = true;
				}
				delete dstat;
				break;			
			}
		}
		delete []max;
		max = NULL;
		delete []min;
		min = NULL;
	
		
		if(!issucess)
		{
			delete pM ;
			delete pVec;
			delete pVecVal ;
			delete []mean;
			return false;
		}
	CProgressDlg dlg;
	dlg.Create(pwnd);
	dlg.SetRange(0,100);
	dlg.SetWindowText("正在运行。。。");
	dlg.ShowWindow(SW_SHOW);
	
	dlg.OffsetPos(5);
		SortMatrix(pVec,pVecVal);
	dlg.OffsetPos(10);
		matrix<double> *pMatrix = GetNormMatrix(dims,pmgr1,mean);
		//double det= MatrixDeterminant(*pVec);
		matrix<double> pResult = (*pVec);///det;
	dlg.OffsetPos(50);
	
		pResult *= *pMatrix;             //得到Y = A*X'
	//	MatrixMultiply(*pResult,*pVec,*pMatrix);
//		matrix<double> *pVecTrans = new matrix<double>(bands,bands)	;
//		MatrixTranspose(*pVec,*pVecTrans);
//		delete pVec;
//		delete pVecVal;
//    dlg.OffsetPos(30);
//
//		MatrixMultiply(*pMatrix,*pVecTrans,pResult);  //X = A’*Y
//	dlg.OffsetPos(70);
//		//delete pResult;
	   
		//WriteIntoImage(*pMatrix,pmgr2,IsMem);   //只要前m个波段；
		WriteIntoImage(pResult,pmgr2,IsMem);   //只要前m个波段；
	dlg.OffsetPos(100);
	    pmgr2->Close();
		delete pMatrix;
	return true;
		
}
void HpsCPCA::WriteIntoImage(matrix<double> pVec,HpsCFileManager *pmgr,bool ismem)
{
	byte *pdata = NULL;
	unsigned short * s_data = NULL;
	float * f_data = NULL;
	double * d_data = NULL;

	int PerPt = pmgr->GetBytesPerPt();

	int samples =pmgr->m_header.m_nSamples;
	int lines = pmgr->m_header.m_nLines;
	int bands = pmgr->m_header.m_nBands;
	if (ismem)   //内存型
	{
		byte *pp = pmgr->m_ImgInMem;
		switch(PerPt)
		{
		case 1:
			{
				pdata	= (byte*)pp;			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						pdata[i*samples*lines + j] = (byte) (pVec(i,j));
				}
				break;
			}
		case 2:
			{
				s_data	= (unsigned short*)pp;			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						s_data[i*samples*lines + j] = (unsigned short) (pVec(i,j));
				}
				break;
			}

		case 4:
			{
				f_data	= (float*)pp;			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						f_data[i*samples*lines + j] = (float) (pVec(i,j));
				}
				break;
			}

		case 8:
			{
				d_data	= (double*)pp;			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						d_data[i*samples*lines + j] = (double) (pVec(i,j));
				}
				break;
			}


		}

	}
	else         //硬盘文件
	{

		switch(PerPt)
		{
		case 1:
			{
				pdata	= new byte[samples*lines];			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						pdata[j] = (byte) (pVec(i,j));

					pmgr->WriteBlock(pdata,samples*lines);
				}
				delete []pdata;
				pdata = NULL;
				break;
			}
		case 2:
			{
				s_data	= new unsigned short[samples*lines];			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						s_data[j] = (unsigned short) (pVec(i,j));

					pmgr->WriteBlock((byte *)s_data,samples*lines*2);
				}
				delete []s_data;
				s_data = NULL;
				break;
			}

		case 4:
			{
				f_data	= new float[samples*lines];			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						f_data[ j] = (float) (pVec(i,j));

					pmgr->WriteBlock((byte *)f_data,samples*lines*4);
				}
				delete []f_data;
				f_data = NULL;
				break;
			}

		case 8:
			{
				d_data	= new double[samples*lines];			
				for(int i=0; i<bands; i++)
				{
					for(int j=0; j<samples*lines;j++)
						d_data[j] = (double) (pVec(i,j));

					pmgr->WriteBlock((byte *)d_data,samples*lines*8);
				}
				delete []d_data;
				d_data = NULL;
				break;
			}
		}

	}

}

void HpsCPCA::SortMatrix(matrix<double> *pVec,valarray<double> *pVecVal)
{	
	int row = pVec->GetRowNum();
	int col = pVec->GetColNum();
    matrix<double> tmpVec(row,col);
	matrix<double> pVecTrans(col,row);
	tmpVec = *pVec;
	

	double *pdata = new double[row];
	
	for(int i =0; i<row; i++)
		pdata[i] = (*pVecVal)[i];

	SortData(pdata,row,false);
/*	CString str;
	for(i =0; i<row; i++)
	{
		CString str1;
		str1.Format("%f ", pdata[i]);
		str = str +str1;
	}
	AfxMessageBox(str);  */

	int j;
	for(i=0;i<row;i++)
	{
		for(j=0; j<row; j++)
		{
			if (pdata[i] == (*pVecVal)[j])
			{
				for(int m=0; m<col;m++)
				{
					(*pVec)(m,i) =tmpVec(m,j);
				}
			}

		}
	}
	//归一化
/*
		for(i=0; i<row;i++ )
		{
			double vec=0;
			for(j=0;j<col; j++)
			{
				vec = vec+(*pVec)(i,j)*(*pVec)(i,j);
			}
			vec= sqrt(vec);
			for(j=0;j<col; j++)
			{		
				(*pVec)(i,j)= (*pVec)(i,j)/vec;
				if((i==0)||(i==1)||(i==4)||(i==5))
					(*pVec)(i,j) = ((*pVec)(i,j))*(-1);
			}
		}
*/
	
/*	str="";

		for(j=0; j<col;j++ )
		{
			for(i=0;i<row; i++)
			{
				CString str2;
				str2.Format("%8.3f  ",(*pVec)(i,j));
				str= str+str2;
			}
			str = str +"\r\n";
		}
		AfxMessageBox(str);*/
	MatrixTranspose(*pVec,pVecTrans);
	*pVec = pVecTrans;
	delete []pdata;
	pdata = NULL;
}

