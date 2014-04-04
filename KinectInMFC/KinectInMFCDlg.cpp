
// KinectInMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KinectInMFC.h"
#include "KinectInMFCDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CKinectInMFCDlg dialog



CKinectInMFCDlg::CKinectInMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKinectInMFCDlg::IDD, pParent)
	, useColorStream(FALSE)
	, useDepthStream(FALSE)
	, useSkeletonStream(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	kinectOpened = false;
	savePath = "D:\\Kinect";
}

void CKinectInMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_USECOLORSTREAM, useColorStream);
	DDX_Check(pDX, IDC_USEDEPTHSTRAM, useDepthStream);
	DDX_Check(pDX, IDC_USESKELETONSTREAM, useSkeletonStream);
}

BEGIN_MESSAGE_MAP(CKinectInMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_KINECT_CONTROL, &CKinectInMFCDlg::OnBnClickedBtnOpenkinect)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_SNAP, &CKinectInMFCDlg::OnBnClickedBtnSnap)
	ON_BN_CLICKED(IDC_BTN_RECORD, &CKinectInMFCDlg::OnBnClickedBtnRecord)
END_MESSAGE_MAP()


// CKinectInMFCDlg message handlers

BOOL CKinectInMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CKinectInMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKinectInMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		CDialogEx::UpdateWindow();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKinectInMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKinectInMFCDlg::OnBnClickedBtnOpenkinect()
{
	// TODO: Add your control notification handler code here

	//open kinect
	UpdateData(TRUE);
	if (!kinectOpened)
	{
		if (false == useColorStream && false == useDepthStream && false == useSkeletonStream)
		{
			AfxMessageBox("please select at least one stream");
			return;
		}
		HRESULT hr = FromKinect::OpenKinect(useColorStream,useDepthStream,useSkeletonStream,
			NUI_IMAGE_RESOLUTION_640x480,NUI_IMAGE_RESOLUTION_640x480);
		if (FAILED(hr))
		{
			MessageBox("Open Kinect Failed!");
			return;
		}
		kinectOpened = true;
	}
	
	if (!kinectOpened)
	{
		MessageBox("Please open the kinect first");
		return;
	}

	//automatically change the caption of the button
	CString btnCaptin;
	GetDlgItemText(IDC_BTN_KINECT_CONTROL,btnCaptin);
	if ("Open Kinect" == btnCaptin)
	{
		SetDlgItemText(IDC_BTN_KINECT_CONTROL,"Close Kinect");
		htUpdateKinectFrames = CreateThread(NULL,0,FromKinect::ProcessThread,NULL,0,NULL);		//run the update frame thread
		if (FAILED(htUpdateKinectFrames))
		{
			MessageBox("create thread failed");
			return;
		}
		SetTimer(1,33,NULL);		//timer to flush picture controls
	}
	else
	{
		SetDlgItemText(IDC_BTN_KINECT_CONTROL,"Open Kinect");
		DWORD dw = 0;
		BOOL bTermThread = TerminateThread(htUpdateKinectFrames,dw);
		if (FALSE == bTermThread)
		{
			MessageBox("terminate thread failed");
			return;
		}

		FromKinect::CloseKinect();
		kinectOpened = FALSE;
		KillTimer(1);
	}
}

void CKinectInMFCDlg:: ShowImage(Mat& img, UINT ID)
{
	if (img.empty())
		return;

// 	if (img.rows!=240 && img.cols != 320)
// 		resize(img,img,Size(320,240));

	CDC* pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();

	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	IplImage iImg = img;
	CvvImage cImg;
	cImg.CopyOf(&iImg);
	cImg.DrawToHDC(hDC,&rect);

	ReleaseDC(pDC);
}

void CKinectInMFCDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	KinectStreams ks;
	FromKinect::GetAllFrames(ks);

	switch (nIDEvent)
	{
	case 1:		//flush picture control
		if (kinectOpened)
		{
			if (useColorStream)
				ShowImage(ks.colorFrame,IDC_PICTURE_COLOR);
			if (useDepthStream)
				ShowImage(ks.depthFrame,IDC_PICTURE_DEPTH);
			if (useSkeletonStream)
				ShowImage(ks.skeletonFrame,IDC_PICTURE_SKELETON);
		}
		break;
	case 2:		//write the video
		{
			if (!kinectOpened)
			{
				MessageBox("open kinect first");
				KillTimer(2);
				return;
			}

			Mat depthImg;
			FromKinect::ConvertUStoUC(ks.depthFrame,depthImg);
			colorVW<<ks.colorFrame;
			depthVW<<depthImg;
			break;

		}		
	case 3:		//save the frame sequence
		{
			if (!kinectOpened)
			{
				MessageBox("open kinect first");
				KillTimer(2);
				return;
			}

			CString saveColorPath = GetSaveColorPhotoPath();
			CString saveDepthPath = GetSaveDepthPhotoPath();
			imwrite(saveColorPath.GetString(),ks.colorFrame);
			imwrite(saveDepthPath.GetString(),ks.depthFrame);
			break;
		}

	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

int CKinectInMFCDlg::GetSelectedID(int sID, int eID)
{
	for (int i=sID;i<=eID;++i){
		if(IsDlgButtonChecked(i))
			return i;
	}
	return -1;
}

CString CKinectInMFCDlg::GetSaveFileName()
{
	CString fileName;
	CString cap;
	for(int id=IDC_RADIO_CORRECT;id<=IDC_RADIO_BLACK;++id)
	{
		if (IsDlgButtonChecked(id))
		{
			GetDlgItemText(id,cap);
			FormatAudioCaption(cap);
			fileName = fileName + "_" + cap;
		}
	}
	CString height;
	GetDlgItemText(IDC_EDIT_HEIGHT,height);
	fileName = fileName + "_" + height;
	CString tm = GetTime();
	fileName = fileName + "_" + tm;
	fileName = fileName.Right(fileName.GetLength()-1);

	if (28 != fileName.GetLength())
		return CString();
	else
		return fileName;
}

CString CKinectInMFCDlg::GetTime()
{
	SYSTEMTIME st;
	CString time;
	GetLocalTime(&st);
	time.Format("%4d%2d%2d%2d%2d%2d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	time.Replace(' ','0');
	return time;
}

void CKinectInMFCDlg::FormatAudioCaption(CString& itemCaption)
{
	if(itemCaption == "Correct")
		itemCaption = "T";
	else if(itemCaption == "Wrong")
		itemCaption = "F";
	else if(itemCaption == "Left")
		itemCaption = "L";
	else if(itemCaption == "Right")
		itemCaption = "R";
	else if(itemCaption == "Not Set")
		itemCaption = "N";
	else if(itemCaption == "Male")
		itemCaption = "M";
	else if(itemCaption == "Female")
		itemCaption = "F";
	else if(itemCaption == "Fat")
		itemCaption = "F";
	else if(itemCaption == "Normal")
		itemCaption = "N";
	else if(itemCaption == "Thin")
		itemCaption = "T";
	else if(itemCaption == "White")
		itemCaption = "W";
	else if(itemCaption == "Normal")
		itemCaption = "N";
	else if(itemCaption == "Black")
		itemCaption = "B";
	else{
		AfxMessageBox("Radio button error");
		return ;
	}
	return;
}

CString CKinectInMFCDlg::GetSaveColorPhotoPath()
{
	//get the folder path
	CString folder = savePath;
	folder += "\\photo";
	CString posture;
	GetDlgItemText(IDC_COMBO_POSTURE,posture);
	if(posture.IsEmpty()){
		AfxMessageBox("Please input the posture!");
		return CString();
	}
	folder = folder + "\\" + posture;
	int sel = GetSelectedID(IDC_RADIO_CORRECT,IDC_RADIO_WRONG);
	CString cap;
	GetDlgItemText(sel,cap);
	folder = folder + "\\" + cap;

	//get file name
	CString fileName = GetSaveFileName();
	if (fileName.IsEmpty())
		return CString();
	CString savePhotoPath = folder + "\\C_" + fileName + ".png";
	return savePhotoPath;
}

CString CKinectInMFCDlg::GetSaveColorVideoPath()
{
	//get the folder path
	CString folder = savePath;
	folder += "\\video";
	CString posture;
	GetDlgItemText(IDC_COMBO_POSTURE,posture);
	if(posture.IsEmpty()){
		AfxMessageBox("Please input the posture!");
		return CString();
	}
	folder = folder + "\\" + posture;
	
	//get file name
	CString fileName = GetSaveFileName();
	if (fileName.IsEmpty())
		return CString();
	CString saveVideoPath = folder + "\\C_" + fileName + ".avi";
	return saveVideoPath;
}

CString CKinectInMFCDlg::GetSaveDepthPhotoPath()
{
	//get the folder path
	CString folder = savePath;
	folder += "\\photo";
	CString posture;
	GetDlgItemText(IDC_COMBO_POSTURE,posture);
	if(posture.IsEmpty()){
		AfxMessageBox("Please input the posture!");
		return CString();
	}
	folder = folder + "\\" + posture;
	int sel = GetSelectedID(IDC_RADIO_CORRECT,IDC_RADIO_WRONG);
	CString cap;
	GetDlgItemText(sel,cap);
	folder = folder + "\\" + cap;

	//get file name
	CString fileName = GetSaveFileName();
	if (fileName.IsEmpty())
		return CString();
	CString savePhotoPath = folder + "\\D_" + fileName+ ".png";
	return savePhotoPath;
}

CString CKinectInMFCDlg::GetSaveDepthVideoPath()
{
	//get the folder path
	CString folder = savePath;
	folder += "\\video";
	CString posture;
	GetDlgItemText(IDC_COMBO_POSTURE,posture);
	if(posture.IsEmpty()){
		AfxMessageBox("Please input the posture!");
		return CString();
	}
	folder = folder + "\\" + posture;

	//get file name
	CString fileName = GetSaveFileName();
	if (fileName.IsEmpty())
		return CString();
	CString saveVideoPath = folder + "\\D_" + fileName + ".avi";
	return saveVideoPath;
}

void CKinectInMFCDlg::OnBnClickedBtnSnap()
{
	// TODO: Add your control notification handler code here
	if (!kinectOpened)
	{
		MessageBox("open the kinect first");
		return;
	}

	CString colorPhotoPath = GetSaveColorPhotoPath();
	CString depthPhotoPath = GetSaveDepthPhotoPath();
	if (colorPhotoPath.IsEmpty() || depthPhotoPath.IsEmpty())
	{
		MessageBox("file name or path is invalid");
		return;
	}

	CString btnCap;
	GetDlgItemText(IDC_BTN_SNAP,btnCap);
	if ("Start Sequence" == btnCap)
	{
		SetTimer(3,1000,NULL);
		SetDlgItemText(IDC_BTN_SNAP,"Stop Sequence");
	}
	else
	{
		KillTimer(3);
		SetDlgItemText(IDC_BTN_SNAP,"Start Sequence");
	}


// 	KinectStreams ks;
// 	FromKinect::GetAllFrames(ks);
// 
// 	imwrite(colorPhotoPath.GetString(),ks.colorFrame);
// 	imwrite(depthPhotoPath.GetString(),ks.depthFrame);
}

void CKinectInMFCDlg::OnBnClickedBtnRecord()
{
	// TODO: Add your control notification handler code here
	if (!kinectOpened)
	{
		MessageBox("open the kinect first");
		return;
	}

	CString colorVideoPath = GetSaveColorVideoPath();
	CString depthVideoPath = GetSaveDepthVideoPath();

	if (colorVideoPath.IsEmpty() && depthVideoPath.IsEmpty())
	{
		MessageBox("video name or saving path is invalid");
		return;
	}

	CString caption;
	GetDlgItemText(IDC_BTN_RECORD,caption);
	VideoWriter vw;

	if ("Start Recording Video" == caption)
	{
		colorVW = VideoWriter((LPSTR)(LPCSTR)colorVideoPath,CV_FOURCC('M','J','P','G'),30,Size(640,480));
		depthVW = VideoWriter((LPSTR)(LPCSTR)depthVideoPath,CV_FOURCC('M','J','P','G'),30,Size(640,480));
		SetTimer(2,33,NULL);
		SetDlgItemText(IDC_BTN_RECORD,"Stop Recording Video");
	}
	else
	{
		KillTimer(2);
		colorVW.release();
		depthVW.release();
		SetDlgItemText(IDC_BTN_RECORD,"Start Recording Video");
	}
}
