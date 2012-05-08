// This file is part of an extension of the original CrashRpt library.
// Modifications Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

#include "stdafx.h"
#include "CrashRptEx.h"

// If this flag is set, the exception will be *swallowed* 
// (i.e. the Server ’03 behavior)
#define PROCESS_CALLBACK_FILTER_ENABLED     0x1

typedef BOOL (WINAPI *set_excpol_t)(DWORD dwFlags);
typedef BOOL (WINAPI *get_excpol_t)(DWORD *dwFlags);

static set_excpol_t _s_pfnSet = 0;
static get_excpol_t _s_pfnGet = 0;

static bool _s_load_kernel32()
{
	static HMODULE hKernel32 = 0;
	if (! _s_pfnSet)
	{
		if (! hKernel32)
			hKernel32 = LoadLibrary(_T("kernel32.dll"));
		_s_pfnSet = (set_excpol_t) GetProcAddress(hKernel32, "SetProcessUserModeExceptionPolicy");
		_s_pfnGet = (get_excpol_t) GetProcAddress(hKernel32, "GetProcessUserModeExceptionPolicy");
	}

	return (_s_pfnSet && _s_pfnGet);
}
	
int crEnableProcessCallbackFilter(BOOL bEnable)
{
	if (_s_load_kernel32())
	{
		DWORD dwFlags;
		if (_s_pfnGet(&dwFlags)) 
		{
			if (bEnable)
			{
				// turn off bit 1
				_s_pfnSet(dwFlags |  PROCESS_CALLBACK_FILTER_ENABLED); 
			}
			else
			{
				_s_pfnSet(dwFlags & ~PROCESS_CALLBACK_FILTER_ENABLED); 
			}
		}
	}
	return crProcessCallbackFilterStatus();
}

// 1: Hotfix present and filter active
// 2: Hotfix not present, filter active if this is an affected system,
// 0: Hotfix present, filter not active
int crProcessCallbackFilterStatus()
{
	DWORD dwFlags;
	if (! _s_load_kernel32())
		return 2;
	if (! _s_pfnGet(&dwFlags))
		return 2;

	return dwFlags & PROCESS_CALLBACK_FILTER_ENABLED;
}
		
static WNDPROC _s_pfnWndProcWrapper = 0;
static WNDPROC _s_pfnWndProc = 0;
static WNDPROC * _s_pWndProcPtr = 0;
static BOOL _s_b_enabled = FALSE;

static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (_s_b_enabled)
		return _s_pfnWndProcWrapper(hWnd, nMsg, wParam, lParam);
	else
		return _s_pfnWndProc(hWnd, nMsg, wParam, lParam);
}


WNDPROC crInstallWndProcWrapperImp(WNDPROC pfnWndProc, BYTE * pModuleState, size_t offset)
{
	if (! pModuleState)
		return NULL;

	if (! _s_pWndProcPtr)
	{
		_s_pWndProcPtr = (WNDPROC *) (pModuleState + offset);
	}

	if (3 == crWndProcWrapperStatus())
		return NULL;

	if (0 == crWndProcWrapperStatus())
	{
		_s_pfnWndProc = * _s_pWndProcPtr;
		_s_pfnWndProcWrapper = pfnWndProc;
		// This will make sure that the Afx hook script uses this to subclass all windows
		// instead of the original WindowsProc. We hope that the m_pfnAfxWndProc will
		// not be overwritten by something else later.
		*_s_pWndProcPtr = _s_WndProc;
	}

	_s_b_enabled = FALSE;
	return _s_pfnWndProc;
}

int crEnableWndProcWrapper(BOOL bEnable)
{
	switch(crWndProcWrapperStatus())
	{
	case 1:
	case 2:
		_s_b_enabled = bEnable;
		break;
	default:
	case 0:
	case 3:
		return false;
	}
	return true;
}

	
//		0: not installed
//		1: installed and enabled
//		2: installed and disabled
//		3: installed and replaced by some other function.
int crWndProcWrapperStatus()
{
	if (!_s_pWndProcPtr)
	{
		// No module state set, this means nothing was installed
		return 0;
	}
	if (_s_pfnWndProcWrapper == NULL)
	{
		// Nothing installed
		return 0;
	}
	else if (_s_WndProc == *_s_pWndProcPtr)
	{
		// Installed and enabled
		return _s_b_enabled ? 1 : 2;
	}
	else 
	{
		// We have a wrapper set to != NULL but neither the old nor the 
		// new function is installed. This is an invalid state
		return 3;
	}	
}