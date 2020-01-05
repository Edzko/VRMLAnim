
// vrmlanimDlg.h : header file
//

#pragma once

#include "CortonaCtl.h"

#define _NO_TEMPLATE
//#define _NO_NAMESPACE
#include "matrix.h"

#ifndef _NO_NAMESPACE
using namespace std;
using namespace math;
#define STD std
#else
#define STD
#endif
#ifndef _NO_TEMPLATE
typedef matrix<double> Matrix;
#else
typedef matrix Matrix;
#endif;

typedef struct VMLR_OP {
	int Operation;
	int Node;
	float Data[3];
} VRML_OP;

class CAnimSocket : public CAsyncSocket
{
	// Attributes
public:
	bool Initialized;
	// Operations
public:
	CAnimSocket();
	virtual ~CAnimSocket();

	// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimSocket)
public:
	virtual void OnAccept(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CAnimSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

// CvrmlanimDlg dialog
class CvrmlanimDlg : public CDialogEx
{
// Construction
public:
	CvrmlanimDlg(CWnd* pParent = nullptr);	// standard constructor
	
	FILETIME create, lastaccess, lastwrite;
	int Element;
	int Operation;
	int Node;
	CTime startAnimTime;

	// public socket for receiving commands
	bool TCPConnected;

	// Allocate a operation buffer for 100 nodes
	VRML_OP BUFFER[100];
	int BufferIndex;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VRMLANIM_DIALOG };
#endif

	CWnd m_ControlWrapper;

	CCortonaCtl *m_CortonaCtl;
	I3DViewService2* m_p3DViewService;
	IEngine* m_pEngine;
	interface ICortona* m_pCortona;
	INodeObject* pMaster, *pTrolley, *pWagon1, *pWagon2, *pChase;
	INodeObject* pCable1, *pCable2, *pCable3, *pCable4, *pContainer;
	ISFVec3fObject* m_pTranslation;
	ISFRotationObject* m_pRotation;

	class CCortonaEvents* m_pCortonaEvents;
	DWORD m_dwCoockie;

	UINT m_SelectedRenderer;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	LRESULT OnSimulink(WPARAM wParam, LPARAM lParam);
	LRESULT OnData(WPARAM wParam, LPARAM lParam);
};
