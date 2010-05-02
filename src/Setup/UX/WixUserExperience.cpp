//-------------------------------------------------------------------------------------------------
// <copyright file="WixUserExperience.cpp" company="Microsoft">
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

static const LPCWSTR WIXUX_WINDOW_CLASS = L"WixBurnUX";

enum WIXUX_STATE
{
    WIXUX_STATE_INITIALIZING,
    WIXUX_STATE_PREPARING,

    WIXUX_STATE_ACCEPT_LICENSE,
    WIXUX_STATE_SELECT_LOCATION,
    WIXUX_STATE_SELECT_FEATURES,
    WIXUX_STATE_MAINTENANCE,

    WIXUX_STATE_CACHING,
    WIXUX_STATE_CACHED,
    WIXUX_STATE_EXECUTING,
    WIXUX_STATE_EXECUTED,
    WIXUX_STATE_RESTART,
    WIXUX_STATE_ERROR,
};


enum WIXUX_CONTROL
{
    WIXUX_CONTROL_HEADER_IMAGE,
    WIXUX_CONTROL_TITLE_WELCOME_TEXT,
    WIXUX_CONTROL_SUBTITLE_WELCOME_TEXT,
    WIXUX_CONTROL_LICENSE_RICHEDIT,
    WIXUX_CONTROL_ACCEPT_CHECKBOX,
    WIXUX_CONTROL_TOP_MESSAGE_TEXT,
    WIXUX_CONTROL_BOTTOM_MESSAGE_TEXT,

    WIXUX_CONTROL_PRINT_BUTTON,
    WIXUX_CONTROL_ADVANCED_BUTTON,
    WIXUX_CONTROL_INSTALL_BUTTON,
    WIXUX_CONTROL_BACK_BUTTON,
    WIXUX_CONTROL_NEXT_BUTTON,
    WIXUX_CONTROL_CLOSE_BUTTON,

    WIXUX_CONTROL_REPAIR_BUTTON,
    WIXUX_CONTROL_UNINSTALL_BUTTON,

    WIXUX_CONTROL_PROGRESS_BAR,
    WIXUX_CONTROL_MESSAGE_TEXT,

    WIXUX_CONTROL_CANCEL_BUTTON,

    WIXUX_CONTROL_TITLE_INSTALL_LOCATION_TEXT,
    WIXUX_CONTROL_SUBTITLE_INSTALL_LOCATION_TEXT,
    WIXUX_CONTROL_TITLE_SELECT_FEATURES_TEXT,
    WIXUX_CONTROL_SUBTITLE_SELECT_FEATURES_TEXT,
    WIXUX_CONTROL_TITLE_MAINTENANCE_MODE_TEXT,
    WIXUX_CONTROL_SUBTITLE_MAINTENANCE_MODE_TEXT,
    WIXUX_CONTROL_TITLE_CONFIGURING_TEXT,
    WIXUX_CONTROL_SUBTITLE_CONFIGURING_TEXT,
    WIXUX_CONTROL_TITLE_COMPLETE_TEXT,
    WIXUX_CONTROL_SUBTITLE_COMPLETE_TEXT,
};

enum WM_WIXUX
{
    WM_WIXUX_FIRST = WM_USER, // this enum value must always be first.

    WM_WIXUX_DETECT_PACKAGES,
    WM_WIXUX_PLAN_PACKAGES,
    WM_WIXUX_APPLY_PACKAGES,
    WM_WIXUX_MODAL_ERROR,

    WM_WIXUX_LAST, // this enum value must always be last.
};


class CWixUserExperience : public IBurnUserExperience
{
public: // IBurnUserExperience
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
    }


    virtual STDMETHODIMP_(int)  OnDetectBegin(
        __in DWORD cPackages
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnDetectBegin()");
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        )
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int)  OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnDetectPackageBegin()");
        return IDOK;
    }


    virtual STDMETHODIMP_(void)  OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnDetectPackageComplete()");
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, L"Wix", -1))
        {
            m_fWixMsiInstalled = (PACKAGE_STATE_PRESENT == state);
        }
    }


    virtual STDMETHODIMP_(void)  OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        HRESULT hr = S_OK;
        BOOL fFailed = FALSE;
        LPWSTR sczMessage = NULL;

        Trace(REPORT_STANDARD, "CWixUserExperience::OnDetectComplete()");
        if (SUCCEEDED(hrStatus))
        {
            // Evaluate the authored launch conditions.
            for (DWORD i = 0; i < m_Conditions.cConditions; ++i)
            {
                BOOL fResult = TRUE;

                hr = BuxConditionEvaluate(m_Conditions.rgConditions + i, m_pCore, &fResult, &sczMessage);
                ExitOnFailure1(hr, "Failed to evaluate condition #%u", i);

                if (!fResult)
                {
                    fFailed = TRUE;

                    if (BURN_DISPLAY_NONE < m_command.display)
                    {
                        ::MessageBoxW(m_hWnd, sczMessage, m_pTheme->wzCaption, MB_ICONERROR | MB_OK);
                    }
                }
            }

            // If any launch conditions failed, exit.
            if (fFailed)
            {
                ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
            }
            else
            {
                // Quiet or passive display just goes straight into planning.
                if (BURN_DISPLAY_NONE == m_command.display || BURN_DISPLAY_PASSIVE == m_command.display)
                {
                    m_fAcceptedLicense = TRUE;
                    ::PostMessageW(m_hWnd, WM_WIXUX_PLAN_PACKAGES, 0, m_command.action);
                }
                else
                {
                    WIXUX_STATE state = !m_fWixMsiInstalled ? WIXUX_STATE_ACCEPT_LICENSE : WIXUX_STATE_MAINTENANCE;
                    SetState(state);
                }
            }
        }

    LExit:
        ReleaseStr(sczMessage);
    }


    virtual STDMETHODIMP_(int)  OnPlanBegin(
        __in DWORD cPackages
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnPlanBegin()");
        SetState(WIXUX_STATE_EXECUTING);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int)  OnPlanPackageBegin(
        __in LPCWSTR wzPackageId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnPlanPackageBegin()");
        // For now, install the package.
        if (pRequestedState)
        {
            *pRequestedState = REQUEST_STATE_PRESENT;
        }
        
        return IDOK;
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
        Trace(REPORT_STANDARD, "CWixUserExperience::OnPlanPackageComplete()");
        return;
    }


    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnPlanComplete()");
        if (SUCCEEDED(hrStatus))
        {
            ::PostMessageW(m_hWnd, WM_WIXUX_APPLY_PACKAGES, 0, 0);
        }
        else
        {
            // TODO: handle failure case.
        }

        return;
    }


    virtual STDMETHODIMP_(int)  OnApplyBegin()
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnRegisterBegin()
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(void) OnRegisterComplete(
        __in HRESULT hrStatus
        )
    {
    }


    virtual STDMETHODIMP_(void) OnUnregisterBegin()
    {
        return;
    }


    virtual STDMETHODIMP_(void) OnUnregisterComplete(
        __in HRESULT hrStatus
        )
    {
    }


    virtual STDMETHODIMP_(int) OnCacheBegin()
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD64 dw64PackageCacheSize
        )
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(void) OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )
    {
    }


    virtual STDMETHODIMP_(void) OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
    }


    virtual STDMETHODIMP_(int) OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnDownloadPayloadBegin(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(void) OnDownloadPayloadComplete(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName,
        __in HRESULT hrStatus
        )
    {
        return;
    }

    virtual STDMETHODIMP_(int) OnDownloadProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnExecuteProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnExecutePackageBegin()");
        return IDOK; // always execute.
    }


    virtual STDMETHODIMP_(int) OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        HRESULT hr = S_OK;
        int nModalResult = IDCANCEL; // assume we always cancel on error.

        if (BURN_DISPLAY_FULL == m_command.display)
        {
            nModalResult = ModalPrompt(WIXUX_STATE_ERROR, dwUIHint, wzError);
        }

        return nModalResult;
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

    virtual STDMETHODIMP_(int) OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        HRESULT hr = S_OK;
        int nModalResult = IDOK;

        hr = ThemeSetProgressControl(m_pTheme, WIXUX_CONTROL_PROGRESS_BAR, dwOverallProgressPercentage);
        ExitOnFailure(hr, "Failed to set progress.");

    LExit:
        if (FAILED(hr))
        {
            nModalResult = IDERROR;
        }

        return nModalResult;
    }


    virtual STDMETHODIMP_(void) OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrExitCode
        )
    {
        Trace(REPORT_STANDARD, "CWixUserExperience::OnExecutePackageComplete()");
        if (FAILED(hrExitCode))
        {
            m_fFailed = TRUE;
        }
    }


    virtual STDMETHODIMP_(void) OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
    }


    virtual STDMETHODIMP_(void) OnApplyComplete(
        __in HRESULT hrStatus
        )
    {
        AssertSz((SUCCEEDED(hrStatus) && !m_fFailed) || (FAILED(hrStatus) && m_fFailed), "Either we failed or we didn't.");

        if (SUCCEEDED(hrStatus))
        {
            OnProgress(100, 100); // ensure we're 100% complete.
        }

        SetState(WIXUX_STATE_EXECUTED);

        // Failure or quiet or passive display just close at the end, no questions asked.
        if (m_fFailed || BURN_DISPLAY_NONE == m_command.display || BURN_DISPLAY_PASSIVE == m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
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

                int nModalResult = ModalPrompt(WIXUX_STATE_RESTART, MB_OKCANCEL, L"Restart required.  Allow restart?");
                m_fAllowRestart = (IDOK == nModalResult); // only allow restart if the user clicked OK.
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
        return IDCANCEL;
    }

    virtual STDMETHODIMP_(BOOL) CanPackagesBeDownloaded(void)
    {
        return TRUE;
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
        CWixUserExperience* pThis = (CWixUserExperience*)pvContext;
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
        pThis->m_pCore->Shutdown(0);

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
        IXMLDOMDocument* pixdManifest = NULL;

        // load and parse the UX manifest.
        hr = BuxManifestLoad(m_hModule, &pixdManifest);
        ExitOnFailure(hr, "Failed to load ux.manifest.");

        hr = BuxConditionsParseFromXml(&m_Conditions, pixdManifest);
        ExitOnFailure(hr, "Failed to parse conditions form ux.manifest.");

        // load theme relative to stdux.dll.
        hr = PathRelativeToModule(&sczThemePath, L"thm.xml", m_hModule);
        ExitOnFailure(hr, "Failed to combine module path with thm.xml.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        ExitOnFailure(hr, "Failed to load theme from thm.xml.");
        
        // Uncomment when GetComandLiineParameters() method is implemented in sprint 2
        /* hr = m_pCore->GetCommandLineParameters(&sczCommandLine);

        if (S_OK == hr)
        {
            hr = ProcessCommandLine(sczCommandLine);
            ExitOnFailure(hr, "Unknown commandline parameters.");
        } */

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
        wc.lpfnWndProc = CWixUserExperience::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = m_hModule;
        wc.hIcon = reinterpret_cast<HICON>(m_pTheme->hIcon);
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = m_pTheme->rgFonts[m_pTheme->dwFontId].hBackground;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = WIXUX_WINDOW_CLASS;
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
        ReleaseObject(pixdManifest);

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
            ::UnregisterClassW(WIXUX_WINDOW_CLASS, m_hModule);
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

        if (argv)
        {
            ::LocalFree(argv);
        }

        return hr;
    }


    //
    // ModalPrompt - prompt the user with a message box and return their choice.
    //
    int ModalPrompt(
        __in WIXUX_STATE modalState,
        __in DWORD dwUIHint,
        __in_z_opt LPCWSTR wzMessage
        )
    {
        switch (modalState)
        {
        case WIXUX_STATE_RESTART:
            dwUIHint |= MB_ICONWARNING;
            break;

        case WIXUX_STATE_ERROR:
            dwUIHint |= MB_ICONERROR;
            break;

        default:
            AssertSz(FALSE, "Unexpected modal state.");
        }

        return ::MessageBoxW(m_hWnd, wzMessage, m_pTheme->wzCaption, dwUIHint);
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
        CWixUserExperience* pUX = reinterpret_cast<CWixUserExperience*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_NCCREATE:
            {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pUX = reinterpret_cast<CWixUserExperience*>(lpcs->lpCreateParams);
#pragma warning(suppress:4244)
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pUX));
            }
            break;

        case WM_NCHITTEST:
            if (pUX->m_pTheme->dwStyle & WS_POPUP)
            {
                return HTCAPTION; // allow windows with no frame to be moved by grabbing any non-control.
            }
            break;

        case WM_NCDESTROY:
            lres = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
            return lres;

        case WM_CREATE:
            if (!pUX->OnCreate(hWnd))
            {
                return -1;
            }
            break;

        case WM_CLOSE:
            pUX->OnClose(hWnd);
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
            if (ThemeSetControlColor(pUX->m_pTheme, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam), &hBrush))
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

        case WM_WIXUX_DETECT_PACKAGES:
            pUX->OnDetect();
            return 0;

        case WM_WIXUX_PLAN_PACKAGES:
            pUX->OnPlan(static_cast<BURN_ACTION>(lParam));
            return 0;

        case WM_WIXUX_APPLY_PACKAGES:
            pUX->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case WIXUX_CONTROL_ACCEPT_CHECKBOX:
                pUX->OnClickAcceptCheckbox();
                break;

            case WIXUX_CONTROL_INSTALL_BUTTON:
                pUX->OnClickInstallButton();
                break;

            case WIXUX_CONTROL_REPAIR_BUTTON:
                pUX->OnClickRepairButton();
                break;

            case WIXUX_CONTROL_UNINSTALL_BUTTON:
                pUX->OnClickUninstallButton();
                break;

            case WIXUX_CONTROL_CANCEL_BUTTON: __fallthrough; // cancel is the same as click the close button or "X" in the top right corner.
            case WIXUX_CONTROL_CLOSE_BUTTON:
                pUX->OnClickCloseButton();
                break;

            default:
                break;
            }
            return 0;
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
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

        hr = ThemeLoadRichEditFromFile(m_pTheme, WIXUX_CONTROL_LICENSE_RICHEDIT, L"License.rtf", m_hModule);
        ExitOnFailure(hr, "Failed to load license.");

#ifndef BCM_SETSHIELD // TODO: don't define this here, use the correct headers to get the define.
#define BCM_SETSHIELD            (0x1600 + 0x000C)
#endif
        ::SendMessage(m_pTheme->rgControls[WIXUX_CONTROL_INSTALL_BUTTON].hWnd, BCM_SETSHIELD, 0, TRUE); // mark the install button as requiring elevation.

        // Okay, we're ready for packages now.
        SetState(WIXUX_STATE_PREPARING);

        ::PostMessageW(hWnd, WM_WIXUX_DETECT_PACKAGES, 0, 0);

    LExit:
        return SUCCEEDED(hr);
    }


    //
    // OnClose - called on close and cancel, prompts for cancel if appropriate.
    //
    void OnClose(
        __in HWND hWnd
        )
    {
        if (WIXUX_STATE_EXECUTED == m_state || m_fFailed || m_fCanceled)
        {
            ::DestroyWindow(hWnd);
        }
        else // for all non-complete states, prompt before cancelling.
        {
            // Check with the user to verify they want to cancel.
            //PromptCancel(hWnd);
            m_fCanceled = TRUE; // TODO: let the prompt set this instead.

            // If the user chose cancel and we're not caching or executing, close the window now. Otherwise,
            // we'll have to let the progress system kick off a rollback which will eventually cause the
            // window to close.
            if (m_fCanceled && (WIXUX_STATE_CACHING > m_state || WIXUX_STATE_EXECUTING < m_state))
            {
                ::DestroyWindow(hWnd);
            }
        }
    }


    //
    // OnDetect - start the processing of packages.
    //
    void OnDetect()
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CWixUserExperience::OnDetect()");

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pCore->Detect();
        ExitOnFailure(hr, "Failed to start detecting chain.");

    LExit:
        return;
    }


    //
    // OnPlan - start the planning of packages.
    //
    void OnPlan(
        __in BURN_ACTION action
        )
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CWixUserExperience::OnPlan()");

        // Tell the core we're ready for the packages to be planned now.
        hr = m_pCore->Plan(action);
        ExitOnFailure(hr, "Failed to start planning chain.");

    LExit:
        return;
    }


    //
    // OnApply - execute the cached packages.
    //
    void OnApply()
    {
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "CWixUserExperience::OnApply()");

        hr = m_pCore->Apply(m_hWnd);
        ExitOnFailure(hr, "Failed to execute install.");

    LExit:
        return;
    }


    //
    // OnClickInstallButton - start the install by caching the packages.
    //
    void OnClickInstallButton()
    {
        HRESULT hr = S_OK;

        //hr = TryElevate();
        //if (SUCCEEDED(hr))
        //{
        //    this->OnCache();
        //}
        //else
        //{
        //    m_fFailed = TRUE;
        //    ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        //}
        hr = m_pCore->Plan(BURN_ACTION_INSTALL);
        ExitOnFailure(hr, "Failed to execute repair.");

    LExit:
        return;
    }


    //
    // OnClickRepairButton - start the repair.
    //
    void OnClickRepairButton()
    {
        HRESULT hr = S_OK;

        hr = m_pCore->Plan(BURN_ACTION_REPAIR);
        ExitOnFailure(hr, "Failed to execute repair.");

    LExit:
        return;
    }


    //
    // OnClickUninstallButton - start the uninstall.
    //
    void OnClickUninstallButton()
    {
        HRESULT hr = S_OK;

        hr = m_pCore->Plan(BURN_ACTION_UNINSTALL);
        ExitOnFailure(hr, "Failed to execute repair.");

    LExit:
        return;
    }


    //
    // OnClickCloseButton - close the application.
    //
    void OnClickCloseButton()
    {
        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }


    //
    // OnClickAcceptCheckbox - allow the install to continue.
    //
    void OnClickAcceptCheckbox()
    {
        m_fAcceptedLicense = ThemeIsControlChecked(m_pTheme, WIXUX_CONTROL_ACCEPT_CHECKBOX);

        //::EnableWindow(m_pTheme->rgControls[WIXUX_CONTROL_ADVANCED_BUTTON].hWnd, m_fAcceptedLicense);
        ::EnableWindow(m_pTheme->rgControls[WIXUX_CONTROL_INSTALL_BUTTON].hWnd, m_fAcceptedLicense);
        return;
    }


    //
    // TryElevate
    //
    HRESULT TryElevate()
    {
        HRESULT hr = S_OK;
        HCURSOR hOriginalCursor = NULL;
        HCURSOR hWaitCursor = ::LoadCursor(NULL, IDC_WAIT);

        if (hWaitCursor)
        {
            hOriginalCursor = ::SetCursor(hWaitCursor);
        }

        // Ensure the elevated process is started.
        do
        {
            // hr = m_pCore->Elevate(m_hWnd); ToDo: // Uncomment when Elevate() method is implemented in sprint 2
            if (FAILED(hr))
            {
                int nResult = this->ModalPrompt(WIXUX_STATE_ERROR, MB_RETRYCANCEL, L"Failed to elevate.");
                if (IDRETRY == nResult)
                {
                    hr = S_FALSE;
                }
            }
        } while (S_FALSE == hr);

        if (hOriginalCursor)
        {
            ::SetCursor(hOriginalCursor);
        }

        return hr;
    }


    //
    // SetState
    //
    void SetState(
        __in WIXUX_STATE state
        )
    {
        if (m_state != state)
        {
            Trace2(REPORT_STANDARD, "CWixUserExperience::SetState() changing from state %d to %d", m_state, state);
            m_state = state;

            int nShowWelcomeMessage = (WIXUX_STATE_ACCEPT_LICENSE >= m_state) ? SW_SHOW : SW_HIDE;
            int nShowInstallLocationMessage = (WIXUX_STATE_SELECT_LOCATION == m_state) ? SW_SHOW : SW_HIDE;
            int nShowSelectFeaturesMessage = (WIXUX_STATE_SELECT_FEATURES == m_state) ? SW_SHOW : SW_HIDE;
            int nShowMaintenanceMessage = (WIXUX_STATE_MAINTENANCE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowConfiguringMessage = (WIXUX_STATE_CACHING <= m_state && WIXUX_STATE_EXECUTED > m_state) ? SW_SHOW : SW_HIDE;
            int nShowCompleteMessage = (WIXUX_STATE_EXECUTED == m_state) ? SW_SHOW : SW_HIDE;

            int nShowTopMessage = (WIXUX_STATE_SELECT_LOCATION == m_state) ? SW_SHOW : SW_HIDE;
            int nShowBottomMessage = (WIXUX_STATE_ACCEPT_LICENSE == m_state || WIXUX_STATE_SELECT_FEATURES == m_state) ? SW_SHOW : SW_HIDE;

            int nShowLicense = (WIXUX_STATE_ACCEPT_LICENSE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowAcceptLicense = (WIXUX_STATE_ACCEPT_LICENSE == m_state) ? SW_SHOW : SW_HIDE;

            int nShowPrint = (WIXUX_STATE_ACCEPT_LICENSE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowAdvanced = (WIXUX_STATE_ACCEPT_LICENSE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowInstall = (WIXUX_STATE_ACCEPT_LICENSE == m_state || WIXUX_STATE_SELECT_FEATURES == m_state) ? SW_SHOW : SW_HIDE;
            int nShowCancel = (WIXUX_STATE_ACCEPT_LICENSE <= m_state && WIXUX_STATE_EXECUTED > m_state) ? SW_SHOW : SW_HIDE;

            int nShowBack = (WIXUX_STATE_SELECT_LOCATION == m_state || WIXUX_STATE_SELECT_FEATURES == m_state) ? SW_SHOW : SW_HIDE;
            int nShowNext = (WIXUX_STATE_SELECT_LOCATION == m_state) ? SW_SHOW : SW_HIDE;

            int nShowRepair = (WIXUX_STATE_MAINTENANCE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowUninstall = (WIXUX_STATE_MAINTENANCE == m_state) ? SW_SHOW : SW_HIDE;
            int nShowClose = (BURN_DISPLAY_FULL == m_command.display && WIXUX_STATE_EXECUTED == m_state) ? SW_SHOW : SW_HIDE;

            int nShowProgressBar = ((WIXUX_STATE_CACHING <= m_state && WIXUX_STATE_EXECUTED > m_state) || WIXUX_STATE_ERROR == m_state) ? SW_SHOW : SW_HIDE;
            int nShowMessage = ((WIXUX_STATE_CACHING <= m_state && WIXUX_STATE_EXECUTED > m_state) || WIXUX_STATE_ERROR == m_state) ? SW_SHOW : SW_HIDE;

            // Set the correct title text.
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_WELCOME_TEXT].hWnd, nShowWelcomeMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_WELCOME_TEXT].hWnd, nShowWelcomeMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_INSTALL_LOCATION_TEXT].hWnd, nShowInstallLocationMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_INSTALL_LOCATION_TEXT].hWnd, nShowInstallLocationMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_SELECT_FEATURES_TEXT].hWnd, nShowSelectFeaturesMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_SELECT_FEATURES_TEXT].hWnd, nShowSelectFeaturesMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_MAINTENANCE_MODE_TEXT].hWnd, nShowMaintenanceMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_MAINTENANCE_MODE_TEXT].hWnd, nShowMaintenanceMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_CONFIGURING_TEXT].hWnd, nShowConfiguringMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_CONFIGURING_TEXT].hWnd, nShowConfiguringMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TITLE_COMPLETE_TEXT].hWnd, nShowCompleteMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_SUBTITLE_COMPLETE_TEXT].hWnd, nShowCompleteMessage);

            // Top/bottom message.
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_TOP_MESSAGE_TEXT].hWnd, nShowTopMessage);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_BOTTOM_MESSAGE_TEXT].hWnd, nShowBottomMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_LICENSE_RICHEDIT].hWnd, nShowLicense);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_ACCEPT_CHECKBOX].hWnd, nShowAcceptLicense);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_PRINT_BUTTON].hWnd, nShowPrint);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_ADVANCED_BUTTON].hWnd, nShowAdvanced);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_INSTALL_BUTTON].hWnd, nShowInstall);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_BACK_BUTTON].hWnd, nShowBack);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_NEXT_BUTTON].hWnd, nShowNext);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_REPAIR_BUTTON].hWnd, nShowRepair);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_UNINSTALL_BUTTON].hWnd, nShowUninstall);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_CLOSE_BUTTON].hWnd, nShowClose);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_PROGRESS_BAR].hWnd, nShowProgressBar);
            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_MESSAGE_TEXT].hWnd, nShowMessage);

            ::ShowWindow(m_pTheme->rgControls[WIXUX_CONTROL_CANCEL_BUTTON].hWnd, nShowCancel);
        }
    }


public:
    //
    // Constructor - intitialize member variables.
    //
    CWixUserExperience(
        __in HMODULE hModule,
        __in BURN_COMMAND* pCommand
        )
    {
        m_hUiThread = NULL;

        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BURN_COMMAND));

        m_cReferences = 1;
        memset(&m_Conditions, 0, sizeof(BUX_CONDITIONS));
        m_pTheme = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;
        m_hwndHover = NULL;
        m_pCore = NULL;

        m_fCanceled = FALSE;
        m_fFailed = FALSE;
        m_state = WIXUX_STATE_INITIALIZING;

        m_fWixMsiInstalled = FALSE;
        m_fAcceptedLicense = FALSE;
        m_fAllowRestart = FALSE;
        m_sczLanguage = NULL;

        m_cref = 1;
    }


    //
    // Destructor - release member variables.
    //
    ~CWixUserExperience()
    {
        BuxConditionsUninitialize(&m_Conditions);
    }

private:
    long m_cReferences;

    HANDLE m_hUiThread;

    HMODULE m_hModule;
    BURN_COMMAND m_command;

    IBurnCore* m_pCore;
    BUX_CONDITIONS m_Conditions;
    THEME* m_pTheme;
    BOOL m_fRegistered;
    HWND m_hWnd;
    HWND m_hwndHover;

    BOOL m_fCanceled;
    BOOL m_fFailed;
    WIXUX_STATE m_state;

    BOOL m_fWixMsiInstalled;
    BOOL m_fAcceptedLicense;
    BOOL m_fAllowRestart;
    LPWSTR m_sczLanguage;

    LONG m_cref;
};


//
// CreateUserExperience - creates a new IBurnUserExperience object.
//
HRESULT CreateUserExperience(
    __in HMODULE hModule,
    __in BURN_COMMAND* pCommand,
    __out IBurnUserExperience **ppUX
    )
{
    HRESULT hr = S_OK;
    CWixUserExperience* pUX = NULL;

    pUX = new CWixUserExperience(hModule, pCommand);
    ExitOnNull(pUX, hr, E_OUTOFMEMORY, "Failed to create new WiX UX object.");

    *ppUX = pUX;
    pUX = NULL;

LExit:
    return hr;
}
