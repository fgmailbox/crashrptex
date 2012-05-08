// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

#pragma once
#include "stdafx.h"
#ifdef CRASHRPT_EX
// AS: Extended interface
#include "CrashRptEx.h"
#else
#include "CrashRpt.h"
#endif


struct CrashThreadInfo
{  
    HANDLE m_hWakeUpEvent;
    bool m_bStop;
    int m_ExceptionType;
};

DWORD WINAPI CrashThread(LPVOID pParam);

void test_seh();
void test_generate_report();

