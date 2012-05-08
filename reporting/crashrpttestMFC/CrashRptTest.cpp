// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.


#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

#include "CrashRptTest.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "CrashThread.h"
#include <shellapi.h>


HANDLE g_hWorkingThread = NULL;
CrashThreadInfo g_CrashThreadInfo;

// Helper function that returns path to application directory
CString GetAppDir()
{
    CString string;
    LPTSTR buf = string.GetBuffer(_MAX_PATH);
    GetModuleFileName(NULL, buf, _MAX_PATH);
    *(_tcsrchr(buf,'\\'))=0; // remove executable name
    string.ReleaseBuffer();
    return string;
}

// Helper function that returns path to module
CString GetModulePath(HMODULE hModule)
{
    CString string;
    LPTSTR buf = string.GetBuffer(_MAX_PATH);
    GetModuleFileName(hModule, buf, _MAX_PATH);
    TCHAR* ptr = _tcsrchr(buf,'\\');
    if(ptr!=NULL)
        *(ptr)=0; // remove executable name
    string.ReleaseBuffer();
    return string;
}


// CCrashRptTestMFCApp

BEGIN_MESSAGE_MAP(CCrashRptTestMFCApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCrashRptTestMFCApp construction

CCrashRptTestMFCApp::CCrashRptTestMFCApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CCrashRptTestMFCApp object

CCrashRptTestMFCApp theApp;

#ifdef CRASHRPT_EX
// AS: Demonstration of how to handle errors in a windows procedure
static CMainDlg * _s_p_Dlg = NULL;

int HandleError(CR_EXCEPTION_INFO &ei, LPCTSTR szLoc)
{
	int ret = -1;
	if (_s_p_Dlg)
	{
		CString s;
		s.Format(_T("---\r\nException caught %s"), szLoc);

		_s_p_Dlg ->Log(s);
		if (ei.hSenderProcess)
			_s_p_Dlg ->Log(_T("  Crash was launched in handler."));
	
		ret = crHandleError(ei);
		_s_p_Dlg ->Log(_T("  Finished error, allowing continue."));
	}
	else
	{
		throw ei;
	}

	return ret;
}

// The old windows procedure
static WNDPROC _s_WndProcOld = NULL;
// The new windows procedure including error handling
static LRESULT CALLBACK _s_WndProcNew(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		return (* _s_WndProcOld)(hWnd, nMsg, wParam, lParam);
	}
	catch (CR_EXCEPTION_INFO &ei) 
	{
		HandleError(ei, _T("in windows procedure."));
		return 0;
	}
}

#endif

BOOL WINAPI CrashCallback(LPVOID lpvState)
{
  UNREFERENCED_PARAMETER(lpvState);
#ifdef CRASHRPT_EX
  _s_p_Dlg ->Log(_T("Crash callback called!"));
#endif
  // Crash happened!
  return TRUE;
}


BOOL CCrashRptTestMFCApp::InitInstance()
{
    AfxSetAmbientActCtx(FALSE);
#ifdef CRASHRPT_EX
	// AS: Install wrapper for WndProc handling exceptions
    _s_WndProcOld = crInstallWndProcWrapper(& _s_WndProcNew);
#endif
	
    // Install crash reporting

    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);  
#ifdef CRASHRPT_EX
// AS: Settings for testing in my environment
    //info.pszAppName = _T("CrashRptTestMFC"); // Define application name.
    //info.pszAppVersion = _T("1.2.7");     // Define application version.
    info.pszEmailSubject = _T("Error from CrashRptTest"); // Define subject for email.
    info.pszEmailTo = _T("aschoen@gwdg.de");   // Define E-mail recipient address.  
    info.pszUrl = _T("https://imspector.mpibpc.mpg.de/crashrpt"); // URL for sending reports over HTTP.
    info.pfnCrashCallback = CrashCallback; // Define crash callback function.   
    // Define delivery transport priorities. 
    info.uPriorities[CR_HTTP] = 3;         // Use HTTP the first.
    info.uPriorities[CR_SMTP] = 2;         // Use SMTP the second.
    info.uPriorities[CR_SMAPI] = 1;        // Use Simple MAPI the last.  
    info.dwFlags = 0;                    
    info.dwFlags |= CR_INST_ADD_USER_INFO;           // Allow (allowing to) continue the application. 
    info.dwFlags |= CR_INST_ALL_EXCEPTION_HANDLERS; // Install all available exception handlers.
    info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;   // Use binary encoding for HTTP uploads (recommended).  
    //info.dwFlags |= CR_INST_APP_RESTART;            // Restart the application on crash.  
#else
    info.pszAppName = _T("CrashRpt Tests"); // Define application name.
    //info.pszAppVersion = _T("1.2.7");     // Define application version.
    info.pszEmailSubject = _T("Error from CrashRptTests"); // Define subject for email.
    info.pszEmailTo = _T("aschoen@gwdg.de");   // Define E-mail recipient address.  
    info.pszUrl = _T("http://localhost:80/crashrpt.php"); // URL for sending reports over HTTP.			
    info.pfnCrashCallback = CrashCallback; // Define crash callback function.   
    // Define delivery transport priorities. 
    info.uPriorities[CR_HTTP] = 3;         // Use HTTP the first.
    info.uPriorities[CR_SMTP] = 2;         // Use SMTP the second.
    info.uPriorities[CR_SMAPI] = 1;        // Use Simple MAPI the last.  
    info.dwFlags = 0;                    
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS; // Install all available exception handlers.
    info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;   // Use binary encoding for HTTP uploads (recommended).  
    info.dwFlags |= CR_INST_APP_RESTART;            // Restart the application on crash. 
#endif 
    //info.dwFlags |= CR_INST_NO_MINIDUMP;          // Do not include minidump.
    //info.dwFlags |= CR_INST_NO_GUI;               // Don't display GUI.
    //info.dwFlags |= CR_INST_DONT_SEND_REPORT;     // Don't send report immediately, just queue for later delivery.
    //info.dwFlags |= CR_INST_STORE_ZIP_ARCHIVES;   // Store ZIP archives along with uncompressed files (to be used with CR_INST_DONT_SEND_REPORT)
    info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;    // Send reports that were failed to send recently.
    info.pszDebugHelpDLL = NULL;                    // Search for dbghelp.dll using default search sequence.
    info.uMiniDumpType = MiniDumpNormal;            // Define minidump size.
    // Define privacy policy URL.
    info.pszPrivacyPolicyURL = _T("http://code.google.com/p/crashrpt/wiki/PrivacyPolicyTemplate");
    info.pszErrorReportSaveDir = NULL;       // Save error reports to the default location.
    info.pszRestartCmdLine = _T("/restart"); // Command line for automatic app restart.
    //info.pszLangFilePath = _T("D:\\");       // Specify custom dir or filename for language file.
    //info.pszSmtpProxy = _T("127.0.0.1:2525");  // Use SMTP proxy.
    CString sCustomSenderIcon = GetModulePath(NULL);
#ifdef _DEBUG
    sCustomSenderIcon += _T("\\CrashRptTestd.exe, -203"); // Use custom icon
#else
    sCustomSenderIcon += _T("\\CrashRptTest.exe, -203"); // Use custom icon
#endif
    //info.pszCustomSenderIcon = sCustomSenderIcon.GetBuffer(0);

    // Install crash handlers.
    CrAutoInstallHelper cr_install_helper(&info);
    if(cr_install_helper.m_nInstallStatus!=0)
    {
        TCHAR buff[256];
        crGetLastErrorMsg(buff, 256);
        MessageBox(NULL, buff, _T("crInstall error"), MB_OK);
        return FALSE;
    }
    ATLASSERT(cr_install_helper.m_nInstallStatus==0); 

    CString sLogFile = GetAppDir() + _T("\\dummy.log");
    CString sIniFile = GetAppDir() + _T("\\dummy.ini");

    int nResult = crAddFile2(sLogFile, NULL, _T("Dummy Log File"), CR_AF_MAKE_FILE_COPY);
    ATLASSERT(nResult==0);
  
#ifdef CRASHRPT_EX  
// AS: CallbackFilters for x64 system and 32bit app
	crEnableProcessCallbackFilter(false);
#endif
    nResult = crAddFile(sIniFile, _T("Dummy INI File"));
    ATLASSERT(nResult==0);

// AS: Different screenshot used
    //nResult = crAddScreenshot2(CR_AS_PROCESS_WINDOWS|CR_AS_USE_JPEG_FORMAT, 10);
    nResult = crAddScreenshot(CR_AS_PROCESS_WINDOWS);
    ATLASSERT(nResult==0);

    nResult = crAddProperty(_T("VideoCard"),_T("nVidia GeForce 9800"));
    ATLASSERT(nResult==0);

// AS: No reginfo
    //nResult = crAddRegKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), _T("regkey.xml"), 0);
    //ATLASSERT(nResult==0);

    //nResult = crAddRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), _T("regkey.xml"), 0);
    //ATLASSERT(nResult==0);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	// SetRegistryKey(_T("CrashRpt"));

	/* Create another thread */
    g_CrashThreadInfo.m_bStop = false;
    g_CrashThreadInfo.m_hWakeUpEvent = CreateEvent(NULL, FALSE, FALSE, _T("WakeUpEvent"));
     ASSERT(g_CrashThreadInfo.m_hWakeUpEvent!=NULL);

    DWORD dwThreadId = 0;
    g_hWorkingThread = ::CreateThread(NULL, 0, CrashThread, (LPVOID)&g_CrashThreadInfo, 0, &dwThreadId);
    ASSERT(g_hWorkingThread!=NULL);


	CMainDlg dlg;
	m_pMainWnd = &dlg;
#if CRASHRPT_EX
	_s_p_Dlg = &dlg;
#endif
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	
	// Close another thread
    g_CrashThreadInfo.m_bStop = true;
    SetEvent(g_CrashThreadInfo.m_hWakeUpEvent);
    // Wait until thread terminates
    WaitForSingleObject(g_hWorkingThread, INFINITE);
  
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
