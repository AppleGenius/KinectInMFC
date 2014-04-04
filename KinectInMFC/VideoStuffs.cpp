#include "OpencvProc.h"

void OpencvProc::ConvertUStoUC(Mat& origMat, Mat& outMat)
{
	CV_Assert(CV_16UC1 == origMat.type());
	if (CV_8UC3 != outMat.type())
		outMat.create(origMat.size(),CV_8UC3);

	int nRows = origMat.rows;
	int nCols = origMat.cols;
	if (origMat.isContinuous() && outMat.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	uchar* puc = NULL;
	ushort* pus = NULL;
	uchar lowEight = 0;
	uchar highEight = 0;

	for (int i = 0; i < nRows; ++i)
	{
		pus = origMat.ptr<ushort>(i);
		puc = outMat.ptr<uchar>(i);
		for (int j = 0; j < nCols; ++j)
		{
			lowEight = (uchar)(pus[j] & 0xff);
			highEight = (uchar)((pus[j] >> 8) & 0xff);
			puc[j*3] = highEight;
			puc[j*3 + 1] = lowEight;
			puc[j*3 + 2] = 255;
		}
	}
}

void OpencvProc::ConvertUCtoUS(Mat& origMat, Mat& outMat)
{
	CV_Assert(CV_8UC3 == origMat.type());
	if (CV_16UC1 != outMat.type())
		outMat.create(origMat.size(),CV_16UC1);

	int nRows = origMat.rows;
	int nCols = origMat.cols;
	if (origMat.isContinuous() && outMat.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	uchar* puc = NULL;
	ushort* pus = NULL;
	ushort s;
	for (int i = 0; i<nRows; ++i)
	{
		puc = origMat.ptr<uchar>(i);
		pus = outMat.ptr<ushort>(i);
		for (int j = 0; j < nCols; ++j)
		{
			s = (puc[j*3]<<8) + puc[j*3+1];
			pus[j] = s;
		}
	}

}