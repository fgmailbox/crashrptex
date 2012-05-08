// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

// MainDlg.h : interface of the CMainDlg class
//

#pragma once
#include "afxwin.h"

// CMainDlg dialog
class CMainDlg : public CDialog
{
// Construction
public:
	CMainDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MAINDLG };

	virtual void OnOK();
	virtual void OnCancel();


    void CloseDialog(int nVal);
    void DoCrash();
    BOOL m_bRestarted;
	
    CComboBox m_cboThread;
    CComboBox m_cboExcType;
#ifdef CRASHRPT_EX
// AS: Logs an entry, adds newline
	void Log(const CString &sEntry);
// AS: Allow continue 
    CComboBox m_cboAllowContinue;
// AS: Launch sender from handler or not
    CButton m_btLaunchSender;
	CButton m_btCallbackFilter;
	CButton m_btHook;
	CButton m_btWndprocWrapper;
#endif
  
	int m_nDocNum;
	virtual INT_PTR DoModal();

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
#ifdef CRASHRPT_EX
	afx_msg void OnBnClickedHook();
	afx_msg void OnBnClickedWrapper();
	afx_msg void OnBnClickedFilter();
	afx_msg void OnCbnSelchangeAllowtype();
	afx_msg void OnBnClickedLaunchSender();
	void LogProcessFilterState();
#endif
	CEdit m_edtLog;
};
