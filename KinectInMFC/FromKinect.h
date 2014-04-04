#ifndef _FROMKINECT_H
#define _FROMKINECT_H
#include <windows.h>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <NuiApi.h>
#include <cassert>
using namespace cv;

typedef std::vector<Vector4> jointsCoord;

typedef struct _KinectStreams 
{
	Mat colorFrame;
	Mat depthFrame;
	Mat skeletonFrame;
	jointsCoord jc;
} KinectStreams;

class FromKinect
{
public:
	//initialize the kinect, return true if successful
	static HRESULT OpenKinect(BOOL useColor, BOOL useDepth, BOOL useSkeleton,
		NUI_IMAGE_RESOLUTION colorFrameRes, NUI_IMAGE_RESOLUTION depthFrameRes);

	//close the kinect, return true if successful
	static void CloseKinect();

	//callback function to update all frames
	static DWORD WINAPI ProcessThread(LPVOID lpParam);

	//show the color frame, depth frame and skeleton frame simultaneously
	static void ShowAll();

	static void GetAllFrames(KinectStreams& ks);

	//convert the image mat type from cv_16usc1 to cv_8uc3
	//the 3rd channel value is set to 255
	static void ConvertUStoUC(Mat& origMat, Mat& outMat);

	//convert the image mat type from cv_8uc3 to cv_16usc1
	static void ConvertUCtoUS(Mat& origMat, Mat& outMat);

private:
	static KinectStreams allFrames;
	static HANDLE allFramesMutex;

	static BOOL colorStreamOpened;
	static BOOL depthStreamOpened;
	static BOOL skeletonEnabled;

	static HANDLE nextColorFrameEvent;
	static HANDLE colorStreamHandle;
	static HANDLE nextDepthFrameEvent;
	static HANDLE depthStreamHandle;
	static HANDLE skeletonEvent;
};

#endif