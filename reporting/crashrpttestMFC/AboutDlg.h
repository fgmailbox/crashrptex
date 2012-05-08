// This file is part of an extension of the original CrashRpt library and derived from
// its CrashRptTest project. (port from WTL to MFC)

// Modifications to support the extensions are guarded by #ifdef CRASHRPT_EX
// Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg() : CDialog(CAboutDlg::IDD)
    {
	}

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX)
	{
        CDialog::DoDataExchange(pDX);
    }

// Implementation
protected:
};
