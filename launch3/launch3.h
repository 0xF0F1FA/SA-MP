
// launch3.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLaunch3App:
// See launch3.cpp for the implementation of this class
//

class CLaunch3App : public CWinApp
{
public:
	CLaunch3App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLaunch3App theApp;
