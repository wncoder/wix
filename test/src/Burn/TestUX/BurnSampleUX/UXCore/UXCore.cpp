//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description: CChainerUXCore implements the IBurnUserExperience in order
//              to provide an interface for a burnstub.exe created chainer
//              setup application
//
//              WARNING: ensure ...\Microsoft.Net\Framework\v2.0.50727 is
//              found first in your PATH otherwise you may not be importing
//              the correct version of mscorlib.tlb
//-----------------------------------------------------------------------------
#include "precomp.h"

#include "uxcore.h"

static BOOL s_fDetectComplete = FALSE;
static BOOL s_fPlanComplete = FALSE;

// needs to match MANAGED_UX_FUNCTIONS sans final enum MUX_FUNCTION_COUNT
LPCWSTR UXFunctionProxyNames[] =
{
	L"Initialize",
	L"Uninitialize",
	L"ShowForm",
	L"OnDetectBegin",
	L"OnDetectPackageBegin",
	L"OnDetectPackageComplete",
	L"OnDetectComplete",
	L"OnPlanBegin",
    L"OnPlanPackageBegin",
    L"OnPlanPackageComplete",
    L"OnPlanComplete",
	L"OnApplyBegin",
    L"OnApplyComplete",
	L"OnProgress",
	L"OnCacheComplete",
	L"OnExecuteBegin",
	L"OnExecutePackageBegin",
	L"OnExecutePackageComplete",
	L"OnExecuteComplete",
	L"OnError",
	L"OnRegisterBegin",
    L"OnRegisterComplete",
    L"OnUnregisterBegin",
    L"OnUnregisterComplete",
    L"OnDownloadProgress",
    L"OnExecuteProgress"
};

//
// Initialize - ensure all the necessary objects are created/initialized.
//
STDMETHODIMP CChainerUXCore::Initialize(
        __in IBurnCore* pCore,
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType
        )
{
	HRESULT hr = S_OK;
	DWORD dwUIThreadId = 0;

	m_fVerificationUXRebootInstant = false;
	m_fRebooted = false;

	if( HasBeenInitialized() ) return S_OK;

	// check args...
	ExitOnNull(pCore, hr, E_INVALIDARG, "Failed to create main window: NULL IBurnCore*.");

	if( SW_HIDE > nCmdShow || SW_MAX < nCmdShow )
	{
		hr = E_INVALIDARG;
		ExitOnFailure(hr, "Failed to create main window: bad nCmdShow.");
	}

	// save IBurnCore pointer and setup the UX
	m_pCore = pCore;

	WCHAR wzVerificationUXRebootInstant[256] = L"";
	INT iBytes = ::GetEnvironmentVariableW(L"VerificationUXRebootInstant", wzVerificationUXRebootInstant, sizeof(wzVerificationUXRebootInstant)/sizeof(WCHAR)-1);

	if (iBytes > 0 && 0 == wcscmp(wzVerificationUXRebootInstant, L"1"))
	{
		m_fVerificationUXRebootInstant = true;
	}

    // create UI thread
    m_hUiThread = ::CreateThread(NULL, 0, UiThreadProc, this, 0, &dwUIThreadId);
    if (!m_hUiThread)
    {
        ExitWithLastError(hr, "Failed to create UI thread.");
    }

LExit:
	if(SUCCEEDED( hr ) )
	{
		m_initialized = S_OK;
	}
	else
	{
		m_initialized = ERROR_APP_INIT_FAILURE;
	}

	return hr;
}

//
// UiThreadProc - entrypoint for UI thread.
//
DWORD WINAPI CChainerUXCore::UiThreadProc(
    __in LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    CChainerUXCore* pThis = (CChainerUXCore*)pvContext;
    BOOL fComInitialized = FALSE;
    BOOL fRet = FALSE;
    MSG msg = { };

   // create main window
    hr = pThis->InitializeUX();
    ExitOnFailure(hr, "Failed to create main window.");

    ::PostMessageW(pThis->m_hWnd, WM_STDUX_DETECT_PACKAGES, 0, 0);

    // message pump
    while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
    {
        if (-1 == fRet)
        {
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Unexpected return value from message pump.");
        }
        else if (!::IsDialogMessageW(pThis->m_hWnd, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

LExit:
    // destroy main window
    pThis->DestroyNativeWindow(pThis->m_hWnd);

    // initiate engine shutdown
    pThis->m_pCore->Shutdown(pThis->m_dwErrorCode);

    // uninitialize COM
    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    return hr;
}

HRESULT CChainerUXCore::CreateMainWindow()
{
	HRESULT hr = S_OK;

	hr = InitializeNativeUX(m_hModule,
		CChainerUXCore::UXWndProc,
		&m_hWnd,
		this);
	ExitOnFailure(hr, "Unable to create main (native) window.");

LExit:
	if (FAILED(hr))
	{
		DestroyNativeWindow(m_hWnd);
	}

	return hr;
}

LRESULT CALLBACK CChainerUXCore::UXWndProc(
	__in HWND hWnd,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
	)
{
   	LRESULT lres = 0;
#pragma warning(suppress:4312)
	CChainerUXCore* pUX = reinterpret_cast<CChainerUXCore*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			pUX = reinterpret_cast<CChainerUXCore*>(lpcs->lpCreateParams);
#pragma warning(suppress:4244)
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pUX));
		}
		break;

	case WM_NCDESTROY:
		lres = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		return lres;

	case WM_CLOSE:
		pUX->DestroyNativeWindow(hWnd);
		return 0;

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_STDUX_SUSPEND:
		pUX->OnSuspend();
		return 0;

	case WM_STDUX_DETECT_PACKAGES:
		pUX->OnDetect();
		return 0;
	case WM_STDUX_PLAN_PACKAGES:
        while (!s_fDetectComplete)
		{
			Sleep(10);
		}
		if (wParam != NULL)
		{
			pUX->SetBurnAction((BURN_ACTION)wParam);
		}
		pUX->OnPlan();
		return 0;
	case WM_STDUX_APPLY_PACKAGES:
		while (!s_fPlanComplete)
		{
			Sleep(10);
		}
		pUX->OnApply((HWND)wParam);
		return 0;

	default:
		{
			if (!pUX ||!pUX->HasBeenInitialized() || pUX->UsingManagedUX())
			{
				break;
			}
		}
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CChainerUXCore::OnPlan()
{
	HRESULT hr = S_OK;

	hr = m_pCore->Plan(m_command.action);
	ExitOnFailure(hr, "Failed to plan chain.");

LExit:
	return;
}

void CChainerUXCore::OnApply(HWND hwnd)
{
	HRESULT hr = S_OK;

	hr = m_pCore->Apply(hwnd);
	ExitOnFailure(hr, "Failed to apply install.");

	m_pCore->Log(BURN_LOG_LEVEL_STANDARD, L"TEST: This message came from the UX DLL");

LExit:
	return;
}

void CChainerUXCore::SetBurnAction(BURN_ACTION burnAction)
{
	m_command.action = burnAction;
}

HRESULT DAPI CChainerUXCore::InitializeNativeUX(HMODULE hModule,
								WNDPROC wndProc,
								HWND* hWnd,
								LPVOID lpParam)
{
	HRESULT hr = S_OK;
	 // Initialize common controls.
    INITCOMMONCONTROLSEX icc = { 0 };
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_PROGRESS_CLASS;
    ::InitCommonControlsEx(&icc);

	LPCWSTR sczWndClassName = L"BurnUXCore";

	WNDCLASSW wc = { 0 };
    DWORD dwWindowStyle = 0;

	wc.style = 0;
    wc.lpfnWndProc = wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hModule;
    wc.hIcon = 0;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = sczWndClassName;
    if (!::RegisterClassW(&wc))
    {
        ExitWithLastError(hr, "Failed to register window.");
    }

	*hWnd = ::CreateWindowExW(0,
		wc.lpszClassName,
		L"Sample Burn",
		dwWindowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		10,
		10,
		HWND_DESKTOP,
		NULL,
		hModule,
		lpParam);

	ExitOnNullWithLastError(*hWnd, hr, "Failed to create window.");

LExit:
	return hr;
}

void CChainerUXCore::DestroyNativeWindow(HWND hwnd)
{
	if( m_hWnd && m_hWnd == hwnd )
	{
		if (m_hWnd)
		{
			::CloseWindow(m_hWnd);
			::DestroyWindow(m_hWnd);
			m_hWnd = NULL;
		}
	}
}

//
// Run - start pumping messages.
//
STDMETHODIMP CChainerUXCore::Run()
{
	if( !HasBeenInitialized() ) return m_initialized;

	HRESULT hr = S_OK;

	if( UsingManagedUX() )
	{
		if( !InvokeRunMethod() )
		{
			hr = E_FAIL;
			ExitOnFailure(hr, "Unable to call InvokeRunMethod().");
		}
	}

	if (S_OK == hr)
	{
		hr = m_dwErrorCode;
	}

LExit:
	return hr;
}

STDMETHODIMP_(void) CChainerUXCore::Uninitialize()
{
    // wait for UX thread to terminate
    if (m_hUiThread)
    {
        ::WaitForSingleObject(m_hUiThread, INFINITE);
        ::CloseHandle(m_hUiThread);
    }

	if( HasBeenInitialized() )
	{
		if( UsingManagedUX() )
		{
			UninitializeManagedUX();
		}
	}

    DestroyNativeWindow(m_hWnd);
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectBegin(
	__in DWORD cPackages
	)
{
	int result = IDABORT;

	if( !HasBeenInitialized() )
	{
		return IDABORT;
	}

	if( UsingManagedUX() )
	{
		result = InvokeOnDetectBegin(cPackages);
	}

	return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectPriorBundle(
    __in_z LPCWSTR wzBundleId
    )
{
    return IDOK;
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectPackageBegin(
	__in_z LPCWSTR wzPackageId
	)
{
	int result = IDABORT;

	if( !HasBeenInitialized() )
	{
		return IDABORT;
	}

	if( UsingManagedUX() )
	{
		result = InvokeOnDetectPackageBegin(wzPackageId);
	}

	return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnDetectPackageComplete(
	__in LPCWSTR wzPackageId,
	__in HRESULT hrStatus,
	__in PACKAGE_STATE state
	)
{
	if( HasBeenInitialized() )
	{
		bool continueInstall = true;
		if( UsingManagedUX() )
		{
			continueInstall = InvokeOnDetectPackageComplete(wzPackageId,
				hrStatus,
				state);
		}

		if( !continueInstall )
		{
			m_cancelInstall = TRUE;
		}
	}
}


STDMETHODIMP_(void) CChainerUXCore::OnDetectComplete(
	__in HRESULT hrStatus
	)
{
    s_fDetectComplete = TRUE;

	if (HasBeenInitialized())
	{
		bool continueInstall = true;
		if( UsingManagedUX() )
		{
			continueInstall = InvokeOnDetectComplete(hrStatus);
		}

		if( !continueInstall )
		{
			m_cancelInstall = TRUE;
		}
	}
}

STDMETHODIMP_(int) CChainerUXCore::OnPlanBegin(
    __in DWORD cPackages
    )
{
    int result = IDABORT;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }

    if( UsingManagedUX() )
    {
        result = InvokeOnPlanBegin(cPackages);
    }

    if( CancelInstall() )
    {
        result = IDABORT;
    }

    return result;
}


STDMETHODIMP_(int) CChainerUXCore::OnPlanPriorBundle(
    __in_z LPCWSTR wzBundleId,
    __inout_z REQUEST_STATE* pRequestedState
    )
{
    return IDOK;
}

STDMETHODIMP_(int) CChainerUXCore::OnPlanPackageBegin(
    __in_z LPCWSTR wzPackageId,
    __inout_z REQUEST_STATE* pRequestedState
    )
{
    int result = IDABORT;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }

    if( UsingManagedUX() )
    {
        result = InvokeOnPlanPackageBegin(wzPackageId);
    }

    if( CancelInstall() )
    {
        result = IDABORT;
    }
	else if( result == IDOK )
	{
		REQUEST_STATE reqState;
		reqState = GetItemRequestState(wzPackageId);

		if (reqState == REQUEST_STATE_NONE || reqState == REQUEST_STATE_ABSENT || reqState == REQUEST_STATE_PRESENT)
		{
			*pRequestedState = reqState;
		}
	}
	else
	{
		// some error returned, we just pass it up...
	}

    return result;
}


STDMETHODIMP_(void) CChainerUXCore::OnPlanPackageComplete(
    __in LPCWSTR wzPackageId,
    __in HRESULT hrStatus,
    __in PACKAGE_STATE state,
    __in REQUEST_STATE requested,
    __in ACTION_STATE execute,
    __in ACTION_STATE rollback
    )
{
    if( HasBeenInitialized() )
    {
        bool continueInstall = true;
        if( UsingManagedUX() )
        {
            continueInstall = InvokeOnPlanPackageComplete(wzPackageId,
                                                          hrStatus,
                                                          state,
                                                          requested,
                                                          execute,
                                                          rollback);
        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}


STDMETHODIMP_(void) CChainerUXCore::OnPlanComplete(
    __in HRESULT hrStatus
    )
{
    s_fPlanComplete = TRUE;

    if (HasBeenInitialized())
    {
        bool continueInstall = true;
        if( UsingManagedUX() )
        {
            continueInstall = InvokeOnPlanComplete(hrStatus);
        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnApplyBegin()
{
    int result = IDABORT;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }

    if( UsingManagedUX() )
    {
        result = InvokeOnApplyBegin();
    }

    if( CancelInstall() )
    {
        result = IDABORT;
    }

    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnApplyComplete(
    __in HRESULT hrStatus
    )
{
    if( HasBeenInitialized() )
    {
        bool success = false;
        if( UsingManagedUX() )
        {
            success = InvokeOnApplyComplete(hrStatus);
            m_dwErrorCode = hrStatus;
        }
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnProgress(
    __in DWORD dwProgressPercentage,
    __in DWORD dwOverallProgressPercentage
    )
{
    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDERROR;
    }

    if( UsingManagedUX() )
    {
        m_nCmdResult = InvokeProgressMethod(dwProgressPercentage,
                                            dwOverallProgressPercentage);
    }

    if( CancelInstall() )
    {
        m_nCmdResult = IDABORT;
    }

    return m_nCmdResult;
}

STDMETHODIMP_(int) CChainerUXCore::OnCacheBegin()
{
    return IDOK;
}

STDMETHODIMP_(void) CChainerUXCore::OnCacheComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
        bool continueInstall = true;
        if( UsingManagedUX() )
        {
            continueInstall = InvokeOnCacheComplete(hrStatus);
        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnRegisterBegin()
{
    int result = IDOK;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }
    else
    {
        if( UsingManagedUX() )
        {

        }
    }

    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnRegisterComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
        bool continueInstall = true;
        if( UsingManagedUX() )
        {

        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}

STDMETHODIMP_(void) CChainerUXCore::OnUnregisterBegin()
{
}

STDMETHODIMP_(void) CChainerUXCore::OnUnregisterComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
        bool continueInstall = true;
        if( UsingManagedUX() )
        {

        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteBegin(
    __in DWORD cExecutingPackages
    )
{
    int result = IDABORT;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }

    if( UsingManagedUX() )
    {
        result = InvokeOnExecuteBegin(cExecutingPackages);
    }

    if( CancelInstall() )
    {
        result = IDABORT;
    }

    return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnExecutePackageBegin(
    __in LPCWSTR wzPackageId,
    __in BOOL fExecute
    )
{
    int result = IDABORT;

    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDABORT;
    }

    if( UsingManagedUX() )
    {
        result = InvokeOnExecutePackageBegin(wzPackageId, fExecute);
    }

    if( CancelInstall() )
    {
        result = IDABORT;
    }

    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnExecutePackageComplete(
    __in LPCWSTR wzPackageId,
    __in HRESULT hrExitCode
    )
{
    HRESULT hr = S_OK;

    if( HasBeenInitialized() )
    {
        bool continueInstall = true;
        if (UsingManagedUX())
        {
            continueInstall = InvokeOnExecutePackageComplete(wzPackageId, hrExitCode);
        }

        if (m_fVerificationUXRebootInstant && hrExitCode == 0x80070bc2)
        {
            // Prompt user for reboot. If they say yes, do it.
            if (OnRestartRequired())
            {
                hr = m_pCore->Reboot();
                if (FAILED(hr))
                {
                    TraceError(hr, "Failed to reboot");
                    m_cancelInstall = TRUE;
                }
            }
        }

        if( !continueInstall )
        {
            m_cancelInstall = TRUE;
        }
    }
}


STDMETHODIMP_(void) CChainerUXCore::OnExecuteComplete(
    __in HRESULT hrStatus
    )
{
    if( HasBeenInitialized() )
    {
        BOOL success = false;
        if( UsingManagedUX() )
        {
            success = InvokeOnExecuteComplete(hrStatus);
        }
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnError(
    __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
    )
{
    if (0 != dwCode)
    {
        m_dwErrorCode = dwCode;
    }

    return IDABORT; // TODO
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )

{
	return IDOK;
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
{
    return IDOK;
}

STDMETHODIMP_(BOOL) CChainerUXCore::OnRestartRequired()
{
    // If our UX already prompted the user to reboot, don't prompt again when engine asks us to
    // (this is valid engine behavior because it doesn't know if we've already prompted the user or not), so just say yes
    if (m_fRebooted)
    {
        return TRUE;
    }

    if (BURN_RESTART_NEVER != m_command.restart &&
       (BURN_RESTART_PROMPT == m_command.restart || BURN_DISPLAY_FULL == m_command.display)
       )
    {
        int result = ::MessageBox(m_hWnd, L"Setup requires a restart. Restart now?", L"Restart Required", MB_YESNO);

        if (IDYES == result)
        {
            m_fRebooted = true;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if (BURN_RESTART_ALWAYS == m_command.restart)
    {
        m_fRebooted = true;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

STDMETHODIMP_(int) CChainerUXCore::ResolveSource(
        __in    LPCWSTR wzPackageId ,
        __in    LPCWSTR wzPackageOrContainerPath
        )
{
    int result = IDNOACTION;
    WCHAR wzFile[MAX_PATH+1];
    wcsncpy_s(wzFile, countof(wzFile), wzPackageOrContainerPath, _TRUNCATE);

    if( HasBeenInitialized() )
    {
        OPENFILENAMEW ofn;       // common dialog box structure
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.hwndOwner = m_hWnd;
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = wzFile;
        ofn.nMaxFile = countof(wzFile);
        ofn.lpstrFilter = L"All\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = const_cast<LPWSTR>(wcsrchr(wzPackageOrContainerPath, L'\\') + 1);
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = wzPackageOrContainerPath;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display the Open dialog box.

        if (GetOpenFileNameW(&ofn)==TRUE)
        {
            PathRemoveFileSpecW(ofn.lpstrFile);
            if (SUCCEEDED(m_pCore->SetSource(ofn.lpstrFile)))
            {
                return IDOK;
            }
            else
            {
                return IDABORT;
            }
        }
        else
        {
            return IDCANCEL;
        }
    }

    return result;
}

STDMETHODIMP_(BOOL) CChainerUXCore::CanPackagesBeDownloaded()
{
    if (m_command.display == BURN_DISPLAY_PASSIVE || m_command.display == BURN_DISPLAY_NONE)
    {
        return TRUE;
    }

    int result = ::MessageBox(NULL, L"This install needs to connect to the internet to proceed. Do you want to allow the installer to connect to the internet?", L"Internet Connection Confirmation", MB_YESNO);

    if (IDYES == result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnCachePackageBegin(
    __in LPCWSTR wzPackageId,
    __in DWORD64 dw64PackageCacheSize
    )
{
    return IDOK;
}

STDMETHODIMP_(void) CChainerUXCore::OnCachePackageComplete(
    __in LPCWSTR wzPackageId,
    __in HRESULT hrStatus
    )
{
}

STDMETHODIMP_(int) CChainerUXCore::OnDownloadPayloadBegin(
    __in LPCWSTR wzPayloadId,
    __in LPCWSTR wzPayloadFileName
    )
{
    return IDOK;
}

STDMETHODIMP_(void) CChainerUXCore::OnDownloadPayloadComplete(
    __in LPCWSTR wzPayloadId,
    __in LPCWSTR wzPayloadFileName,
    __in HRESULT hrStatus
    )
{
}

STDMETHODIMP_(int) CChainerUXCore::OnDownloadProgress(
    __in DWORD dwProgressPercentage,
    __in DWORD dwOverallPercentage
    )
{
    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDERROR;
    }

    if( UsingManagedUX() )
    {
        m_nCmdResult = InvokeDownloadProgressMethod(dwProgressPercentage,
                                            dwOverallPercentage);
    }

    if( CancelInstall() )
    {
        m_nCmdResult = IDABORT;
    }

    return m_nCmdResult;
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteProgress(
    __in DWORD dwProgressPercentage,
    __in DWORD dwOverallPercentage
    )
{
    if( !HasBeenInitialized() || CancelInstall() )
    {
        return IDERROR;
    }

    if( UsingManagedUX() )
    {
        m_nCmdResult = InvokeExecuteProgressMethod(dwProgressPercentage,
                                            dwOverallPercentage);
    }

    if( CancelInstall() )
    {
        m_nCmdResult = IDABORT;
    }

    return m_nCmdResult;
}

BOOL CChainerUXCore::CancelInstall()
{
    BOOL cancelInstall = m_cancelInstall;

    if (m_cancelInstall)
    {
        m_cancelInstall = FALSE;
        m_installCanceled = TRUE;
    }

    return cancelInstall;
}

BOOL CChainerUXCore::HasBeenInitialized()
{
	return m_initialized == S_OK;
}

BOOL CChainerUXCore::UsingManagedUX()
{
	return true;  // always using the UX, it will hide itself in slient/passive modes.
}

//
// InitializeUX - preps interface based on BURN_DISPLAY setting
//
HRESULT CChainerUXCore::InitializeUX()
{
	HRESULT hr = S_OK;
	LPWSTR sczModuleDirectory = NULL;

	// need to figure out what our working path is relative to the DLL
	WCHAR buffer[MAX_PATH];
	if(::GetModuleFileNameW(m_hModule, buffer, sizeof(buffer)/2))
	{
		hr = PathGetDirectory(buffer, &sczModuleDirectory);
		ExitOnFailure(hr, "Failed to get directory from module");
	}
	else
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unable to set retrieve module path");
	}

	hr = CreateMainWindow();

	if( SUCCEEDED(hr) && UsingManagedUX() )
	{
	    // create our main window (note this may or may not be displayed)
	    hr = InitializeManagedUX(sczModuleDirectory);
        ExitOnFailure(hr, "Unable to create full display UX (managed) window.");
	}

	ExitOnFailure(hr, "Unable to create main window.");

LExit:
	ReleaseStr(sczModuleDirectory);

	return hr;
}

void CChainerUXCore::ShutDownManagedHost()
{
	if( m_pAppDomain )
	{
		m_pClrHost->UnloadDomain(m_pAppDomain);
		ReleaseNullObject(m_pAppDomain);
	}

	if( m_pClrHost )
	{
		m_pClrHost->Stop();
		ReleaseNullObject(m_pClrHost);
	}
}

HRESULT CChainerUXCore::InitializeManagedUX(LPWSTR sczModuleDirectory)
{
	if( m_pClrHost ) return S_OK;
	HRESULT hr = S_OK;
	IUnknown *pIUnknown = NULL;
	IAppDomainSetup *pAppDomainSetup = NULL;
	BSTR appBase = NULL;

	ExitOnNull(sczModuleDirectory, hr, E_INVALIDARG, "Received NULL module directory path.");

	hr = CorBindToRuntimeEx(
		DOTNET20,
		WORKSTATION_BUILD, // because this runs on end-user PCs, not servers
		STARTUP_CONCURRENT_GC|STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN,
		CLSID_CorRuntimeHost,
		IID_ICorRuntimeHost,
		(void**)&m_pClrHost);
	ExitOnFailure(hr, "Unable to initialize CLR for UX (managed) window.");

	hr = m_pClrHost->Start();
	ExitOnFailure(hr, "Unable to start hosted CLR for UX (managed) window.");

	hr = m_pClrHost->CreateDomainSetup(&pIUnknown);
	ExitOnFailure(hr, "Unable to get create domain setup for hosted CLR.");

	hr = pIUnknown->QueryInterface(__uuidof(IAppDomainSetup),
		(void**) &pAppDomainSetup);
	ReleaseNullObject(pIUnknown);

	ExitOnFailure(hr, "Unable to retrieve IAppDomainSetup pointer for hosted CLR.");

	appBase = ::SysAllocString(sczModuleDirectory);
	ExitOnNull(appBase, hr, E_OUTOFMEMORY, "Unable to set ApplicationBase: Out of memory");

	hr = pAppDomainSetup->put_ApplicationBase(appBase);
	ExitOnFailure(hr, "Unable to set ApplicationBase.");

	hr = m_pClrHost->CreateDomainEx(L"ManagedInstallUX", pAppDomainSetup, NULL, &pIUnknown);
	ExitOnFailure(hr, "Unable to get create domain for hosted CLR.");

	hr = pIUnknown->QueryInterface(__uuidof(_AppDomain),
		(void**) &m_pAppDomain);
	ReleaseNullObject(pIUnknown);
	ExitOnFailure(hr, "Unable to retrieve _AppDomain pointer for hosted CLR.");

	if( !GetMethods(m_pAppDomain) )
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unable to retrieve pointers to managed methods.");
	}

	if( !InvokeInitializeMethod(sczModuleDirectory) )
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unable to initialize managed UX.");
	}

LExit:
	ReleaseBSTR(appBase)
		ReleaseNullObject(pAppDomainSetup);

	if( FAILED(hr) )
	{
		ShutDownManagedHost();
	}

	return hr;
}

int CChainerUXCore::InvokeOnDetectBegin(DWORD numPackages)
{
	if( !m_managedMethods[MUX_OnDetectBegin] )
	{
		return IDABORT;
	}

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

	// 1st arg
	VARIANT vNumPackages;
	vNumPackages.vt = VT_UI4;
	vNumPackages.ulVal = (ULONG)numPackages;
	LONG index = 0;
	hr = SafeArrayPutElement(saArgs, &index, &vNumPackages);
	ExitOnFailure(hr, "Unable to add vNumPackages to MUX_OnDetectBegin arg list.");

	hr = m_managedMethods[MUX_OnDetectBegin]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_OnDetectBegin().");

LExit:
	SafeArrayDestroy(saArgs);

	int result;
	if (SUCCEEDED(hr))
	{
		result = vResult.intVal;
	}
	else
	{
		result = IDABORT;
	}

	return result;
}
int CChainerUXCore::InvokeOnDetectPackageBegin(LPCWSTR wzPackageId)
{
	if( !m_managedMethods[MUX_OnDetectPackageBegin] )
	{
		return IDABORT;
	}

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

	// 1st arg
	VARIANT vPackageID;
	vPackageID.vt = VT_BSTR;
	vPackageID.bstrVal = ::SysAllocString(wzPackageId);
	if (vPackageID.bstrVal == NULL)
	{
		hr = E_OUTOFMEMORY;
		ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnDetectPackageBegin.");
	}
	LONG index = 0;
	hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
	ExitOnFailure(hr, "Unable to add vPackageID to MUX_OnDetectPackageBegin arg list.");

	hr = m_managedMethods[MUX_OnDetectPackageBegin]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_OnDetectPackageBegin().");

LExit:
	SafeArrayDestroy(saArgs);

	int result;
	if (SUCCEEDED(hr))
	{
		result = vResult.intVal;
	}
	else
	{
		result = IDABORT;
	}

	return result;
}

bool CChainerUXCore::InvokeOnDetectPackageComplete(LPCWSTR wzPackageId,
												   HRESULT hrStatus,
												   PACKAGE_STATE state)
{
	if( !m_managedMethods[MUX_OnDetectPackageComplete] )
	{
		return false;
	}

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 3);

	// 1st arg
	VARIANT vPackageID;
	vPackageID.vt = VT_BSTR;
	vPackageID.bstrVal = ::SysAllocString(wzPackageId);
	if (vPackageID.bstrVal == NULL)
	{
		hr = E_OUTOFMEMORY;
		ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnDetectPackageComplete.");
	}
	LONG index = 0;
	hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
	ExitOnFailure(hr, "Unable to add vPackageID to MUX_OnDetectPackageComplete arg list.");

	// 2nd arg
	VARIANT vHrStatus;
	vHrStatus.vt = VT_I4;
	vHrStatus.lVal = (LONG)hrStatus;
	index = 1;
	hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
	ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnDetectPackageComplete arg list.");

	// 3rd arg
	VARIANT vCurPackageState;
	vCurPackageState.vt = VT_I4;
	vCurPackageState.lVal = (LONG)state;
	index = 2;
	hr = SafeArrayPutElement(saArgs, &index, &vCurPackageState);
	ExitOnFailure(hr, "Unable to add vCurPackageState to MUX_OnDetectPackageComplete arg list.");

	hr = m_managedMethods[MUX_OnDetectPackageComplete]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_OnDetectPackageComplete().");

LExit:
	SafeArrayDestroy(saArgs);

	if (SUCCEEDED(hr))
	{
		bool result;
		if (vResult.boolVal == VARIANT_TRUE)
		{
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}
	else
	{
		return false;
	}
}

bool CChainerUXCore::InvokeOnDetectComplete(HRESULT hrStatus)
{
	if( !m_managedMethods[MUX_OnDetectComplete] )
	{
		return false;
	}

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

	// 1st arg
	VARIANT vHrStatus;
	vHrStatus.vt = VT_I4;
	vHrStatus.lVal = (LONG)hrStatus;
	LONG index = 0;
	hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
	ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnDetectComplete arg list.");

	hr = m_managedMethods[MUX_OnDetectComplete]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_OnDetectComplete().");

LExit:
	SafeArrayDestroy(saArgs);

	if (SUCCEEDED(hr))
	{
		bool result;
		if (vResult.boolVal == VARIANT_TRUE)
		{
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}
	else
	{
		return false;
	}
}

int CChainerUXCore::InvokeOnPlanBegin(DWORD numPackages)
{
    if( !m_managedMethods[MUX_OnPlanBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vNumPackages;
    vNumPackages.vt = VT_UI4;
    vNumPackages.ulVal = (ULONG)numPackages;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vNumPackages);
    ExitOnFailure(hr, "Unable to add vNumPackages to MUX_OnPlanBegin arg list.");

    hr = m_managedMethods[MUX_OnPlanBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnPlanBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

int CChainerUXCore::InvokeOnPlanPackageBegin(LPCWSTR wzPackageId)
{
    if( !m_managedMethods[MUX_OnPlanPackageBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vPackageID;
    vPackageID.vt = VT_BSTR;
    vPackageID.bstrVal = ::SysAllocString(wzPackageId);
    if (vPackageID.bstrVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnPlanPackageBegin.");
    }
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
    ExitOnFailure(hr, "Unable to add vPackageID to MUX_OnPlanPackageBegin arg list.");

    hr = m_managedMethods[MUX_OnPlanPackageBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnPlanPackageBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

REQUEST_STATE CChainerUXCore::GetItemRequestState(LPCWSTR wzPackageId)
{
	LPWSTR pszAppData = (LPWSTR)new TCHAR [MAX_PATH];
	DWORD numChars = ::ExpandEnvironmentStrings(L"%AppData%",  pszAppData, MAX_PATH + 1);
	LPWSTR pszPackageId = NULL;
	BSTR bs2;
	HRESULT hr = S_OK;

	hr = StrAllocString(&pszPackageId, wzPackageId, 0);
	ExitOnFailure(hr, "Failed to allocate string for package ID");

    hr = EscapeStringXml(&pszPackageId);
    ExitOnFailure1(hr, "Failed to escape string for XML:%S", pszPackageId);

	bs2 = SysAllocString(L"./ItemPlan/Item[@Id='']");
	//bs2 += pszPackageId; // TODO:
	//bs2 += L"']";

	LPWSTR sczFullPath = NULL;
	LPCWSTR path = pszAppData;
	hr = PathConcat(path, L"ItemPlan.xml", &sczFullPath);
    ExitOnFailure(hr, "Unable to build path to ItemPlan.xml.");

	IXMLDOMDocument* pXmlDoc = NULL;
	// Initialize XML util
    hr = XmlInitialize();
    ExitOnFailure(hr, "Failed to initialize XML util.");
	hr = XmlLoadDocumentFromFile(sczFullPath, &pXmlDoc);
    ExitOnFailure(hr, "Unable to load ItemPlan.xml file.");

	IXMLDOMNode* itemNode = NULL;
	hr = pXmlDoc->selectSingleNode(bs2, &itemNode);
	if (S_FALSE == hr)
	{
		hr = E_NOTFOUND;
	}
	ExitOnFailure(hr, "Unable to get Item node");

	LPWSTR sczRequestState = NULL;
	XmlGetAttribute(itemNode, L"RequestState", &sczRequestState);


	if (m_command.action == BURN_ACTION_INSTALL &&
		(m_command.display == BURN_DISPLAY_FULL || m_command.display == BURN_DISPLAY_PASSIVE  || m_command.display == BURN_DISPLAY_NONE ))
	{
		if (wcscmp(sczRequestState, L"ABSENT") == 0)
		{
			return 	REQUEST_STATE_ABSENT;
		}

		if (wcscmp(sczRequestState, L"PRESENT") == 0)
		{
			return 	REQUEST_STATE_PRESENT;
		}

		if (wcscmp(sczRequestState, L"NONE") == 0)
		{
			return	REQUEST_STATE_NONE;
		}

	}
	else if ( (m_command.action == BURN_ACTION_UNINSTALL || m_command.action == BURN_ACTION_REPAIR) &&
		(m_command.display == BURN_DISPLAY_FULL || m_command.display == BURN_DISPLAY_PASSIVE  || m_command.display == BURN_DISPLAY_NONE ) )
	{
		if (wcscmp(sczRequestState, L"NONE") == 0)
		{
			return	REQUEST_STATE_NONE;
		}
	}

	LExit:
	ReleaseObject(pXmlDoc);
	ReleaseStr(sczFullPath);
	ReleaseStr(pszPackageId);
	SysFreeString(bs2);

	return REQUEST_STATE_CACHE;
}

bool CChainerUXCore::InvokeOnPlanPackageComplete(LPCWSTR wzPackageId,
                                 HRESULT hrStatus,
                                 PACKAGE_STATE state,
                                 REQUEST_STATE requested,
                                 ACTION_STATE execute,
                                 ACTION_STATE rollback)
{
    if( !m_managedMethods[MUX_OnPlanPackageComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 6);

    // 1st arg
    VARIANT vPackageID;
    vPackageID.vt = VT_BSTR;
    vPackageID.bstrVal = ::SysAllocString(wzPackageId);
    if (vPackageID.bstrVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnPlanPackageComplete.");
    }
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
    ExitOnFailure(hr, "Unable to add vPackageID to MUX_OnPlanPackageComplete arg list.");

    // 2nd arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnPlanPackageComplete arg list.");

    // 3rd arg
    VARIANT vCurPackageState;
    vCurPackageState.vt = VT_I4;
    vCurPackageState.lVal = (LONG)state;
    index = 2;
    hr = SafeArrayPutElement(saArgs, &index, &vCurPackageState);
    ExitOnFailure(hr, "Unable to add vCurPackageState to MUX_OnPlanPackageComplete arg list.");

    // 4th arg
    VARIANT vReqPackageState;
    vReqPackageState.vt = VT_I4;
    vReqPackageState.lVal = (LONG)requested;
    index = 3;
    hr = SafeArrayPutElement(saArgs, &index, &vReqPackageState);
    ExitOnFailure(hr, "Unable to add vReqPackageState to MUX_OnPlanPackageComplete arg list.");

    // 5th arg
    VARIANT vExecuteState;
    vExecuteState.vt = VT_I4;
    vExecuteState.lVal = (LONG)execute;
    index = 4;
    hr = SafeArrayPutElement(saArgs, &index, &vExecuteState);
    ExitOnFailure(hr, "Unable to add vExecuteState to MUX_OnPlanPackageComplete arg list.");

    // 6th arg
    VARIANT vRollbackState;
    vRollbackState.vt = VT_I4;
    vRollbackState.lVal = (LONG)rollback;
    index = 5;
    hr = SafeArrayPutElement(saArgs, &index, &vRollbackState);
    ExitOnFailure(hr, "Unable to add vRollbackState to MUX_OnPlanPackageComplete arg list.");

    hr = m_managedMethods[MUX_OnPlanPackageComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnPlanPackageComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

bool CChainerUXCore::InvokeOnPlanComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnPlanComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnPlanComplete arg list.");

    hr = m_managedMethods[MUX_OnPlanComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnPlanComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

int CChainerUXCore::InvokeOnExecutePackageBegin(LPCWSTR wzPackageId, BOOL fExecute)
{
    if( !m_managedMethods[MUX_OnExecutePackageBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2);

    // 1st arg
    VARIANT vPackageID;
    vPackageID.vt = VT_BSTR;
    vPackageID.bstrVal = ::SysAllocString(wzPackageId);
    if (vPackageID.bstrVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnExecutePackageBegin.");
    }
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
    ExitOnFailure(hr, "Unable to add vPackageID to MUX_OnExecutePackageBegin arg list.");

    // 2nd arg
    VARIANT vInstall;
    vInstall.vt = VT_BOOL;
    vInstall.boolVal = (VARIANT_BOOL)fExecute;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vInstall);
    ExitOnFailure(hr, "Unable to add vInstall to MUX_OnExecutePackageBegin arg list.");

    hr = m_managedMethods[MUX_OnExecutePackageBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnExecutePackageBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

bool CChainerUXCore::InvokeOnExecutePackageComplete(LPCWSTR wzPackageId, HRESULT hrExitCode)
{
    if( !m_managedMethods[MUX_OnExecutePackageComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2);

    // 1st arg
    VARIANT vPackageID;
    vPackageID.vt = VT_BSTR;
    vPackageID.bstrVal = ::SysAllocString(wzPackageId);
    if (vPackageID.bstrVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        ExitOnFailure(hr, "Unable to allocate memory for wzPackageId arg for MUX_OnExecutePackageComplete.");
    }
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vPackageID);
    ExitOnFailure(hr, "Unable to add packageID to MUX_OnExecutePackageComplete arg list.");

    // 2nd arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrExitCode;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add hrExitCode to MUX_OnExecutePackageComplete arg list.");

    hr = m_managedMethods[MUX_OnExecutePackageComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnExecutePackageComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    bool success;
    if (SUCCEEDED(hr))
    {
        if (vResult.boolVal == VARIANT_TRUE)
        {
            success = true;
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    return success;
}

int CChainerUXCore::InvokeOnRegisterBegin()
{
    if( !m_managedMethods[MUX_OnRegisterBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

    hr = m_managedMethods[MUX_OnRegisterBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnRegisterBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

bool CChainerUXCore::InvokeOnRegisterComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnRegisterComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnRegisterComplete arg list.");

    hr = m_managedMethods[MUX_OnRegisterComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnRegisterComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

bool CChainerUXCore::InvokeOnUnregisterBegin()
{
    if( !m_managedMethods[MUX_OnUnregisterBegin] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

    hr = m_managedMethods[MUX_OnUnregisterBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnUnregisterBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

bool CChainerUXCore::InvokeOnUnregisterComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnUnregisterComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnUnregisterComplete arg list.");

    hr = m_managedMethods[MUX_OnUnregisterComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnUnregisterComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

int CChainerUXCore::InvokeOnApplyBegin()
{
    if( !m_managedMethods[MUX_OnApplyBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

    hr = m_managedMethods[MUX_OnApplyBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnApplyBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

int CChainerUXCore::InvokeProgressMethod(DWORD dwProgressPercentage,
                         DWORD dwOverallProgressPercentage)
{
    if( !m_managedMethods[MUX_OnProgress] )
    {
        return IDERROR;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2);

    VARIANT vProgress;
    vProgress.vt = VT_I4;
    vProgress.lVal = (LONG)dwProgressPercentage;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vProgress);
    ExitOnFailure(hr, "Unable to add vProgress to MUX_OnProgress arg list.");

    VARIANT vOverallProgress;
    vOverallProgress.vt = VT_I4;
    vOverallProgress.lVal = (LONG)dwOverallProgressPercentage;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vOverallProgress);
    ExitOnFailure(hr, "Unable to add vOverallProgress to MUX_OnProgress arg list.");

    hr = m_managedMethods[MUX_OnProgress]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnProgress().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        return vResult.intVal;
    }
    else
    {
        return IDERROR;
    }
}

int CChainerUXCore::InvokeDownloadProgressMethod(DWORD dwProgressPercentage,
                         DWORD dwOverallProgressPercentage)
{
    if( !m_managedMethods[MUX_OnDownloadProgress] )
    {
        return IDERROR;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2);

    VARIANT vProgress;
    vProgress.vt = VT_I4;
    vProgress.lVal = (LONG)dwProgressPercentage;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vProgress);
    ExitOnFailure(hr, "Unable to add vProgress to MUX_OnDownloadProgress arg list.");

    VARIANT vOverallProgress;
    vOverallProgress.vt = VT_I4;
    vOverallProgress.lVal = (LONG)dwOverallProgressPercentage;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vOverallProgress);
    ExitOnFailure(hr, "Unable to add vOverallProgress to MUX_OnDownloadProgress arg list.");

    hr = m_managedMethods[MUX_OnDownloadProgress]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnProgress().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        return vResult.intVal;
    }
    else
    {
        return IDERROR;
    }
}

int CChainerUXCore::InvokeExecuteProgressMethod(DWORD dwProgressPercentage,
                         DWORD dwOverallProgressPercentage)
{
    if( !m_managedMethods[MUX_OnExecuteProgress] )
    {
        return IDERROR;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2);

    VARIANT vProgress;
    vProgress.vt = VT_I4;
    vProgress.lVal = (LONG)dwProgressPercentage;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vProgress);
    ExitOnFailure(hr, "Unable to add vProgress to MUX_OnExecuteProgress arg list.");

    VARIANT vOverallProgress;
    vOverallProgress.vt = VT_I4;
    vOverallProgress.lVal = (LONG)dwOverallProgressPercentage;
    index = 1;
    hr = SafeArrayPutElement(saArgs, &index, &vOverallProgress);
    ExitOnFailure(hr, "Unable to add vOverallProgress to MUX_OnExecuteProgress arg list.");

    hr = m_managedMethods[MUX_OnExecuteProgress]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnProgress().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        return vResult.intVal;
    }
    else
    {
        return IDERROR;
    }
}
bool CChainerUXCore::InvokeOnApplyComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnApplyComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add hrStatus to MUX_OnApplyComplete() arg list.");

    hr = m_managedMethods[MUX_OnApplyComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnApplyComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    bool success;
    if (SUCCEEDED(hr))
    {
        if (vResult.boolVal == VARIANT_TRUE)
        {
            success = true;
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    return success;
}

void CChainerUXCore::InvokeUninitializeMethod()
{
	if( !m_managedMethods[MUX_Uninitialize] )
	{
		return;
	}

	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

	HRESULT hr = m_managedMethods[MUX_Uninitialize]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_Uninitialize().");

LExit:
	SafeArrayDestroy(saArgs);
}

int CChainerUXCore::InvokeOnExecuteBegin(DWORD cExecutingPackages)
{
    if( !m_managedMethods[MUX_OnExecuteBegin] )
    {
        return IDABORT;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vNumPackages;
    vNumPackages.vt = VT_UI4;
    vNumPackages.ulVal = (ULONG)cExecutingPackages;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vNumPackages);
    ExitOnFailure(hr, "Unable to add vNumPackages to MUX_OnExecuteBegin arg list.");

    hr = m_managedMethods[MUX_OnExecuteBegin]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnExecuteBegin().");

LExit:
    SafeArrayDestroy(saArgs);

    int result;
    if (SUCCEEDED(hr))
    {
        result = vResult.intVal;
    }
    else
    {
        result = IDABORT;
    }

    return result;
}

bool CChainerUXCore::InvokeRunMethod()
{
	if( !m_managedMethods[MUX_Run] )
	{
		return false;
	}

	VARIANT vResult;
	VariantInit(&vResult);

	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

	HRESULT hr = m_managedMethods[MUX_Run]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_Run().");

LExit:
	SafeArrayDestroy(saArgs);

	if (SUCCEEDED(hr))
	{
		bool result;
		if (vResult.boolVal == VARIANT_TRUE)
		{
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}
	else
	{
		return false;
	}
}

bool CChainerUXCore::InvokeOnExecuteComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnExecuteComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add hrStatus to MUX_OnExecuteComplete() arg list.");

    hr = m_managedMethods[MUX_OnExecuteComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnExecuteComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    bool success;
    if (SUCCEEDED(hr))
    {
        if (vResult.boolVal == VARIANT_TRUE)
        {
            success = true;
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    return success;
}

bool CChainerUXCore::InvokeInitializeMethod(LPWSTR sczWorkingDirectory)
{

	if( !m_managedMethods[MUX_Initialize] ||
		sczWorkingDirectory == NULL)
	{
		return false;
	}

	INSTALL_MODE mode;
	if( m_command.display == BURN_DISPLAY_FULL )
	{
		if( m_command.action == BURN_ACTION_INSTALL )
		{
			mode = INSTALL_FULL_DISPLAY;
		}
		else if( m_command.action == BURN_ACTION_UNINSTALL )
		{
			mode = UNINSTALL_FULL_DISPLAY;
		}
		else if( m_command.action == BURN_ACTION_REPAIR )
        {
            mode = REPAIR_FULL_DISPLAY;
        }
		else
		{
			return false;
		}
	}
	else if( ( m_command.display == BURN_DISPLAY_PASSIVE ) ||
		     ( m_command.display == BURN_DISPLAY_NONE ) )
	{
		if( m_command.action == BURN_ACTION_INSTALL )
		{
			mode = INSTALL_MIN_DISPLAY;
		}
		else if( m_command.action == BURN_ACTION_REPAIR )
        {
            mode = REPAIR_MIN_DISPLAY;
        }
		else if( m_command.action == BURN_ACTION_UNINSTALL )
		{
			mode = UNINSTALL_MIN_DISPLAY;
		}
		else
		{
			return false;
		}
	}

	else
	{
		return false;
	}

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);
	LONG index = 0;
	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 5);

	// 1st arg
    VARIANT vMainHWND;
    vMainHWND.vt = VT_I4;
    vMainHWND.lVal = (LONG)m_hWnd;
    index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vMainHWND);
    ExitOnFailure(hr, "Unable to add main window handle to MUX_Initialize() arg list.");

	// 2nd arg
	VARIANT vInstallMode;
	vInstallMode.vt = VT_UI4;
	vInstallMode.ulVal = (ULONG)mode;
	index = 1;
	hr = SafeArrayPutElement(saArgs, &index, &vInstallMode);
	ExitOnFailure(hr, "Unable to add install mode to MUX_Initialize() arg list.");

	// 3rd arg
	VARIANT vWorkingDir;
	vWorkingDir.vt = VT_BSTR;
	vWorkingDir.bstrVal = ::SysAllocString(sczWorkingDirectory);
	if (vWorkingDir.bstrVal == NULL)
	{
		hr = E_OUTOFMEMORY;
		ExitOnFailure(hr, "Unable to allocate memory for working directory arg for MUX_Initialize().");
	}
	index = 2;
	hr = SafeArrayPutElement(saArgs, &index, &vWorkingDir);
	ExitOnFailure(hr, "Unable to add working directory to MUX_Initialize() arg list.");

	// 4th arg
	VARIANT vSetupAssembly;
	vSetupAssembly.vt = VT_BSTR;
	vSetupAssembly.bstrVal = ::SysAllocString(m_managedSetupAssembly);
	if (vSetupAssembly.bstrVal == NULL)
	{
		hr = E_OUTOFMEMORY;
		ExitOnFailure(hr, "Unable to allocate memory for managed setup assembly name arg for MUX_Initialize().");
	}
	index = 3;
	hr = SafeArrayPutElement(saArgs, &index, &vSetupAssembly);
	ExitOnFailure(hr, "Unable to add managed setup assembly name to MUX_Initialize() arg list.");

	// 5th arg
	VARIANT vSetupClass;
	vSetupClass.vt = VT_BSTR;
	vSetupClass.bstrVal = ::SysAllocString(m_managedSetupClass);
	if (vSetupClass.bstrVal == NULL)
	{
		hr = E_OUTOFMEMORY;
		ExitOnFailure(hr, "Unable to allocate memory for managed setup class name arg for MUX_Initialize().");
	}
	index = 4;
	hr = SafeArrayPutElement(saArgs, &index, &vSetupClass);
	ExitOnFailure(hr, "Unable to add managed setup class name to MUX_Initialize() arg list.");

	// okay, now invoke Initialize
	hr = m_managedMethods[MUX_Initialize]->Invoke_3(vNull, saArgs, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_Initialize().");

LExit:
	SafeArrayDestroy(saArgs);

	if (SUCCEEDED(hr))
	{
		bool result;
		if (vResult.boolVal == VARIANT_TRUE)
		{
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}
	else
	{
		return false;
	}
}

bool CChainerUXCore::GetMethods(_AppDomain* const pAppDomain)
{
	HRESULT hr = S_OK;
	BSTR bstrAssemblyName = NULL;
	BSTR bstrClassName = NULL;
	_Assembly* pAssembly = NULL;

	ExitOnNull(m_managedProxyAssembly, hr, E_POINTER, "Managed Proxy assembly pointer not set.");
	ExitOnNull(m_managedProxyClass, hr, E_POINTER, "Managed Proxy class  pointer not set.");
	ExitOnNull(pAppDomain, hr, E_INVALIDARG, "Did not receive a loaded AppDomain");

	bstrAssemblyName = ::SysAllocString(m_managedProxyAssembly);
	ExitOnNull(bstrAssemblyName, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for assembly name");

	bstrClassName = ::SysAllocString(m_managedProxyClass);
	ExitOnNull(bstrClassName, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for managed class name");

	hr = pAppDomain->Load_2(bstrAssemblyName, &pAssembly);
	ExitOnFailure(hr, "Unable to load Assembly");

	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		if( !GetMethod(pAssembly,
			&bstrClassName,
			UXFunctionProxyNames[i],
			&m_managedMethods[i]) )
		{
			ExitOnFailure1(hr = E_UNEXPECTED, "Unable to obtain pointer to managed %s function.", UXFunctionProxyNames[i]);
		}
	}

LExit:
	ReleaseBSTR(bstrAssemblyName);
	ReleaseBSTR(bstrClassName);
	ReleaseObject(pAssembly);

	return SUCCEEDED(hr);
}

bool CChainerUXCore::GetMethod(_Assembly* const pAssembly,
							   const BSTR* const bstrClass,
							   const WCHAR* const szMethod,
							   _MethodInfo** ppMethod)
{
	HRESULT hr = S_OK;
	_Type* pType = NULL;
	BSTR bstrMethod = NULL;

	ExitOnNull(pAssembly, hr, E_INVALIDARG, "Did not receive a loaded Assembly.");
	ExitOnNull(bstrClass, hr, E_INVALIDARG, "Did not receive a Proxy Class name.");
	ExitOnNull(szMethod, hr, E_INVALIDARG, "Did not receive a Method name.");

	hr = pAssembly->GetType_2(*bstrClass, &pType);
	ExitOnFailure(hr, "Unable to get proxy class from managed assembly");
	ExitOnNull(pType, hr, E_POINTER, "Unable to get proxy class from managed assembly");

	bstrMethod = ::SysAllocString(szMethod);
	ExitOnNull(bstrMethod, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for proxy function name");

	hr = pType->GetMethod_2(bstrMethod,
		(BindingFlags) (BindingFlags_Public | BindingFlags_Static),
		ppMethod);
	ExitOnFailure(hr, "Unable to get proxy class method from managed assembly");
	ExitOnNull(*ppMethod, hr, E_POINTER, "Unable to get proxy class method from managed assembly");

LExit:
	ReleaseObject(pType);
	ReleaseBSTR(bstrMethod);

	return SUCCEEDED(hr);
}

bool CChainerUXCore::InvokeOnCacheComplete(HRESULT hrStatus)
{
    if( !m_managedMethods[MUX_OnCacheComplete] )
    {
        return false;
    }

    HRESULT hr;
    VARIANT vResult;
    VariantInit(&vResult);

    VARIANT vNull;
    vNull.vt = VT_EMPTY;

    SAFEARRAY* saArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    // 1st arg
    VARIANT vHrStatus;
    vHrStatus.vt = VT_I4;
    vHrStatus.lVal = (LONG)hrStatus;
    LONG index = 0;
    hr = SafeArrayPutElement(saArgs, &index, &vHrStatus);
    ExitOnFailure(hr, "Unable to add vHrStatus to MUX_OnCacheComplete arg list.");

    hr = m_managedMethods[MUX_OnCacheComplete]->Invoke_3(vNull, saArgs, &vResult);
    ExitOnFailure(hr, "Unable to invoke MUX_OnCacheComplete().");

LExit:
    SafeArrayDestroy(saArgs);

    if (SUCCEEDED(hr))
    {
        bool result;
        if (vResult.boolVal == VARIANT_TRUE)
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    else
    {
        return false;
    }
}

//
// OnSuspend - suspend the install
//
void CChainerUXCore::OnSuspend()
{
	HRESULT hr = S_OK;

	hr = m_pCore->Suspend();
	ExitOnFailure(hr, "Failed to Suspend install.");

LExit:
	return;
}

//
// OnDetect - determine the current state the of packages.
//
void CChainerUXCore::OnDetect()
{
	HRESULT hr = S_OK;

	hr = m_pCore->Detect();
	ExitOnFailure(hr, "Failed to Detect chain.");

LExit:
	return;
}


void CChainerUXCore::UninitializeManagedUX()
{
	InvokeUninitializeMethod();

	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		ReleaseNullObject(m_managedMethods[i]);
	}

	// once the managed object has been cleaned up,
	// shutdown the host
	ShutDownManagedHost();
}


//
// Constructors
//

CChainerUXCore::CChainerUXCore(
							   __in HMODULE hModule,
							   __in BURN_COMMAND* pCommand,
							   __in LPCWSTR managedSetupAssembly,
							   __in LPCWSTR managedSetupClass
							   )
{
	Init(hModule, pCommand, managedSetupAssembly, managedSetupClass);
}

CChainerUXCore::CChainerUXCore(HMODULE hModule,
							   BURN_COMMAND* pCommand,
							   LPCWSTR managedProxyAssembly,
							   LPCWSTR managedProxyClass,
							   LPCWSTR managedSetupAssembly,
							   LPCWSTR managedSetupClass
							   )
{
	Init(hModule, pCommand, managedProxyAssembly, managedProxyClass, managedSetupAssembly, managedSetupClass);
}

//
// Init (constructor helper) - intitialize member variables.
//
void CChainerUXCore::Init(
						  __in HMODULE hModule,
						  __in BURN_COMMAND* pCommand,
						  __in LPCWSTR managedSetupAssembly,
						  __in LPCWSTR managedSetupClass
						  )
{
	Init(hModule,
		pCommand,
		L"UXManagedProxy",
		L"ManagedSetupUX.ManagedSetupUXProxy",
		managedSetupAssembly,
		managedSetupClass
		);
}

void CChainerUXCore::Init(
						  __in HMODULE hModule,
						  __in BURN_COMMAND* pCommand,
						  __in LPCWSTR managedProxyAssembly,
						  __in LPCWSTR managedProxyClass,
						  __in LPCWSTR managedSetupAssembly,
						  __in LPCWSTR managedSetupClass

						  )
{
	m_initialized = ERROR_APP_INIT_FAILURE;

	m_hUiThread = NULL;
	m_pClrHost = NULL;
	m_pAppDomain = NULL;
	m_dwErrorCode = 0;

	m_hModule = hModule;
	memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BURN_COMMAND));

	m_pCore = NULL;
	m_hWnd = NULL;
	m_installCanceled = FALSE;
	m_cancelInstall = FALSE;
	m_nCmdResult = IDNOACTION;
	m_cref = 1;

	if (managedProxyAssembly)
	{
		m_managedProxyAssembly = managedProxyAssembly;
	}
	else
	{
		m_managedProxyAssembly = NULL;
	}

	if (managedProxyClass)
	{
		m_managedProxyClass = managedProxyClass;
	}
	else
	{
		m_managedProxyClass = NULL;
	}

	if (managedSetupAssembly)
	{
		m_managedSetupAssembly = managedSetupAssembly;
	}
	else
	{
		m_managedSetupAssembly = NULL;
	}

	if (managedSetupClass)
	{
		m_managedSetupClass = managedSetupClass;
	}
	else
	{
		m_managedSetupClass = NULL;
	}

	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		m_managedMethods[i] = NULL;
	}
}

//
// Destructor - release member variables.
//
CChainerUXCore::~CChainerUXCore()
{
	if( UsingManagedUX() )
	{
		UninitializeManagedUX();
	}
}


BOOL RequiredDotNetVersionFound()
{
	BOOL found = FALSE;
	HKEY hKey;

	if( ERROR_SUCCESS == ::RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP",
		0, KEY_READ, &hKey) )
	{
		DWORD dwIndex = 0;
		const int BUF_LEN = 256;
		WCHAR buffer[BUF_LEN];

		while( !found )
		{
			DWORD len = BUF_LEN;
			for (int i = 0; i < BUF_LEN; ++i)
			{
				buffer[i] = 0;
			}

			if( ERROR_SUCCESS == ::RegEnumKeyEx(
				hKey,
				dwIndex,
				buffer,
				&len,
				NULL,
				NULL,
				NULL,
				NULL) )
			{
				if( len > 0 && lstrlenW(buffer) > 0 )
				{
					if( lstrcmpW(buffer, DOTNET20) == 0 )
					{
						found = TRUE;
						break;
					}
				}
			}
			else
			{
				break;
			}

			dwIndex++;
		}
	}

	return found;
}

BOOL ManagedDependenciesInstalled()
{
	if( !RequiredDotNetVersionFound() )
	{
		return FALSE;
	}

	return TRUE;
}

HRESULT EscapeStringXml(LPWSTR *ppszString)
{
    HRESULT hr = S_OK;
	LPWSTR pszPackageId = NULL;

	// Escape backslashes in the ID first, before we escape other characters
	hr = StrReplaceStringAll(ppszString, L"\\", L"\\\\");
	ExitOnFailure1(hr, "Failed to escape backslashes in string: %S", pszPackageId);

	// Escape single quotes in the ID
	hr = StrReplaceStringAll(ppszString, L"'", L"\\'");
	ExitOnFailure1(hr, "Failed to escape backslashes in string: %S", pszPackageId);

	// Escape double quotes in the ID
	hr = StrReplaceStringAll(ppszString, L"\"", L"\\\"");
	ExitOnFailure1(hr, "Failed to escape backslashes in string: %S", pszPackageId);

LExit:
    return S_OK;
}
