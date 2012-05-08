// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.


#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#ifdef CRASHRPT_EX
#include "CrashRptEx.h"
int HandleError(CR_EXCEPTION_INFO &ei, LPCTSTR sWhere);
#else
#include "CrashRpt.h"
#endif

// CCrashRptTestMFCApp:
// See CrashRptTestMFC.cpp for the implementation of this class
//

class CCrashRptTestMFCApp : public CWinAppEx
{
public:
	CCrashRptTestMFCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCrashRptTestMFCApp theApp;