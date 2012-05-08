// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

// CrashRptTestMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CrashRptTest.h"
#include "AboutDlg.h"
#include "MainDlg.h"
#include "CrashThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MANUAL_REPORT 128

#ifdef CRASHRPT_EX
HHOOK hHook = 0;
static LRESULT CALLBACK CallWndProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	return CallNextHookEx(hHook, nCode, wParam, lParam );
}

void InstallWndprocHook()
{
	if (!hHook)
		hHook = (HHOOK)SetWindowsHookEx( WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
}

void UninstallWndprocHook()
{
	if (hHook)
	{
		UnhookWindowsHookEx(hHook);
		hHook = NULL;
	}
}

BOOL IsWndprocHookInstalled()
{
	return hHook != NULL;
}
#endif

// CMainDlg dialog
CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_THREAD, m_cboThread);
	DDX_Control(pDX, IDC_EXCTYPE, m_cboExcType);
#ifdef CRASHRPT_EX
	DDX_Control(pDX, IDC_ALLOW_CONTINUE, m_cboAllowContinue);
	DDX_Control(pDX, IDC_LAUNCH_SENDER, m_btLaunchSender);
	DDX_Control(pDX, IDC_CALLBACK_FILTER, m_btCallbackFilter);
	DDX_Control(pDX, IDC_HOOK, m_btHook);
	DDX_Control(pDX, IDC_WNDPROC_WRAPPER, m_btWndprocWrapper);
	DDX_Control(pDX, IDC_LOG, m_edtLog);
#endif
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
#ifdef CRASHRPT_EX
	ON_BN_CLICKED(IDC_HOOK, &CMainDlg::OnBnClickedHook)
	ON_BN_CLICKED(IDC_WNDPROC_WRAPPER, &CMainDlg::OnBnClickedWrapper)
	ON_BN_CLICKED(IDC_CALLBACK_FILTER, &CMainDlg::OnBnClickedFilter)
	ON_CBN_SELCHANGE(IDC_ALLOW_CONTINUE, &CMainDlg::OnCbnSelchangeAllowtype)
	ON_BN_CLICKED(IDC_LAUNCH_SENDER, &CMainDlg::OnBnClickedLaunchSender)
#endif
END_MESSAGE_MAP()

// CMainDlg message handlers

void CMainDlg::LogProcessFilterState()
{
	switch(crProcessCallbackFilterStatus())
	{
	case 0:
		Log(_T("Process callback filter disabled."));
		break;
	case 1:
		Log(_T("Process filter enabled, exceptions will not pass!"));
		break;
	case 2:
		Log(_T("Hotfix not present. Process filter active if this is an affected system."));
		break;
	}
}

BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

    int nItem = 0;


    nItem = m_cboThread.AddString(_T("Main thread"));
    m_cboThread.SetItemData(nItem, 0);

    nItem = m_cboThread.AddString(_T("Worker thread"));
    m_cboThread.SetItemData(nItem, 1);

    m_cboThread.SetCurSel(0);

#ifdef CRASHRPT_EX
    m_btLaunchSender.SetCheck(TRUE);


    nItem = m_cboAllowContinue.AddString(_T("Not allowed"));
    m_cboAllowContinue.SetItemData(nItem, 0);

    nItem = m_cboAllowContinue.AddString(_T("Allowed"));
    m_cboAllowContinue.SetItemData(nItem, CR_INST_APP_CONTINUE);

    nItem = m_cboAllowContinue.AddString(_T("Allowed: default"));
    m_cboAllowContinue.SetItemData(nItem, CR_INST_APP_CONTINUE_DEFAULT);

    nItem = m_cboAllowContinue.AddString(_T("Allowed: only option"));
    m_cboAllowContinue.SetItemData(nItem, CR_INST_APP_CONTINUE_ONLY);
    m_cboAllowContinue.SetCurSel(0);
	
	m_btHook.SetCheck(IsWndprocHookInstalled());
	m_btWndprocWrapper.SetCheck(crWndProcWrapperStatus() == 1);
	m_btCallbackFilter.SetCheck(crProcessCallbackFilterStatus());

	LogProcessFilterState();
#endif
  

    nItem = m_cboExcType.AddString(_T("SEH exception"));
    m_cboExcType.SetItemData(nItem, CR_SEH_EXCEPTION);

    nItem = m_cboExcType.AddString(_T("terminate"));
    m_cboExcType.SetItemData(nItem, CR_CPP_TERMINATE_CALL);

    nItem = m_cboExcType.AddString(_T("unexpected"));
    m_cboExcType.SetItemData(nItem, CR_CPP_UNEXPECTED_CALL);

    nItem = m_cboExcType.AddString(_T("pure virtual method call"));
    m_cboExcType.SetItemData(nItem, CR_CPP_PURE_CALL);

    nItem = m_cboExcType.AddString(_T("new operator fault"));
    m_cboExcType.SetItemData(nItem, CR_CPP_NEW_OPERATOR_ERROR);

    nItem = m_cboExcType.AddString(_T("buffer overrun"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SECURITY_ERROR);

    nItem = m_cboExcType.AddString(_T("invalid parameter"));
    m_cboExcType.SetItemData(nItem, CR_CPP_INVALID_PARAMETER);

    nItem = m_cboExcType.AddString(_T("SIGABRT"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGABRT);

    nItem = m_cboExcType.AddString(_T("SIGFPE"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGFPE);

    nItem = m_cboExcType.AddString(_T("SIGILL"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGILL);

    nItem = m_cboExcType.AddString(_T("SIGINT"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGINT);

    nItem = m_cboExcType.AddString(_T("SIGSEGV"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGSEGV);

    nItem = m_cboExcType.AddString(_T("SIGTERM"));
    m_cboExcType.SetItemData(nItem, CR_CPP_SIGTERM);

    nItem = m_cboExcType.AddString(_T("throw C++ typed exception"));
    m_cboExcType.SetItemData(nItem, CR_THROW);

    nItem = m_cboExcType.AddString(_T("Manual report"));
    m_cboExcType.SetItemData(nItem, MANUAL_REPORT);

    m_cboExcType.SetCurSel(0);


  if(m_bRestarted)
  {
    ;//PostMessage(WM_POSTCREATE);    
  }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMainDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDlg::OnOK()
{
	DoCrash();
}

void CMainDlg::OnCancel()
{
	CDialog::OnCancel();
}

#ifdef CRASHRPT_EX
// AS: Allow continue in main app and crash thread
DWORD g_dwAllowContinue = 0;
#endif

void CMainDlg::DoCrash()
{
    int nSel = m_cboThread.GetCurSel();
    int nThread = (int)m_cboThread.GetItemData(nSel);

    nSel = m_cboExcType.GetCurSel();
    int nExcType = (int)m_cboExcType.GetItemData(nSel);

    if(nThread==0) // The main thread
    {    
        if(nExcType==MANUAL_REPORT)
        {
            test_generate_report();
            return;
        }
        else
        {
            int nResult = crEmulateCrash(nExcType);
            if(nResult!=0)
            {
                TCHAR szErrorMsg[256];
                CString sError = _T("Error creating exception situation!\nErrorMsg:");
                crGetLastErrorMsg(szErrorMsg, 256);
                sError+=szErrorMsg;
                MessageBox(sError);
            }
        }  
    }
    else // Worker thread
    {
        extern CrashThreadInfo g_CrashThreadInfo;
        g_CrashThreadInfo.m_ExceptionType = nExcType;
        SetEvent(g_CrashThreadInfo.m_hWakeUpEvent); // wake up the working thread
    }
}

#ifdef CRASHRPT_EX
void CMainDlg::OnBnClickedHook()
{
	if (m_btHook.GetCheck())
		InstallWndprocHook();
	else
		UninstallWndprocHook();
}

void CMainDlg::OnBnClickedWrapper()
{
	if (m_btWndprocWrapper.GetCheck())
		crEnableWndProcWrapper(true);
	else
		crEnableWndProcWrapper(false);
}

void CMainDlg::OnBnClickedFilter()
{
	if (m_btCallbackFilter.GetCheck())
		crEnableProcessCallbackFilter(true);
	else
		crEnableProcessCallbackFilter(false);
	LogProcessFilterState();
}

void CMainDlg::OnCbnSelchangeAllowtype()
{
	extern DWORD g_dwAllowContinue;
	
	int nSel = m_cboAllowContinue.GetCurSel();
	g_dwAllowContinue = (DWORD) m_cboAllowContinue.GetItemData(nSel);
	if (! m_btLaunchSender.GetCheck())
		g_dwAllowContinue |= CR_INST_APP_CONTINUE_NOSENDER;

	crAllowContinue(g_dwAllowContinue);
}

void CMainDlg::OnBnClickedLaunchSender()
{
	OnCbnSelchangeAllowtype();
}

#endif

// Copied most of it from dlgcore.cpp
INT_PTR CMainDlg::DoModal()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND hWndParent = PreModal();
	AfxUnhookWindowCreate();
	BOOL bEnableParent = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	CWnd* pMainWnd = NULL;
	BOOL bEnableMainWnd = FALSE;
#endif
	if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd = AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
			//
			// We are hosted by non-MFC container
			// 
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
		if (CreateDlgIndirect(lpDialogTemplate,
						CWnd::FromHandle(hWndParent), hInst))
		{
			if (m_nFlags & WF_CONTINUEMODAL)
			{
				while(1)
				{
#ifdef CRASHRPT_EX
					try
					{
#endif
						// enter modal loop
						DWORD dwFlags = MLF_SHOWONIDLE;
						if (GetStyle() & DS_NOIDLEMSG)
							dwFlags |= MLF_NOIDLEMSG;
						VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
						break;
#ifdef CRASHRPT_EX
					}
					catch (CR_EXCEPTION_INFO &ei) 
					{
						HandleError(ei, _T("outside modal loop."));
						m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
					}
#endif
				}
			}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		if (e) 
			e ->Delete();
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OLE_SUPPORT
	if (bEnableMainWnd)
		pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}

#ifdef CRASHRPT_EX
void CMainDlg::Log(const CString&str)
{
	CString s;
	m_edtLog.GetWindowText(s);
	s += str + _T("\r\n");
	m_edtLog.SetWindowText(s);
	int n = s.GetLength();
	m_edtLog.SetSel(n,n);
}
#endif