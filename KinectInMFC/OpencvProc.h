#ifndef OPENCVPROC_H
#define OPENCVPROC_H

#include "stdafx.h"
#include <opencv2/opencv.hpp>
using namespace cv;

namespace OpencvProc
{
	//convert the image mat type from cv_16usc1 to cv_8uc3
	//the 3rd channel value is set to 255
	void ConvertUStoUC(Mat& origMat, Mat& outMat);

	//convert the image mat type from cv_8uc3 to cv_16usc1
	void ConvertUCtoUS(Mat& origMat, Mat& outMat);
}

#endif