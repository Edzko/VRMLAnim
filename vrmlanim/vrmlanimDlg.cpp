
// vrmlanimDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "vrmlanim.h"
#include "vrmlanimDlg.h"
#include "afxdialogex.h"
#include "CortonaEvents.h"
#include "lib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static Matrix Data(3, 1), QData(4, 1);
CAnimSocket pVRMLSock, pAnimSock;
CvrmlanimDlg *AnimDlg;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CvrmlanimDlg dialog



CvrmlanimDlg::CvrmlanimDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VRMLANIM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CvrmlanimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CvrmlanimDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_VRML1, OnSimulink)
	ON_MESSAGE(WM_VRML2, OnData)
END_MESSAGE_MAP()


// CvrmlanimDlg message handlers

BOOL CvrmlanimDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	m_CortonaCtl = (CCortonaCtl*)GetDlgItem(IDC_CORTONA);
	m_CortonaCtl->GetControlUnknown()->QueryInterface(IID_ICortona, reinterpret_cast<void**>(&m_pCortona));

	m_pCortonaEvents = new CCortonaEvents;
	AfxConnectionAdvise(m_CortonaCtl->GetControlUnknown(), DIID__CortonaEvents,
		m_pCortonaEvents->GetIDispatch(FALSE), FALSE, &m_dwCoockie);

	m_CortonaCtl->SetScene(fname);
	m_CortonaCtl->SetHeadLight(TRUE);

	m_CortonaCtl->SetNavigationBar(1);
	//m_CortonaCtl->Edit();  // mandatory in case we want to edit the scene programmatically

	IDispatch* pdisp = m_CortonaCtl->GetEngine();
	if (pdisp != NULL) {
		if (FAILED(pdisp->QueryInterface(IID_IEngine, reinterpret_cast<void**>(&m_pEngine))))
			m_pEngine = NULL;
		IServiceProvider* pServiceProvider;
		if (SUCCEEDED(m_pEngine->QueryInterface(IID_IServiceProvider,
			reinterpret_cast<void**>(&pServiceProvider))))
		{
			pServiceProvider->QueryService(IID_I3DViewService2, IID_I3DViewService2,
				reinterpret_cast<void**>(&m_p3DViewService));
			pServiceProvider->Release();
		}
		pdisp->Release();
	}

	ScanWRL(fname);

	// Initialize node operations buffer
	for (int i = 0; i < 100; i++)
		BUFFER[i].Node = 0;
	BufferIndex = 0;
	lastwrite.dwLowDateTime = 0;  // invalidate reload
	
	startAnimTime = CTime::GetCurrentTime();

	pVRMLSock.Initialized = false;
	long nResult = pVRMLSock.Create(50100, SOCK_STREAM, FD_READ | FD_CONNECT | FD_ACCEPT);
	nResult = pVRMLSock.Listen();
	TCPConnected = false;

	AnimDlg = this;
	DragAcceptFiles(1);

	SetTimer(1, 1000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CvrmlanimDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CvrmlanimDlg::OnPaint()
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
	}

	RECT Rect;
	CDialog::GetWindowRect(&Rect);
	((CWnd *)GetDlgItem(IDC_CORTONA))->SetWindowPos(&wndTop,
		0, 0, Rect.right - Rect.left - 15, Rect.bottom - Rect.top - 38, SWP_NOREDRAW);
	((CWnd *)GetDlgItem(IDC_CORTONA))->RedrawWindow();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CvrmlanimDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CvrmlanimDlg::OnDestroy()
{
	if (m_CortonaCtl) {
		if (m_pCortonaEvents) {
			AfxConnectionUnadvise(m_CortonaCtl->GetControlUnknown(), DIID__CortonaEvents,
				m_pCortonaEvents->GetIDispatch(FALSE), FALSE, m_dwCoockie);
			m_pCortonaEvents->GetIDispatch(FALSE)->Release();
			m_pCortonaEvents = 0;
		}
		m_pCortona->Release();
		m_pCortona = 0;
	}

	//Free all memory
	if (m_p3DViewService != NULL)
	{
		m_p3DViewService->Release();
		m_p3DViewService = NULL;
	}
	m_pEngine->Release(); 
	
	CDialogEx::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CAnimSocket

CAnimSocket::CAnimSocket()
{
}

CAnimSocket::~CAnimSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CAnimSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CAnimSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CAnimSocket member functions

void CAnimSocket::OnAccept(int nErrorCode)
{
	SOCKADDR pSockAddr;
	int lAddr = sizeof(SOCKADDR);
	int Err;

	// close and shutdown any previous lingering socket
	if (pAnimSock.m_hSocket != INVALID_SOCKET) {
		pAnimSock.Close();
		pAnimSock.ShutDown();
	}
	// connect the new socket from the server.
	if (pVRMLSock.Accept((CAsyncSocket&)pAnimSock, &pSockAddr, &lAddr) == 0) {
		Err = GetLastError();
	};
	CAsyncSocket::OnAccept(nErrorCode);
}

void CAnimSocket::OnReceive(int nErrorCode)
{
	char NodeName[200];
	int Operation=0, len, iVar=0, templen;
	char lpBuf[2000];
	float NodeData[3];
	CTime now;

	len = pAnimSock.Receive(lpBuf, 2000, 0);
	if (!Initialized) {
		now = CTime::GetCurrentTime();
		now -= (AnimDlg->startAnimTime.GetTime());
		if (now.GetSecond() > 10)
			Initialized = true;
	}
	while ((len > 0) && (Initialized)) {
		// copy the data from the Socket into the data fields
		//iVar = sscanf_s(lpBuf, "%200s%i%f%f%f", (char*)NodeName, &Operation, &NodeData[0], &NodeData[1], &NodeData[2]);
		if (iVar == 5) {
			Data(0, 0) = (float)NodeData[0];
			Data(1, 0) = (float)NodeData[1];
			Data(2, 0) = (float)NodeData[2];

			switch (Operation) {
			case 1: // translation 
				//m_EAIManager.doTranslate(NodeName, Data);
				break;
			case 2: // rotation
				//m_EAIManager.doRotate(NodeName, Data);
				break;
			case 3: // position
				//m_EAIManager.doPosition(NodeName, Data);
				break;
			case 4: // orientation
				//m_EAIManager.doOrientation(NodeName, Data);
				break;
			case 5: // scale
				//m_EAIManager.doScale(NodeName, Data);
				break;
			case 6: // Quaternion roatation
				// Note that the data from Matlab represents X Y and Q. 
				// Z is implicit since |[X,Y,Z]|==1
				//m_EAIManager.doRotateQ(NodeName, Data);
				break;
			case 10:
				AnimDlg->DestroyWindow();
				break;
			}
			templen = (int)strlen(lpBuf);
			if (templen < len) {
				memcpy(lpBuf, lpBuf + templen + 1, 2000 - templen - 1);
				len = len - 1 - templen;
			}
			else {
				len = pAnimSock.Receive(lpBuf, 2000, 0);
			}
		}
	}

	CAsyncSocket::OnReceive(nErrorCode);
}


void CvrmlanimDlg::OnDropFiles(HDROP hDropInfo)
{
	lastwrite.dwLowDateTime = 0;  // invalidate reload
	UINT result = DragQueryFile(hDropInfo, 0, fname, 500);
	//m_EAIManager.shutdownCosmoPlayer();
	//m_EAIManager.InitializeCosmoPlayer(GetDlgItem(IDC_COSMOPLAYER), fname);
	m_CortonaCtl->SetScene(fname);
	m_CortonaCtl->SetHeadLight(TRUE);

	m_CortonaCtl->SetNavigationBar(1);

	CDialogEx::OnDropFiles(hDropInfo);
}


void CvrmlanimDlg::OnTimer(UINT_PTR nIDEvent)
{
	FILETIME newlastwrite;

	// check current scene file time and data. 
	// if changed, then refresh
	HANDLE hFile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		GetFileTime(hFile, &create, &lastaccess, &newlastwrite);
		if ((lastwrite.dwLowDateTime != newlastwrite.dwLowDateTime) ||
			(lastwrite.dwHighDateTime != newlastwrite.dwHighDateTime))
		{
			if ((lastwrite.dwLowDateTime != 0) && (lastwrite.dwHighDateTime != 0))
			{
				//m_EAIManager.shutdownCosmoPlayer();
				//m_EAIManager.InitializeCosmoPlayer(GetDlgItem(IDC_COSMOPLAYER), fname);
				m_CortonaCtl->SetScene(fname);

				ShowWindow(SW_SHOW);
			}
			lastwrite.dwLowDateTime = newlastwrite.dwLowDateTime;
			lastwrite.dwHighDateTime = newlastwrite.dwHighDateTime;
		}
		CloseHandle(hFile);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CvrmlanimDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	OnPaint();
}


// We can receive two kinds of messages from Simulink.
// Format is:
// Msg 1 - wParam = [Operation,NodeID]
//         lParam = Data_x
// Msg 2 - wParam = Data_y
//       - lParam = Data_z
LRESULT CvrmlanimDlg::OnSimulink(WPARAM wParam, LPARAM lParam)
{
	float Value;

	// extract data from wParam
	BUFFER[BufferIndex].Operation = (int)wParam & 0x0007;
	BUFFER[BufferIndex].Node = ((int)wParam & 0xFFF8) >> 3;

	// there are still 16 bits left unused in wParam
	memcpy(&Value, &lParam, sizeof(float));
	BUFFER[BufferIndex].Data[0] = Value;

	// double-check for a 'special operation'
	if (BUFFER[BufferIndex].Operation == 0) {

		switch (lParam) {
		case 0: ShowWindow(SW_SHOWNORMAL);
			//MessageBox("Message 0");
			break;
		case 1: DestroyWindow();
			//MessageBox("Message 1");
			break;
		case 2: ShowWindow(SW_HIDE);
			break;
		default: break;// illegal operation
		}
	}
	return true;
}

// Callback for the second message from Simulink
LRESULT CvrmlanimDlg::OnData(WPARAM wParam, LPARAM lParam)
{
	float Value;
	int i;
	//char msg[50];

	// current buffer size is 100 (see VRMLANIMDlg.h) 
	// If buffer is full, than we probably have lost track of the first node, 
	// and we'll reset the buffer
	if (BufferIndex == 100) {
		BufferIndex = 0;
	}

	memcpy(&Value, &wParam, sizeof(float));
	BUFFER[BufferIndex].Data[1] = Value;
	memcpy(&Value, &lParam, sizeof(float));
	BUFFER[BufferIndex].Data[2] = Value;

	i = BufferIndex;
	// if the current node and operation is the same as the first one in the buffer
	// then process and reset the VRML operations buffer
/*
	if ((BufferIndex>0) &&
		(BUFFER[BufferIndex].Node==BUFFER[0].Node) &&
		(BUFFER[BufferIndex].Operation==BUFFER[0].Operation)) {
		for (i=0;i<BufferIndex;i++) {
*/			Data(0, 0) = BUFFER[i].Data[0];
	Data(1, 0) = BUFFER[i].Data[1];
	Data(2, 0) = BUFFER[i].Data[2];
	switch (BUFFER[i].Operation) {
	case 1: // translation 
		//m_EAIManager.doTranslate(vrml_lib[BUFFER[i].Node], Data);
		break;
	case 2: // rotation
		//m_EAIManager.doRotate(vrml_lib[BUFFER[i].Node], Data);
		break;
	case 3: // position
		//m_EAIManager.doPosition(vrml_lib[BUFFER[i].Node], Data);
		break;
	case 4: // orientation
		//m_EAIManager.doOrientation(vrml_lib[BUFFER[i].Node], Data);
		break;
	case 5: // scale
		//m_EAIManager.doScale(vrml_lib[BUFFER[i].Node], Data);
		break;
	case 6: // Quaternion roatation
		// Note that the data from Matlab represents X Y and Q. 
		// Z is implicit since |[X,Y,Z]|==1
		//m_EAIManager.doRotateQ(vrml_lib[BUFFER[i].Node], Data);
		break;
	}
	// clear buffer entries
	BUFFER[i].Operation = 0;
	BUFFER[i].Node = 0;
	/*		}
			// reset buffer
			BUFFER[0] = BUFFER[BufferIndex];
			BUFFER[BufferIndex].Operation = 0;
			BUFFER[BufferIndex].Node = 0;
			BufferIndex = 0;
		} else {
			BufferIndex++;
		}

	 */
	return true;
}
