// This file is part of an extension of the original CrashRpt library.
// Modifications Copyright (c) 2012, Andreas Schoenle
// All rights reserved.

#ifndef _CRASHRPTEX_H_
#define _CRASHRPTEX_H_
// Additional function exports for the continue execution functionality.

#ifndef CRASHRPT_EX
#define CRASHRPT_EX
#endif

#include "CrashRpt.h"

//! New property by AS. This will cause the library to also submit the dns name of the 
//! computer and the username of the user logged on.
#define CR_INST_ADD_USER_INFO				   0x0100000

/* New properties by AS, not standard.
 * When used with the crAllowContinue function these will disable the Terminate() call 
 * in the handlers and throw a crash description struct instead which can be caught.
 * Also the API will display an option to continue execution.
 * see crAllowContinue() for more detaily
 */

//! Do not offer to continue the application (default).
#define CR_INST_APP_TERMINATE                  0x0000000 
//! Offer to continue the application. If chosen in the handler an exception 
//! will be thrown instead of Terminate() being called.
#define CR_INST_APP_CONTINUE				   0x0200000 
//! Offer to continue the application as the only option 
#define CR_INST_APP_CONTINUE_ONLY			   0x0400000
//! Offer to continue the application as the default option
#define CR_INST_APP_CONTINUE_DEFAULT		   0x0800000 
//! This will disable launching the crash sender inside the handler and just 
//! throw the exception. Call crHandleError() or crDiscardError() to launch or
//! not launch the sender depending on the above flags in your catch() statement
#define CR_INST_APP_CONTINUE_NOSENDER		   0x1000000 
//! Bits reserved for app continue options
#define CR_INST_APP_CONTINUE_MASK			   0x1e00000


/*! \brief Modify CrashRpt flags without reinstalling
 *  \ingroup CrashRptAPI
 */
CRASHRPTAPI(int)
crModifyFlags(
  DWORD dwFlags, DWORD dwMask);

/*! \brief Modify the way the handlers treat errors 
 *  \ingroup CrashRptAPI
 * Modify the way the handlers treat errors in the current thread. Depending on 
 * dwFlags this will cause a CR_EXCEPTION_INFO instead of or after generating 
 * the report and the process will not be terminated. Catch this and call 
 * crHandleError() or crDiscardError() if you want to do additional work.
 * @param dwFlags
 *   The flags are documented above and all start with CR_INST_APP_CONTINUE
 * @return 
 *   It returns the previous flags cast to an int or (-1) if the function 
 *   failed (the state is then unchanged)
 */
CRASHRPTAPI(int)
crAllowContinue(
  DWORD dwFlags);

/*! Handle an error inside the catch clause when allowing to continue and 
 * setting CR_INST_APP_CONTINUE_NOSENDER. Will Terminate or allow to continue 
 * depending on the users's choice.
 */
CRASHRPTAPI(int)
crHandleError(
  CR_EXCEPTION_INFO &ei);


/*! Discard an error such that the application can continue 
 */
CRASHRPTAPI(int)
crDiscardError(
  CR_EXCEPTION_INFO &ei);

#ifndef _CRASHRPT_NO_WRAPPERS

class CrAllowContinueHelper
{
public:
  //! Installs exception handlers to the caller thread
  CrAllowContinueHelper(DWORD dwFlags = CR_INST_APP_CONTINUE)
  {
    m_nInstallStatus = crAllowContinue(dwFlags);
  }

  //! Uninstalls exception handlers from the caller thread
  ~CrAllowContinueHelper()
  {
    // Reset to the previous state
  if (m_nInstallStatus != -1)
  	crAllowContinue((DWORD) m_nInstallStatus);
  }
  
  DWORD m_nInstallStatus;
};

#endif 

//! On 64bit systems running a 32bit application the process will usually 
//! swallow exceptions behind a window callback.
//! This is a windows bug which may stay in place because it has been there
//! for a long time.
//! See:
//! http://support.microsoft.com/kb/976038
//! http://connect.microsoft.com/VisualStudio/feedback/details/550944/hardware-exceptions-on-x64-machines-are-silently-caught-in-wndproc-messages
//! Calling this function will enable the hotfix and cause an 
//! an application error dialog to appear instead. Importantly the exception
//! is still not propagated upwards, i.e. there is no stack unwinding to a possible
//! exeption handler before the callback. 
//! @param bEnable
//!		Enable or disable the callback filter. If b_enable == true the default
//!		behaviour is produced, i.e. the exception is silently caught. If 
//!		b_enable == false the exception is handled and an error dialog 
//!		appears.
//! @return
//!		The status after the call. See crProcessCallbackFilterStatus()
int crEnableProcessCallbackFilter(BOOL bEnable);

//! Test whether it is enabled. 
//! @return
//!		1: Hotfix present and filter active
//!		2: Hotfix not present, filter active if this is an affected system,
//!		   not active otherwise
//!		0: Hotfix present, filter not active
int crProcessCallbackFilterStatus();

//! For some applications, enabling the hotfix is not enough. They rely on 
//! being able to catch the exception and do e.g. some data recovery before 
//! exiting or they roll their own diagnostics.
//! There is no known way to make the exception go past the callback but with 
//! this function you can wrap the default windows procedure in MFC with a 
//! procedure that handles exceptions. The easiest way to do this is shown
//! in the comment below.
//! NOTE: Subclassing in MFC is a little complicated as there is both the
//! windows procedures (and the default MFC windows procedure calls the 
//! MFC CWnd::WindowProc implementation which can be overwritten). So basically
//! MFC windows can be subclassed by (1) overwriting the virtual 
//! WindowProc function, (2) adding entries to the message map handled by the 
//! standard implementation and (3) subclassing in the windows sense, that is
//! by calling SetWindowLong(). 
//! We do not want to call SetWindowLong on each and every window, maintain
//! a map of previous functions etc. Rather we replace the function pointer 
//! that is used by _AfxCbtFilterHook() to set the subclass the window 
//! making it call some windows procedure that will actually call the member
//! function.
//! NOTE: This explanation, even the way MFC handles this may change and then 
//!       the code may stop working. If that happens, find out where the new 
//!		  MFC version calls SetWindowLong() to subclass the windows and replace
//!		  the pointer it uses by the installed windows procedure.
//! @param pfnWndProc
//!		The windows procedure that should be called instead of the old 
//!		one as default.
//!		If this is NULL and a wrapper was previously installed, it it is
//!		re-installed. Use this with care as a library from which the 
//!		wrapper came may have been unloaded.
//!	@return
//!		The old windows procedure which should be wrapped by pfnWndProc
WNDPROC crInstallWndProcWrapperImp(WNDPROC pfnWndProc, BYTE * pModuleState, size_t offset);

#ifdef __AFX_H__
WNDPROC crInstallWndProcWrapperImp(WNDPROC pfnWndProc, BYTE * pModuleState, size_t offset);
#define crInstallWndProcWrapper(pfnWndProc)	\
		crInstallWndProcWrapperImp(pfnWndProc, (BYTE *) AfxGetAppModuleState(), offsetof(AFX_MODULE_STATE, m_pfnAfxWndProc))
#else
#define crInstallWndProcWrapper(pfnWndProc)	\
		crInstallWndProcWrapperImp(pfnWndProc, NULL, NULL)
#endif


// NOTE: One should also look at context switching. Exceptions occuring 
// behind a context switch will be caught and rethrown, obscuring their
// origin. In order to avoid this, call
//		AfxSetAmbientActCtx(FALSE);
// In every InitInstance() function.
// See:
// http://connectppe.microsoft.com/VisualStudio/feedback/details/563622/mfc-default-exception-handling-causes-problems-with-activation-context

//! Enable or disable and installed wndproc wrapper
//! @return
//!		The current status (see crWndProcWrapperStatus())
int crEnableWndProcWrapper(BOOL bEnable);

//! Test whether a wrapper was installed using these functions
//! @return
//!		0: not installed
//!		1: installed and enabled
//!		2: installed and disabled
//!		3: installed and replaced by some other function. This is not good 
//!		   and mostly an error condition. Cannot uninstall
int crWndProcWrapperStatus();

/* Example code to catch structured exceptions before Dr.Watson is 
   triggered or the exception is swallowed:

// Stores the old windows procedure
static WNDPROC _s_wndproc = NULL;

// Fitler the exception, report error etc.
int exceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep) 
{
   if (code == EXCEPTION_ACCESS_VIOLATION) 
   {
	  // Access violations are handled in the handler ...
      return EXCEPTION_EXECUTE_HANDLER;
   }
   else 
   {
	  // Do whatever the system would have done. In our case, an error dialog
	  // will appear unless we are in SysWow and have no hotfix (see above)
      return EXCEPTION_CONTINUE_SEARCH;
   };
}

// The new windows procedure
static LRESULT CALLBACK wndproc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	__try
	{
		if (_s_wndproc)
			_s_wndproc(hWnd, nMsg, wParam, lParam);
	}
	__except(exceptionFilter(GetExceptionCode(), GetExceptionInformation()))
	{
		return 0;
	}
}

// And somewhere in your MFC code (e.g. CWinApp::InitInstance()) install the
// wrapper
_s_wndproc = crInstallWndProcWrapper(&wndproc);

*/

#endif