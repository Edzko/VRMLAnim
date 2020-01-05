#pragma once

#define PASSWD "VVSSVRML23"

/////////////////////////////////////////////////////////////////////////////
// PassWd dialog

class PassWd : public CDialog
{
// Construction
public:
	PassWd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PassWd)
	enum { IDD = IDD_PASSWD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PassWd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PassWd)
	afx_msg void OnChangePasswd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

CString Password;

PassWd::PassWd(CWnd* pParent /*=NULL*/)
	: CDialog(PassWd::IDD, pParent)
{
	//{{AFX_DATA_INIT(PassWd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void PassWd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PassWd)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PassWd, CDialog)
	//{{AFX_MSG_MAP(PassWd)
	ON_EN_CHANGE(IDC_PASSWD, OnChangePasswd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PassWd message handlers

void PassWd::OnChangePasswd() 
{	
	GetDlgItemText(IDC_PASSWD,Password);
}

BOOL CvrmlanimApp::CheckPasswd(const char *Code)
{
	//todo: maybe should put not(serial) ...
   time_t Now;
   struct tm today;
	int nResponse;
	DWORD Vol=-1, PwVol;
	GetVolumeInformation("C:\\",NULL,NULL,&Vol,NULL,NULL,NULL,NULL);
	time(&Now);
	_localtime64_s( &today, &Now );

	SetRegistryKey("TEC");
	Password = GetProfileString("","Code");
#ifndef OU
	// password could also be in "Inst"
	if (Password.GetLength()==0) 
		Password = GetProfileString("","Inst");
	WriteProfileString("","Inst","Installed");
#endif
#ifdef OU
	if (Password.GetLength()==0) {
		WriteProfileString("","Code",Code);
		WriteProfileInt("","SN",Vol);
		WriteProfileString("","Code",Code);
		WriteProfileInt("","Operation",Now >> 32);
	}
#endif
	if (Password.Compare(Code)!=0) {
		if (Password.Compare("AutoInstall")!=0) {
			if (Password.Compare("Install")!=0) {    // if code is not "Install"
				MessageBox(NULL,"Installation incomplete\nPlease install again",
					"VRML Software",MB_OK | MB_ICONEXCLAMATION);
				return FALSE;
			}
			PassWd pwd;
			nResponse = (int)pwd.DoModal();       // ask for password
			if (!nResponse) return FALSE;    // if cancelled, abort
			if (Password.Compare(Code)!=0) return FALSE; // if not correct password, abort
		}
		WriteProfileInt("","SN",Vol);
		WriteProfileString("","Code",Code);
		WriteProfileInt("","Operation",0);
	}

	// Make sure that date is in registry and data in computer is not set back
	nResponse = GetProfileInt("","Operation",0);  // default, if does not exist = 0
	/*
	if ((nResponse==0) || (nResponse>(today.tm_yday+today.tm_year*365))) {
		MessageBox(NULL,"Date Error","VRML Software",MB_OK | MB_ICONEXCLAMATION);
		WriteProfileInt("","Operation",0);
		WriteProfileString("","Code","");
		WriteProfileInt("","SN",0);
		return FALSE;
	} else {
		//WriteProfileInt("","Operation",(Now / 86400));
	}
	*/

	
	// for making a license time limit
	/*
	struct tm when;
   when = *localtime( &now );
   when.tm_mday = when.tm_mday + 365;  // 365 days license
   asctime( &when ) 
	*/
	
	// make sure that disk serial number is OK
	PwVol = GetProfileInt("","SN",0);
	if (Vol!=PwVol) {
		MessageBox(NULL,"Illegal Software Copy\nPlease install again",
			"VRML Software",MB_OK | MB_ICONEXCLAMATION);
		WriteProfileInt("","Operation",0);
		WriteProfileString("","Code","");
		WriteProfileInt("","SN",0);
		return FALSE;
	}
	// Could check here for CRC of [SN,Code]

	return TRUE;
}

