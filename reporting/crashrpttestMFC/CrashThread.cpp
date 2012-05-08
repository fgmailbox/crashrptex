// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

#include "stdafx.h"
#include "CrashThread.h"
#include "CrashRptTest.h"
#include <exception>
#include <signal.h>
#include <assert.h>

// Tests crExceptionFilter
void test_seh()
{
    __try
    {
        int nResult = crEmulateCrash(CR_NONCONTINUABLE_EXCEPTION);
        if(nResult!=0)
        {      
            MessageBox(NULL, _T("Error raising noncontinuable exception!"), _T("Error"), 0);    
        }
    }
    __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {    
        // Terminate program
        ExitProcess(1);
    }
}

// Tests crGenerateErrorReport
void test_generate_report()
{
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_SEH_EXCEPTION;
    ei.code = 0x1234;
    ei.pexcptrs = NULL;
    ei.bManual = TRUE; // Signal the report is being generated manually.

    int nResult = crGenerateErrorReport(&ei);
    if(nResult!=0)
    {
        TCHAR szErrorMsg[256];
        CString sError = _T("Error generating error report!\nErrorMsg:");
        crGetLastErrorMsg(szErrorMsg, 256);
        sError+=szErrorMsg;
        MessageBox(NULL, sError, 0, 0);
    }
}

#ifdef CRASHRPT_EX
// AS: allow continue in the crash thread
extern DWORD g_dwAllowContinue;
#endif

DWORD WINAPI CrashThread(LPVOID pParam)
{
    CrashThreadInfo* pInfo = (CrashThreadInfo*)pParam;

    // Install per-thread exception handlers
    CrThreadAutoInstallHelper cr_install_helper(0);

    for(;;)
    {
        // Wait until wake up event is signaled
        WaitForSingleObject(pInfo->m_hWakeUpEvent, INFINITE);   

#ifdef CRASHRPT_EX
// AS: Allow continue depending on g_dwAllowContinue
    try {
        CrAllowContinueHelper cr_ac_helper(g_dwAllowContinue);
#endif

        if(pInfo->m_bStop)
            break; // Exit the loop

        if(pInfo->m_ExceptionType==128)
        {
            // Test generate report manually
            test_generate_report();
        }
        else if(pInfo->m_ExceptionType==CR_NONCONTINUABLE_EXCEPTION)
        {
            // Test crExceptionFilter
            test_seh();
        }
        else if(crEmulateCrash(pInfo->m_ExceptionType)!=0)
        {
            // Test exception handler
            TCHAR szErrorMsg[256];
            CString sError = _T("Error creating exception situation!\nErrorMsg:");
            crGetLastErrorMsg(szErrorMsg, 256);
            sError+=szErrorMsg;
            MessageBox(NULL, sError, _T("Error"), 0);    
        }
#ifdef CRASHRPT_EX
// AS: If crash sender is not lanched in the handler, do it here.
        }
        catch (CR_EXCEPTION_INFO &ei) 
        {
            HandleError(ei, _T("in crash thread"));
        }
#endif
    }
    // Exit this thread
    return 0;
}




