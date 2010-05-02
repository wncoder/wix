//-------------------------------------------------------------------------------------------------
// <copyright file="StandardUserExperience.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//-------------------------------------------------------------------------------------------------


#include "precomp.h"

static const LPCWSTR STDUX_WINDOW_CLASS = L"WixBurnStdux";

enum STDUX_STATE
{
    STDUX_STATE_INITIALIZING,
    STDUX_STATE_DETECTING,
    STDUX_STATE_DETECTED,
    STDUX_STATE_PLANNING,
    STDUX_STATE_PLANNED,
    STDUX_STATE_APPLYING,
    STDUX_STATE_CACHING,
    STDUX_STATE_CACHED,
    STDUX_STATE_EXECUTING,
    STDUX_STATE_EXECUTED,
    STDUX_STATE_APPLIED,
    STDUX_STATE_RESTART,
    STDUX_STATE_ERROR,
};


enum STDUX_CONTROL
{
    STDUX_CONTROL_TOU_LINK,
    STDUX_CONTROL_INSTALL_BUTTON,
    STDUX_CONTROL_REPAIR_BUTTON,
    STDUX_CONTROL_UNINSTALL_BUTTON,
    STDUX_CONTROL_CLOSE_BUTTON,
    STDUX_CONTROL_OK_BUTTON,
    STDUX_CONTROL_CANCEL_BUTTON,
    STDUX_CONTROL_RETRY_BUTTON,
    STDUX_CONTROL_ABORT_BUTTON,
    STDUX_CONTROL_IGNORE_BUTTON,
    STDUX_CONTROL_YES_BUTTON,
    STDUX_CONTROL_NO_BUTTON,
    STDUX_CONTROL_PROGRESS_BAR,
    STDUX_CONTROL_MESSAGE_TEXT,
};

enum WM_STDUX
{
    WM_STDUX_DETECT_PACKAGES = WM_APP + 1,
    WM_STDUX_PLAN_PACKAGES,
    WM_STDUX_APPLY_PACKAGES,
    WM_STDUX_MODAL_ERROR,
};


class CStandardUserExperience : public IBurnUserExperience
{
public:

public: 
    //
    // IUnknown
    //
    virtual STDMETHODIMP QueryInterface( 
        REFIID riid,
        LPVOID *ppvObject
        )
    {
        HRESULT hr = S_OK;
        *ppvObject = NULL;

        if  (__uuidof(IBurnUserExperience) == riid)
        {
            AddRef();
            *ppvObject = reinterpret_cast<LPVOID>(static_cast<IBurnUserExperience*>(this));
        }
        else if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvObject = reinterpret_cast<LPVOID>(static_cast<LPUNKNOWN>(this));
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        return hr;
    }

    virtual STDMETHODIMP_(ULONG) AddRef()
    {
        return ::InterlockedIncrement(&m_cref);
    }

    virtual STDMETHODIMP_(ULONG) Release()
    {
        UINT cref = ::InterlockedDecrement(&m_cref);
        if (0 == cref)
        {
            delete this;
        }

        return cref;
    }

    //
    // IBurnUserExperience
    //
    // Initialize - ensure all the necessary objects are created/initialized.
    //
    virtual STDMETHODIMP Initialize(
        __in IBurnCore* pCore,
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType
        )
    {
        HRESULT hr = S_OK;
        DWORD dwUIThreadId = 0;

        m_pCore = pCore;

        m_hModalWait = ::CreateEventW(NULL, TRUE, FALSE, NULL);
        ExitOnNullWithLastError(m_hModalWait, hr, "Failed to create modal event.");

        // create UI thread
        m_hUiThread = ::CreateThread(NULL, 0, UiThreadProc, this, 0, &dwUIThreadId);
        if (!m_hUiThread)
        {
            ExitWithLastError(hr, "Failed to create UI thread.");
        }

    LExit:
        return hr;
    }


    virtual STDMETHODIMP_(void) Uninitialize()
    {
        // wait for UX thread to terminate
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ::CloseHandle(m_hUiThread);
        }

        if (m_hModalWait)
        {
            ::CloseHandle(m_hModalWait);
        }
    }


    virtual STDMETHODIMP_(int)  OnDetectBegin(
        __in DWORD cPackages
        )
    {
        return IDOK;
    }


    virtual int __stdcall OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        )
    {
        HRESULT hr = S_OK;
        if ( NULL == m_sczFamilyBundleId)
        {
            hr = StrAllocString(&m_sczFamilyBundleId, wzBundleId, 0);
            ExitOnFailure(hr, "Failed to alloc string." );
        }
    LExit:
        return IDOK;
    }


    virtual STDMETHODIMP_(int)  OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnDetectPackageBegin()");
        return IDOK; // be a good Boy Scout, always prepare.
    }


    virtual STDMETHODIMP_(void)  OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnDetectPackageComplete()");
    }


    virtual STDMETHODIMP_(void)  OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnDetectComplete()");
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_DETECTED);

            // Quiet or passive display just goes straight into planning.
            if (BURN_DISPLAY_NONE == m_command.display || BURN_DISPLAY_PASSIVE == m_command.display)
            {
                ::PostMessageW(m_hWnd, WM_STDUX_PLAN_PACKAGES, 0, m_command.action);
            }
        }
    }


    virtual STDMETHODIMP_(int)  OnPlanBegin(
        __in DWORD cPackages
        )
    {
        HRESULT hr = S_OK;

        hr = m_pCore->SetVariableNumeric(L"PI", 31415927);
        ExitOnFailure(hr, "SetVariableNumeric failed.");

        hr = m_pCore->SetVariableString(L"PieFlavor", L"Apple");
        ExitOnFailure(hr, "SetVariableString failed.");

        hr = m_pCore->SetVariableVersion(L"PieVersion", 0x1000100010001000);
        ExitOnFailure(hr, "SetVariableVersion failed.");

    LExit:
        return IDOK;
    }


    virtual int __stdcall OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        *pRequestedState = REQUEST_STATE_PRESENT;

        return IDOK;
    }


    virtual STDMETHODIMP_(int)  OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId, 
        __inout REQUEST_STATE *pRequestState
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnPlanPackageBegin()");
        return IDOK; // be a good Boy Scout, always prepare.
    }


    virtual STDMETHODIMP_(void)  OnPlanPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state,
        __in REQUEST_STATE requested,
        __in ACTION_STATE execute,
        __in ACTION_STATE rollback
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnPlanPackageComplete()");
    }


    virtual STDMETHODIMP_(void)  OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnPlanComplete()");
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_PLANNED);
            ::PostMessageW(m_hWnd, WM_STDUX_APPLY_PACKAGES, 0, 0);
        }
    }


    virtual STDMETHODIMP_(int)  OnApplyBegin()
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int)  OnRegisterBegin()
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(void)  OnRegisterComplete(
        __in HRESULT hrStatus
        )
    {
        return;
    }


    virtual STDMETHODIMP_(void)  OnUnregisterBegin()
    {
        return;
    }


    virtual STDMETHODIMP_(void)  OnUnregisterComplete(
        __in HRESULT hrStatus
        )
    {
        return;
    }


    virtual STDMETHODIMP_(void)  OnApplyComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(STDUX_STATE_APPLIED);

        // Failure or quiet or passive display just close at the end, no questions asked.
        if (BURN_DISPLAY_NONE == m_command.display || BURN_DISPLAY_PASSIVE == m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }

        // Save this to enable us to detect failed apply phase
        m_hrFinal = hrStatus;

        return;
    }


    virtual STDMETHODIMP_(int) OnCacheBegin()
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(void)  OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_CACHED);
        }
    }


    virtual STDMETHODIMP_(BOOL) OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        return TRUE;
    }


    virtual STDMETHODIMP_(BOOL) OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnExecutePackageBegin()");
        return TRUE; // always execute.
    }


    virtual STDMETHODIMP_(int)  OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        HRESULT hr = S_OK;

        if (BURN_DISPLAY_FULL == m_command.display)
        {
            ModalPrompt(STDUX_STATE_ERROR, dwUIHint, wzError);
        }
        else // just cancel when quiet or passive.
        {
            m_nModalResult = IDABORT;
        }

        return m_nModalResult;
    }


    virtual STDMETHODIMP_(int)  OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        HRESULT hr = S_OK;

        m_nModalResult = IDOK;

        hr = ThemeSetProgressControl(m_pTheme, STDUX_CONTROL_PROGRESS_BAR, dwOverallProgressPercentage);
        ExitOnFailure(hr, "Failed to set progress.");

    LExit:
        if (FAILED(hr) && IDNOACTION == m_nModalResult)
        {
            m_nModalResult = IDERROR;
        }

        return m_nModalResult;
    }

    virtual int __stdcall OnDownloadPayloadBegin(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName
        )
    {
        return IDOK;
    }

    virtual void __stdcall OnDownloadPayloadComplete(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName,
        __in HRESULT hrStatus
        )
    {
        return;
    }

    virtual int __stdcall  OnDownloadProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        return IDOK;
    }

    virtual int __stdcall  OnExecuteProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(void)  OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrExitCode
        )
    {
        Trace(REPORT_STANDARD, "CStandardUserExperience::OnExecutePackageComplete()");
    }



    virtual STDMETHODIMP_(void)  OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
        // Failure or quiet or passive display just close at the end, no questions asked.
        if (BURN_DISPLAY_NONE == m_command.display || BURN_DISPLAY_PASSIVE == m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }
        else
        {
            if (SUCCEEDED(hrStatus))
            {
                OnProgress(100, 100); // ensure we're 100% complete.
            }

            SetState(STDUX_STATE_EXECUTED);
        }
    }


    virtual STDMETHODIMP_(BOOL) OnRestartRequired()
    {
        // If the user already said "OK", don't ask again.
        if (!m_fAllowRestart)
        {
            if (BURN_DISPLAY_FULL == m_command.display && BURN_RESTART_PROMPT == m_command.restart)
            {
                // TODO: Get restart message from somewhere

                ModalPrompt(STDUX_STATE_RESTART, MB_OKCANCEL, L"Restart required.  Allow restart?");
                m_fAllowRestart = (IDOK == m_nModalResult); // only allow restart if the user clicked OK.
            }
            else
            {
                m_fAllowRestart = (BURN_RESTART_ALWAYS == m_command.restart || BURN_RESTART_AUTOMATIC == m_command.restart || BURN_RESTART_PROMPT == m_command.restart);
            }
        }

        return m_fAllowRestart;
    }

    virtual STDMETHODIMP_(int) ResolveSource (
        __in    LPCWSTR itemID ,
        __in    LPCWSTR itemPath
        )
    {
        OPENFILENAMEW ofn;       // common dialog box structure
        WCHAR szFile[MAX_PATH];       // buffer for file name
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = L"All\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
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

    virtual STDMETHODIMP_(BOOL) CanPackagesBeDownloaded(void)
    {
        return TRUE;
    }

    virtual int __stdcall OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD64 dw64PackageCacheSize
        )
    {
        return IDOK;
    }

    virtual void __stdcall OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )
    {
    }

private: // privates
    //
    // UiThreadProc - entrypoint for UI thread.
    //
    static DWORD WINAPI UiThreadProc(
        __in LPVOID pvContext
        )
    {
        HRESULT hr = S_OK;
        CStandardUserExperience* pThis = (CStandardUserExperience*)pvContext;
        BOOL fComInitialized = FALSE;
        BOOL fRet = FALSE;
        MSG msg = { };

        // initialize COM
        hr = ::CoInitialize(NULL);
        ExitOnFailure(hr, "Failed to initialize COM.");
        fComInitialized = TRUE;

        // create main window
        hr = pThis->CreateMainWindow();
        ExitOnFailure(hr, "Failed to create main window.");

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
        pThis->DestroyMainWindow();

        // initiate engine shutdown
        pThis->m_pCore->Shutdown(pThis->m_hrFinal);

        // uninitialize COM
        if (fComInitialized)
        {
            ::CoUninitialize();
        }

        return hr;
    }


    //
    // CreateMainWindow - creates the main install window.
    //
    HRESULT CreateMainWindow()
    {
        HRESULT hr = S_OK;
        WNDCLASSW wc = { };
        DWORD dwWindowStyle = 0;
        LPWSTR sczThemePath = NULL;
        LPWSTR sczCommandLine = NULL;
        DWORD cchCommandLine = 8192; // Use limit imposed by CMD.EXE
        
        // load theme relative to stdux.dll.
        hr = PathRelativeToModule(&sczThemePath, L"thm.xml", m_hModule);
        ExitOnFailure(hr, "Failed to combine module path with thm.xml.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        ExitOnFailure(hr, "Failed to load theme from thm.xml.");

        // Get any command line arguments that engine does not process
        for (DWORD cRetry = 0; cRetry < 2; ++cRetry)
        {
            hr = StrAlloc(&sczCommandLine, cchCommandLine );
            ExitOnFailure(hr, "Failed to allocate space for command line string.");

            hr = m_pCore->GetCommandLineParameters(sczCommandLine,&cchCommandLine);
            if (SUCCEEDED(hr)  ||  (FAILED(hr) && hr != E_MOREDATA))
            {
                break;
            }
            else if (hr == E_MOREDATA)
            {
                // GetCommandLineParameters has returned character count so add space for null terminator and try again
                ++cchCommandLine;
            }
        }

        ExitOnFailure(hr, "Failed to get command line string.");

        if (S_OK == hr)
        {
            hr = ProcessCommandLine(sczCommandLine);
            ExitOnFailure(hr, "Unknown commandline parameters.");
        }

        if (NULL != m_sczLanguage)
        {
            hr = ThemeLoadLocFromFile(m_pTheme, m_sczLanguage, m_hModule);
            ExitOnFailure(hr, "Failed to localize from /lang.");
        }
        else
        {
            hr = ThemeLoadLocFromFile(m_pTheme, L"en-us.wxl", m_hModule);
            ExitOnFailure(hr, "Failed to localize from default language.");
        }

        // Register the window class and create the window.
        wc.style = 0;
        wc.lpfnWndProc = CStandardUserExperience::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = m_hModule;
        wc.hIcon = reinterpret_cast<HICON>(m_pTheme->hIcon);
        wc.hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        wc.hbrBackground = m_pTheme->rgFonts[m_pTheme->dwFontId].hBackground;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = STDUX_WINDOW_CLASS;
        if (!::RegisterClassW(&wc))
        {
            ExitWithLastError(hr, "Failed to register window.");
        }

        m_fRegistered = TRUE;

        // Calculate the window style based on the theme style and command display value.
        dwWindowStyle = m_pTheme->dwStyle;
        if (BURN_DISPLAY_NONE == m_command.display)
        {
            dwWindowStyle &= ~WS_VISIBLE;
        }

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->wzCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
        ExitOnNullWithLastError(m_hWnd, hr, "Failed to create window.");

    LExit:
        ReleaseStr(sczThemePath);
        ReleaseStr(sczCommandLine);
        ReleaseStr(m_sczLanguage);

        if (FAILED(hr))
        {
            DestroyMainWindow();
        }

        return hr;
    }


    //
    // DestroyMainWindow - clean up all the window registration.
    //
    void DestroyMainWindow()
    {
        if (m_hWnd)
        {
            ::CloseWindow(m_hWnd);
            m_hWnd = NULL;
        }

        if (m_fRegistered)
        {
            ::UnregisterClassW(STDUX_WINDOW_CLASS, m_hModule);
            m_fRegistered = FALSE;
        }

        if (m_pTheme)
        {
            ThemeFree(m_pTheme);
            m_pTheme = NULL;
        }
    }


    //
    // ProcessCommandLine - process the provided command line arguments.
    //
    HRESULT ProcessCommandLine(
        __in_z LPCWSTR wzCommandLine
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczName = NULL;

        int argc = 0;
        LPWSTR* argv = ::CommandLineToArgvW(wzCommandLine, &argc);
        ExitOnNullWithLastError(argv, hr, "Failed to get command line.");

        for (int i = 0; i < argc; ++i)
        {
            if (argv[i][0] == L'-' || argv[i][0] == L'/')
            {
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"lang", -1))
                {
                    if (i + 1 >= argc)
                    {
                        ExitOnRootFailure(hr = E_INVALIDARG, "Must specify a language.");
                    }

                    ++i;

                    hr = StrAllocString(&m_sczLanguage, &argv[i][0], 0);
                    ExitOnFailure(hr, "Failed to copy language.");

                    hr = StrAllocConcat(&m_sczLanguage, L".wxl", 0);
                    ExitOnFailure(hr, "Failed to concatenate wxl extension.");
                }
            }
        }

    LExit:
        ReleaseStr(sczName);

        ::LocalFree(argv);

        return hr;
    }


    //
    // ModalPrompt - prompt the user to set the "m_nModalResult" value.
    //
    void ModalPrompt(
        __in STDUX_STATE modalState,
        __in DWORD dwUIHint,
        __in_z_opt LPCWSTR wzMessage
        )
    {
        HRESULT hr = S_OK;
        STDUX_STATE state = m_state; // remember the state so we can reset after going into the error state.

        m_nModalResult = IDNOACTION;

        if (wzMessage)
        {
            hr = ThemeSetTextControl(m_pTheme, STDUX_CONTROL_MESSAGE_TEXT, wzMessage);
            ExitOnFailure(hr, "Failed to set message text.");
        }

        SetState(modalState);
        ShowModalControls(dwUIHint);

        ::ResetEvent(m_hModalWait);

        ::WaitForSingleObject(m_hModalWait, INFINITE);

        ShowModalControls(0xF);

    LExit:
        m_state = state; // put the state back.
    }


    //
    // WndProc - standard windows message handler.
    //
    static LRESULT CALLBACK WndProc(
        __in HWND hWnd,
        __in UINT uMsg,
        __in WPARAM wParam,
        __in LPARAM lParam
        )
    {
        LRESULT lres = 0;
#pragma warning(suppress:4312)
        CStandardUserExperience* pUX = reinterpret_cast<CStandardUserExperience*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_NCCREATE:
            {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pUX = reinterpret_cast<CStandardUserExperience*>(lpcs->lpCreateParams);
#pragma warning(suppress:4244)
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pUX));
            }
            break;

        case WM_NCHITTEST:
            if (pUX->m_pTheme->dwStyle & WS_POPUP)
            {
                return HTCAPTION; // allow pop-up windows to be moved by grabbing any non-control.
            }
            break;

        case WM_NCDESTROY:
            lres = ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            return lres;

        case WM_CREATE:
            if (!pUX->OnCreate(hWnd))
            {
                return -1;
            }
            break;

        case WM_CLOSE:
            if (STDUX_STATE_EXECUTED > pUX->m_state)
            {
                // Check with the user to verify they want to cancel.
                //pUX->PromptCancel(hWnd);
            }
            pUX->m_fCanceled = TRUE; // TODO: let the prompt set this instead.

            if (pUX->m_fCanceled)
            {
                ::DestroyWindow(hWnd);
            }
            return 0;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;

        case WM_DRAWITEM:
            ThemeDrawControl(pUX->m_pTheme, reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
            return TRUE;

        case WM_CTLCOLORSTATIC:
            {
            HBRUSH hBrush = NULL;
            if (ThemeSetControlColor(pUX->m_pTheme, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(hWnd), &hBrush))
            {
                return reinterpret_cast<LRESULT>(hBrush);
            }
            }
            break;

        case WM_SETCURSOR:
            ThemeHoverControl(pUX->m_pTheme, hWnd, reinterpret_cast<HWND>(wParam));
            break;

        case WM_PAINT:
            // If there is anything to update, do so.
            if (::GetUpdateRect(hWnd, NULL, FALSE))
            {
                PAINTSTRUCT ps;
                ::BeginPaint(hWnd, &ps);
                ThemeDrawBackground(pUX->m_pTheme, &ps);
                ::EndPaint(hWnd, &ps);
            }
            return 0;

        case WM_STDUX_DETECT_PACKAGES:
            pUX->OnDetect();
            return 0;

        case WM_STDUX_PLAN_PACKAGES:
            pUX->OnPlan(static_cast<BURN_ACTION>(lParam));
            return 0;

        case WM_STDUX_APPLY_PACKAGES:
            pUX->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case STDUX_CONTROL_TOU_LINK:
                pUX->OnClickTermsOfUse();
                break;

            case STDUX_CONTROL_INSTALL_BUTTON:
                pUX->OnClickInstallButton();
                break;

            case STDUX_CONTROL_REPAIR_BUTTON:
                pUX->OnClickRepairButton();
                break;

            case STDUX_CONTROL_UNINSTALL_BUTTON:
                pUX->OnClickUninstallButton();
                break;

            case STDUX_CONTROL_CLOSE_BUTTON:
                pUX->OnClickCloseButton();
                break;

            case STDUX_CONTROL_OK_BUTTON: __fallthrough;
            case STDUX_CONTROL_CANCEL_BUTTON: __fallthrough;
            case STDUX_CONTROL_RETRY_BUTTON: __fallthrough;
            case STDUX_CONTROL_ABORT_BUTTON: __fallthrough;
            case STDUX_CONTROL_IGNORE_BUTTON: __fallthrough;
            case STDUX_CONTROL_YES_BUTTON: __fallthrough;
            case STDUX_CONTROL_NO_BUTTON: __fallthrough;
                pUX->OnClickModalButton(LOWORD(wParam));
                break;

            default:
                break;
            }
            return 0;
        }

        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }


    //
    // OnCreate - finishes loading the theme and tells the core we're ready for data.
    //
    BOOL OnCreate(
        __in HWND hWnd
        )
    {
        HRESULT hr = S_OK;

        hr = ThemeLoadControls(m_pTheme, hWnd);
        ExitOnFailure(hr, "Failed to load theme controls.");

        // Okay, we're ready for packages now.
        SetState(STDUX_STATE_DETECTING);

        ::PostMessageW(hWnd, WM_STDUX_DETECT_PACKAGES, 0, 0);

    LExit:
        return SUCCEEDED(hr);
    }


    //
    // OnDetect - start the processing of packages.
    //
    void OnDetect()
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CStandardUserExperience::OnDetect()");

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pCore->Detect();
        ExitOnFailure(hr, "Failed to start detecting chain.");

    LExit:
        return;
    }


    //
    // OnPlan - plan the detected changes.
    //
    void OnPlan(
        __in BURN_ACTION action
        )
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CStandardUserExperience::OnPlan()");

        hr = m_pCore->Plan(action);
        ExitOnFailure(hr, "Failed to start planning packages.");

        SetState(STDUX_STATE_PLANNING);

    LExit:
        return;
    }


    //
    // OnApply - apply the packages.
    //
    void OnApply()
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CStandardUserExperience::OnApply()");

        hr = m_pCore->Apply(m_hWnd);
        ExitOnFailure(hr, "Failed to start applying packages.");

        SetState(STDUX_STATE_APPLYING);

    LExit:
        return;
    }
    
    //
    // OnSuspend - start the processing of packages.
    //
    void OnSuspend()
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CStandardUserExperience::OnSuspend()");

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pCore->Suspend();
        ExitOnFailure(hr, "Failed to suspend chain.");

    LExit:
        return;
    }


    //
    // OnClickInstallButton - start the install by planning the packages.
    //
    void OnClickInstallButton()
    {
        this->OnPlan(BURN_ACTION_INSTALL);
    }


    //
    // OnClickRepairButton - start the repair.
    //
    void OnClickRepairButton()
    {
        this->OnPlan(BURN_ACTION_REPAIR);
    }


    //
    // OnClickUninstallButton - start the uninstall.
    //
    void OnClickUninstallButton()
    {
        this->OnPlan(BURN_ACTION_UNINSTALL);
    }


    //
    // OnClickCloseButton - close the application.
    //
    void OnClickCloseButton()
    {
        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }


    //
    // OnClickModalButton - handle the modal button clicks.
    //
    void OnClickModalButton(
        int nButton
        )
    {
        Trace1(REPORT_STANDARD, "CStandardUserExperience::OnClickModalButton(%d) - enter", nButton);

        switch (nButton)
        {
        case STDUX_CONTROL_OK_BUTTON:
            m_nModalResult = IDOK;
            break;

        case STDUX_CONTROL_CANCEL_BUTTON:
            m_nModalResult = IDCANCEL;
            break;

        case STDUX_CONTROL_RETRY_BUTTON:
            m_nModalResult = IDRETRY;
            break;

        case STDUX_CONTROL_ABORT_BUTTON:
            m_nModalResult = IDABORT;
            break;

        case STDUX_CONTROL_IGNORE_BUTTON:
            m_nModalResult = IDIGNORE;
            break;

        case STDUX_CONTROL_YES_BUTTON:
            m_nModalResult = IDYES;
            break;

        case STDUX_CONTROL_NO_BUTTON:
            m_nModalResult = IDNO;
            break;
        }

        ::SetEvent(m_hModalWait);

        return;
    }


    //
    // OnClickTermsOfUse - show the terms of use.
    //
    void OnClickTermsOfUse()
    {
        HRESULT hr = S_OK;
        LPWSTR sczResUri = NULL;

        hr = PathRelativeToModule(&sczResUri, L"tou.htm", m_hModule);
        ExitOnFailure(hr, "Failed to create URI to TOU.");

        hr = ShellExec(sczResUri);
        ExitOnFailure(hr, "Failed to launch URI to TOU.");

    LExit:
        ReleaseStr(sczResUri);
        return;
    }


    //
    // SetState
    //
    void SetState(
        __in STDUX_STATE state
        )
    {
        if (m_state != state)
        {
            Trace2(REPORT_STANDARD, "CStandardUserExperience::SetState() changing from state %d to %d", m_state, state);
            m_state = state;

            int nShowTermsOfUse = (m_state < STDUX_STATE_EXECUTING) ? SW_SHOW : SW_HIDE;
            int nShowInstall = (BURN_DISPLAY_FULL == m_command.display && BURN_ACTION_INSTALL == m_command.action && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            int nShowRepair = (FALSE && BURN_DISPLAY_FULL == m_command.display && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            int nShowUninstall = (BURN_DISPLAY_FULL == m_command.display && BURN_ACTION_UNINSTALL == m_command.action && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            int nShowClose = (BURN_DISPLAY_FULL == m_command.display && STDUX_STATE_APPLIED == m_state) ? SW_SHOW : SW_HIDE;

            int nShowMessage = ((STDUX_STATE_DETECTED < m_state && m_state <= STDUX_STATE_APPLIED) || STDUX_STATE_ERROR == m_state) ? SW_SHOW : SW_HIDE;
            int nShowProgressBar = (STDUX_STATE_DETECTED < m_state && m_state <= STDUX_STATE_APPLIED) ? SW_SHOW : SW_HIDE;

            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_TOU_LINK].hWnd, nShowTermsOfUse);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_INSTALL_BUTTON].hWnd, nShowInstall);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_REPAIR_BUTTON].hWnd, nShowRepair);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_UNINSTALL_BUTTON].hWnd, nShowUninstall);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_CLOSE_BUTTON].hWnd, nShowClose);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_MESSAGE_TEXT].hWnd, nShowMessage);
            ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_PROGRESS_BAR].hWnd, nShowProgressBar);
        }
    }


    //
    // ShowModalControls
    //
    void ShowModalControls(
        __in DWORD dwControls
        )
    {
        int nControl = dwControls & 0xF;
        int nShowOK = (MB_OK == nControl || MB_OKCANCEL == nControl) ? SW_SHOW : SW_HIDE;
        int nShowCancel = (STDUX_STATE_EXECUTING == m_state || MB_OKCANCEL == nControl || MB_RETRYCANCEL == nControl || MB_YESNOCANCEL == nControl) ? SW_SHOW : SW_HIDE;
        int nShowRetry = (MB_RETRYCANCEL == nControl || MB_ABORTRETRYIGNORE == nControl) ? SW_SHOW : SW_HIDE;
        int nShowAbort = (MB_ABORTRETRYIGNORE == nControl) ? SW_SHOW : SW_HIDE;
        int nShowIgnore = (MB_ABORTRETRYIGNORE == nControl) ? SW_SHOW : SW_HIDE;
        int nShowYes = (MB_YESNO == nControl || MB_YESNOCANCEL == nControl) ? SW_SHOW : SW_HIDE;
        int nShowNo = (MB_YESNO == nControl || MB_YESNOCANCEL == nControl) ? SW_SHOW : SW_HIDE;

        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_OK_BUTTON].hWnd, nShowOK);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_CANCEL_BUTTON].hWnd, nShowCancel);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_RETRY_BUTTON].hWnd, nShowRetry);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_ABORT_BUTTON].hWnd, nShowAbort);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_IGNORE_BUTTON].hWnd, nShowIgnore);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_YES_BUTTON].hWnd, nShowYes);
        ::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_NO_BUTTON].hWnd, nShowNo);
    }


    HRESULT ShellExec(
        __in LPCWSTR wzTarget
        )
    {
        HRESULT hr = S_OK;

        HINSTANCE hinst = ::ShellExecuteW(m_hWnd, L"open", wzTarget, NULL, NULL, SW_SHOWDEFAULT);
        if (hinst <= HINSTANCE(32))
        {
            switch (int(hinst))
            {
            case ERROR_FILE_NOT_FOUND:
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                break;
            case ERROR_PATH_NOT_FOUND:
                hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
                break;
            case ERROR_BAD_FORMAT:
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                break;
            case SE_ERR_ASSOCINCOMPLETE:
            case SE_ERR_NOASSOC:
                hr = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
                break;
            case SE_ERR_DDEBUSY:
            case SE_ERR_DDEFAIL:
            case SE_ERR_DDETIMEOUT:
                hr = HRESULT_FROM_WIN32(ERROR_DDE_FAIL);
                break;
            case SE_ERR_DLLNOTFOUND:
                hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
                break;
            case SE_ERR_OOM:
                hr = E_OUTOFMEMORY;
                break;
            case SE_ERR_ACCESSDENIED:
                hr = E_ACCESSDENIED;
                break;
            default:
                hr = E_FAIL;
            }

            ExitOnFailure1(hr, "ShellExec failed with return code %d", int(hinst));
        }

    LExit:
        return hr;
    }


public:
    //
    // Constructor - intitialize member variables.
    //
    CStandardUserExperience(
        __in HMODULE hModule,
        __in const BURN_COMMAND* pCommand
        )
    {
        m_hUiThread = NULL;

        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BURN_COMMAND));

        m_cReferences = 1;
        m_pTheme = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;
        m_hwndHover = NULL;
        m_pCore = NULL;

        m_fCanceled = FALSE;
        m_state = STDUX_STATE_INITIALIZING;
        m_nModalResult = IDNOACTION;
        m_hModalWait = NULL;

        m_fAllowRestart = FALSE;
        m_sczLanguage = NULL;
        m_hrFinal = S_OK;

        m_sczFamilyBundleId = NULL;

        m_cref = 1;
    }


    //
    // Destructor - release member variables.
    //
    ~CStandardUserExperience()
    {
        DestroyMainWindow();

        if (m_pTheme)
        {
            ThemeFree(m_pTheme);
            m_pTheme = NULL;
        }

        ThemeUninitialize();

        if (m_hModalWait)
        {
            ::CloseHandle(m_hModalWait);
        }
        ReleaseStr(m_sczFamilyBundleId);
    }

private:
    long m_cReferences;

    HANDLE m_hUiThread;

    HMODULE m_hModule;
    BURN_COMMAND m_command;

    IBurnCore* m_pCore;
    THEME* m_pTheme;
    BOOL m_fRegistered;
    HWND m_hWnd;
    HWND m_hwndHover;

    BOOL m_fCanceled;
    STDUX_STATE m_state;
    INT m_nModalResult;
    HANDLE m_hModalWait;

    BOOL m_fAllowRestart;
    LPWSTR m_sczLanguage;
    HRESULT m_hrFinal;
    LPWSTR m_sczFamilyBundleId;

    LONG m_cref;
};


//
// CreateUserExperience - creates a new IBurnUserExperience object.
//
HRESULT CreateUserExperience(
    __in HMODULE hModule,
    __in const BURN_COMMAND* pCommand,
    __out IBurnUserExperience **ppUX
    )
{
    HRESULT hr = S_OK;
    CStandardUserExperience* pUX = NULL;

    pUX = new CStandardUserExperience(hModule, pCommand);
    ExitOnNull(pUX, hr, E_OUTOFMEMORY, "Failed to create new standard UX object.");

    *ppUX = pUX;
    pUX = NULL;

LExit:
    return hr;
}
