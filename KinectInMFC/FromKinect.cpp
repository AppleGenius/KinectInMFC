#include "stdafx.h" 
#include "FromKinect.h"

KinectStreams FromKinect::allFrames = KinectStreams();
HANDLE FromKinect::allFramesMutex = CreateMutex(NULL,FALSE,NULL);

BOOL FromKinect:: colorStreamOpened = FALSE;	
BOOL FromKinect:: depthStreamOpened = FALSE;
BOOL FromKinect:: skeletonEnabled = FALSE;

HANDLE FromKinect::nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
HANDLE FromKinect::colorStreamHandle = NULL;
HANDLE FromKinect::nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
HANDLE FromKinect::depthStreamHandle = NULL;
HANDLE FromKinect::skeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

HRESULT FromKinect::OpenKinect(BOOL useColor, BOOL useDepth, BOOL useSkeleton,
							   NUI_IMAGE_RESOLUTION colorFrameRes, NUI_IMAGE_RESOLUTION depthFrameRes)
{
	//HRESULT hr = NuiStatus();
//  	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH|
//  		NUI_INITIALIZE_FLAG_USES_SKELETON);
 	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH |
 		NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);
	if (FAILED(hr))
		return hr;
	//check if the resolution valid
	if (NUI_IMAGE_RESOLUTION_1280x960 != colorFrameRes && NUI_IMAGE_RESOLUTION_640x480 != colorFrameRes)
		return E_INVALIDARG;
	if (NUI_IMAGE_RESOLUTION_1280x960 == depthFrameRes)
		return E_INVALIDARG;

	//initialize the color and depth frame resolution 
	if (NUI_IMAGE_RESOLUTION_1280x960 == colorFrameRes)
		allFrames.colorFrame.create(960,1280,CV_8UC3);
	else
		allFrames.colorFrame.create(480,640,CV_8UC3);

	if (NUI_IMAGE_RESOLUTION_640x480 == depthFrameRes)
		allFrames.depthFrame.create(480,640,CV_16UC1);
	else if(NUI_IMAGE_RESOLUTION_320x240 == depthFrameRes)
		allFrames.depthFrame.create(240,320,CV_16UC1);
	else
		allFrames.depthFrame.create(60,80,CV_16UC1);

	allFrames.skeletonFrame.create(240,320,CV_16UC1);

	//open color frames
	if (useColor)
	{
		if (!colorStreamOpened)		//color frame already opened
		{
			HRESULT hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, colorFrameRes,
				0,2, nextColorFrameEvent, &colorStreamHandle); 
			if (FAILED(hr))
				return hr;
			else
				colorStreamOpened = TRUE;
		}
	}

	//open depth frame
	if (useDepth)
	{
		if (!depthStreamOpened)
		{
  			/*HRESULT hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, depthFrameRes, 
  				NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE, 2, nextDepthFrameEvent, &depthStreamHandle); */
 			HRESULT hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, depthFrameRes, 
  				0, 2, nextDepthFrameEvent, &depthStreamHandle); 
			if (FAILED(hr))
				return hr;
			else
				depthStreamOpened = TRUE;
		}
	}

	//open skeleton frame
	if (useSkeleton)
	{
		if (FALSE == skeletonEnabled)
		{
			HRESULT hr = NuiSkeletonTrackingEnable(skeletonEvent,0);
			if (FAILED(hr))
				return hr;
			else
				skeletonEnabled = TRUE;
		}
	}
	return S_OK;
}

void FromKinect:: CloseKinect()
{
	if (TRUE == skeletonEnabled)
	{
		NuiSkeletonTrackingDisable();
		skeletonEnabled = FALSE;
	}
	if (TRUE == colorStreamOpened)
		colorStreamOpened = FALSE;
	if (TRUE == depthStreamOpened)
		depthStreamOpened = FALSE;
	NuiShutdown();
}

DWORD WINAPI FromKinect:: ProcessThread(LPVOID lpParam)
{
	BOOL continueUpdate = TRUE;
	//loop to update frames
	while (continueUpdate)
	{
		WaitForSingleObject(allFramesMutex,INFINITE);
		Sleep(33);
		//begin to read color frame
		if (colorStreamOpened)
		{
			const NUI_IMAGE_FRAME* pColorImageFrame = NULL;
			//wait until getting the frame data
			WaitForSingleObject(nextColorFrameEvent,INFINITE);
			//read the frame data in the frame handle to the pointer
			HRESULT hr = NuiImageStreamGetNextFrame(colorStreamHandle,0,&pColorImageFrame);
			if (SUCCEEDED(hr))
			{
				INuiFrameTexture* pColorTexture = pColorImageFrame->pFrameTexture;
				NUI_LOCKED_RECT LockedRect;

				//lock the data
				pColorTexture->LockRect(0, &LockedRect, NULL, 0); 
				if (LockedRect.Pitch != 0)		//make sure the data valid
				{
					//copy the data into mat
					for (int i=0; i<allFrames.colorFrame.rows; i++) 
					{
						uchar *ptr = allFrames.colorFrame.ptr<uchar>(i);  
						uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
						for (int j=0; j<allFrames.colorFrame.cols; j++) 
						{ 
							ptr[3*j] = pBuffer[4*j];  
							ptr[3*j+1] = pBuffer[4*j+1]; 
							ptr[3*j+2] = pBuffer[4*j+2]; 
						} 
					} 
				}
				pColorTexture->UnlockRect(0);
				NuiImageStreamReleaseFrame(colorStreamHandle, pColorImageFrame ); 
			}
		}

		//begin to read depth frame
		if (depthStreamOpened)
		{
			const NUI_IMAGE_FRAME* pDepthImageFrame = NULL;
			//wait until getting the frame data
			WaitForSingleObject(nextDepthFrameEvent,INFINITE);
			//read the frame data in the frame handle to the pointer
			HRESULT hr = NuiImageStreamGetNextFrame(depthStreamHandle,0,&pDepthImageFrame);
			if (SUCCEEDED(hr))
			{
				INuiFrameTexture* pDepthTexture = pDepthImageFrame->pFrameTexture;
				NUI_LOCKED_RECT LockedRect;

				//lock the data
				pDepthTexture->LockRect(0, &LockedRect, NULL, 0); 
				if (LockedRect.Pitch != 0)		//make sure the data valid
				{
					//copy the data into mat
					for (int rowIndex = 0; rowIndex < allFrames.depthFrame.rows; ++rowIndex)
					{
						ushort* ptr = allFrames.depthFrame.ptr<ushort>(rowIndex);
						uchar *pBufferRun = (uchar*)(LockedRect.pBits) + rowIndex * LockedRect.Pitch;
						USHORT * pBuffer = (USHORT*)pBufferRun;
						for (int cIndex = 0; cIndex < allFrames.depthFrame.cols; ++cIndex)
							ptr[cIndex] = pBuffer[cIndex];
					}
				}
				pDepthTexture->UnlockRect(0);
				NuiImageStreamReleaseFrame(depthStreamHandle, pDepthImageFrame ); 
			}
		}

		//begin to update skeleton frame
		if (skeletonEnabled)
		{
			//begin to read joint coordinates
			if (!allFrames.jc.empty())
				allFrames.jc.clear();
			NUI_SKELETON_FRAME skeletonFrame;	//define the skeleton frame
			BOOL bFoundSkeleton = FALSE;		//whether find a skeleton

			WaitForSingleObject(skeletonEvent,INFINITE);	//wait for skeleton data
			HRESULT hr = NuiSkeletonGetNextFrame(0,&skeletonFrame);	//retrieve the data to skeleton frame
			if (SUCCEEDED(hr))
			{
				int nearestPersonIndex = -1;
				float nearestDist = 4, tmpDist = 0;
				for (int i = 0 ; i < NUI_SKELETON_COUNT ; ++i)		//check the tracking state, find the nearest person
				{
					tmpDist = skeletonFrame.SkeletonData[i].Position.z;
					if (nearestDist > tmpDist && tmpDist != 0)		//find the nearest person
					{
						nearestDist = tmpDist;
						nearestPersonIndex = i;
					}

					NUI_SKELETON_TRACKING_STATE trackingState = 
						skeletonFrame.SkeletonData[i].eTrackingState;	
					if (NUI_SKELETON_TRACKED == trackingState)
						bFoundSkeleton = TRUE;
				}
				if (FALSE == bFoundSkeleton)
				{
					ReleaseMutex(allFramesMutex);
					continue;
				}

 				NuiTransformSmooth(&skeletonFrame,NULL);		//smooth the skeleton

				//skeleton valid only if the skeleton tracked and the shoulder center tracked
				if (NUI_SKELETON_TRACKED == skeletonFrame.SkeletonData[nearestPersonIndex].eTrackingState &&
					NUI_SKELETON_POSITION_NOT_TRACKED != skeletonFrame.SkeletonData[nearestPersonIndex].
					eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER])
				{
					for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
						allFrames.jc.push_back(skeletonFrame.SkeletonData[nearestPersonIndex].SkeletonPositions[i]);
				}
				else
				{
					ReleaseMutex(allFramesMutex);
					continue;
				}
			}

			//draw the skeleton image
			//transform the joints coordinates to depth image coordinate
			allFrames.skeletonFrame = Mat(240,320,CV_8UC3,Scalar::all(255));
			std::vector<cv::Point> pointSet;
			float x = 0, y = 0;

			for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
				//		for (int i = 0; i < pointSet.size(); ++i)
			{
				NuiTransformSkeletonToDepthImage(allFrames.jc[i], &x, &y);
				pointSet.push_back(cv::Point((int)x,(int)y));							
				//circle(allFrames.skeletonFrame, pointSet[i], 3, cvScalar(255, 0, 0), 1, 8, 0); 			 
			}

			//
			for(int i = 0; i < NUI_SKELETON_POSITION_COUNT - 8; ++i)
				circle(allFrames.skeletonFrame, pointSet[i], 3, cvScalar(255, 0, 0), 1, 8, 0);

			//
			Scalar color = cv::Scalar(0,255,0);
			//draw the joints
			if((pointSet[NUI_SKELETON_POSITION_HEAD].x!=0 || pointSet[NUI_SKELETON_POSITION_HEAD].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_HEAD], pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_SPINE].x!=0 || pointSet[NUI_SKELETON_POSITION_SPINE].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SPINE], color, 2); 
 			if((pointSet[NUI_SKELETON_POSITION_SPINE].x!=0 || pointSet[NUI_SKELETON_POSITION_SPINE].y!=0) && 
 				(pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SPINE], pointSet[NUI_SKELETON_POSITION_HIP_CENTER], color, 2); 

			//×óÉÏÖ« 
			if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_HAND_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HAND_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], pointSet[NUI_SKELETON_POSITION_HAND_LEFT], color, 2); 

			//ÓÒÉÏÖ« 
			if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], pointSet[NUI_SKELETON_POSITION_HAND_RIGHT], color, 2); 

			//×óÏÂÖ« 
			if((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_HIP_LEFT], pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y!=0) &&  
				(pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], pointSet[NUI_SKELETON_POSITION_FOOT_LEFT], color, 2); 

			//ÓÒÏÂÖ« 
			if((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT],color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], color, 2); 
			if((pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y!=0) && 
				(pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].y!=0)) 
				line(allFrames.skeletonFrame, pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT], color, 2); 
		}
		ReleaseMutex(allFramesMutex);
	}
	return 0;
}

void FromKinect::ShowAll()
{
	if (colorStreamOpened)
		namedWindow("color image");
	if (depthStreamOpened)
		namedWindow("depth image");
	if (skeletonEnabled)
		namedWindow("skeleton image");

	while (TRUE)
	{
		WaitForSingleObject(allFramesMutex,INFINITE);
		if (colorStreamOpened)
			if (!allFrames.colorFrame.empty())
				imshow("color image",allFrames.colorFrame);
		if (depthStreamOpened)
			if (!allFrames.depthFrame.empty())
				imshow("depth image",allFrames.depthFrame);
		if (skeletonEnabled)
			if (!allFrames.skeletonFrame.empty())
				imshow("skeleton image",allFrames.skeletonFrame);
		ReleaseMutex(allFramesMutex);
		int c = waitKey(100);
		if (27 == c)
			break;
	}
}

void FromKinect::GetAllFrames(KinectStreams& ks)
{
	WaitForSingleObject(allFramesMutex,INFINITE);
	ks.colorFrame = allFrames.colorFrame.clone();
	ks.depthFrame = allFrames.depthFrame.clone();
	ks.skeletonFrame = allFrames.skeletonFrame.clone();
	ks.jc = allFrames.jc;
	ReleaseMutex(allFramesMutex);
}

void FromKinect::ConvertUStoUC(Mat& origMat, Mat& outMat)
{
	CV_Assert(CV_16UC1 == origMat.type());
	if (CV_8UC3 != outMat.type())
		outMat.create(origMat.size(), CV_8UC3);

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
			puc[j * 3] = highEight;
			puc[j * 3 + 1] = lowEight;
			puc[j * 3 + 2] = 255;
		}
	}
}

void FromKinect::ConvertUCtoUS(Mat& origMat, Mat& outMat)
{
	CV_Assert(CV_8UC3 == origMat.type());
	if (CV_16UC1 != outMat.type())
		outMat.create(origMat.size(), CV_16UC1);

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
	for (int i = 0; i < nRows; ++i)
	{
		puc = origMat.ptr<uchar>(i);
		pus = outMat.ptr<ushort>(i);
		for (int j = 0; j < nCols; ++j)
		{
			s = (puc[j * 3] << 8) + puc[j * 3 + 1];
			pus[j] = s;
		}
	}

}