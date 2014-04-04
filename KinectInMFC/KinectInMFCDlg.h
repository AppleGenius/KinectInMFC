
// KinectInMFCDlg.h : header file
//

#pragma once

//#include "OpencvProc.h"
#include "FromKinect.h"
#include "CvvImage.h"
using namespace cv;

// CKinectInMFCDlg dialog
class CKinectInMFCDlg : public CDialogEx
{
// Construction
public:
	CKinectInMFCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_KINECTINMFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	BOOL useColorStream;
	BOOL useDepthStream;
	BOOL useSkeletonStream;

	bool kinectOpened;
	HANDLE htUpdateKinectFrames;		//handle of thread to update kinect streams

	CString savePath;		//path to save the photos and videos
	VideoWriter colorVW;
	VideoWriter depthVW;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	void ShowImage(Mat& img, UINT ID);

	CString GetSaveColorPhotoPath();
	CString GetSaveColorVideoPath();
	CString GetSaveDepthPhotoPath();
	CString GetSaveDepthVideoPath();
	CString GetSaveFileName();
	CString GetTime();
	void FormatAudioCaption(CString& itemCaption);
	int GetSelectedID(int sID, int eID);
public:
	afx_msg void OnBnClickedBtnOpenkinect();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedBtnSnap();
	afx_msg void OnBnClickedBtnRecord();
	afx_msg void OnBnClickedBtnRcdFrm();
};
