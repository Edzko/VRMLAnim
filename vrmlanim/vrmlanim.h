
// vrmlanim.h : main header file for the PROJECT_NAME application
//

#pragma once
extern char fname[500];

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CvrmlanimApp:
// See vrmlanim.cpp for the implementation of this class
//

class CvrmlanimApp : public CWinApp
{
public:
	CvrmlanimApp();
	BOOL CheckPasswd(const char *Code);

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CvrmlanimApp theApp;
