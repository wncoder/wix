//-------------------------------------------------------------------------------------------------
// <copyright file="PreqUserExperience.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


static const LPCWSTR PREQ_WINDOW_CLASS = L"WixBurnMbaPreq";

enum PREQ_STATE
{
    PREQ_STATE_INITIALIZING,
    PREQ_STATE_DETECTING,
    PREQ_STATE_DETECTED,
    PREQ_STATE_PLANNING,
    PREQ_STATE_PLANNED,
    PREQ_STATE_APPLYING,
    PREQ_STATE_APPLIED,
};

enum PREQ_CONTROL
{
    PREQ_CONTROL_INTRODUCTION_TEXT,
    PREQ_CONTROL_EULA_LABEL_TEXT,
    PREQ_CONTROL_EULA_LINK,
    PREQ_CONTROL_PROGRESS_TEXT,
    PREQ_CONTROL_PERCENTAGE_TEXT,

    PREQ_CONTROL_INSTALL_BUTTON,
    PREQ_CONTROL_CLOSE_BUTTON,
};

enum WM_PREQ
{
    WM_PREQ_DETECT_PACKAGES = WM_APP + 1,
    WM_PREQ_PLAN_PACKAGES,
    WM_PREQ_APPLY_PACKAGES,
};


class CPreqUserExperience : public CBalBaseBootstrapperApplication
{
public: // IBootstrapperApplication overrides
    virtual STDMETHODIMP OnStartup()
    {
        HRESULT hr = S_OK;

        hr = ReadNetfxPackageId();
        ExitOnFailure(hr, "Failed to read the NETFX Package identifier from engine variable.");

        // Create UI thread.
        m_hUiThread = ::CreateThread(NULL, 0, UiThreadProc, this, 0, &m_dwThreadId);
        if (!m_hUiThread)
        {
            ExitWithLastError(hr, "Failed to create UI thread.");
        }

    LExit:
        return hr;
    }

    virtual STDMETHODIMP_(int) OnShutdown()
    {
        // Wait for UX thread to terminate.
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ::CloseHandle(m_hUiThread);
        }

        return IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR /*wzBundleId*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION /*operation*/
        )
    {
        return IDNOACTION; // Ignore the related bundles since we're only interested in NETFX.
    }

    virtual STDMETHODIMP_(int) OnDetectRelatedMsiPackage(
        __in_z LPCWSTR /*wzPackageId*/,
        __in LPCWSTR /*wzProductCode*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION /*operation*/
        )
    {
        return IDNOACTION; // Ignore the related packages since we're only interested in NETFX.
    }

    virtual STDMETHODIMP_(void) OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state
        )
    {
        // TODO: Handle case where NETFX is actually on the machine. This bootstrapper application shouldn't be loaded if things were okay...
    }

    virtual STDMETHODIMP_(void) OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        if (SUCCEEDED(hrStatus))
        {
            SetState(PREQ_STATE_DETECTED);
            // TODO: only do this in quiet mode, ::PostMessageW(m_hWnd, WM_PREQ_PLAN_PACKAGES, 0, BOOTSTRAPPER_ACTION_INSTALL);
        }
        else
        {
            // TODO: handle error.
        }
    }

    virtual STDMETHODIMP_(int) OnPlanRelatedBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        )
    {
        *pRequestedState = BOOTSTRAPPER_REQUEST_STATE_NONE; // Do not touch related bundles since we're only installing NETFX.
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout BOOTSTRAPPER_REQUEST_STATE *pRequestState
        )
    {
        // If we're planning NETFX, install it. Skip everything else.
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, this->m_sczNetfxPackageId, -1))
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        }
        else
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
        }

        return IDOK;
    }

    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        if (SUCCEEDED(hrStatus))
        {
            SetState(PREQ_STATE_PLANNED);
            ::PostMessageW(m_hWnd, WM_PREQ_APPLY_PACKAGES, 0, 0);
        }
        else
        {
            // TODO: handle error.
        }
    }

    virtual STDMETHODIMP_(int) OnApplyBegin()
    {
        HRESULT hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PROGRESS_TEXT, L"Initializing...");
        if (SUCCEEDED(hr))
        {
            hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PERCENTAGE_TEXT, L"");
        }

        return FAILED(hr) ? IDERROR : IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnCacheBegin()
    {
        HRESULT hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PROGRESS_TEXT, L"Downloading...");
        if (SUCCEEDED(hr))
        {
            hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PERCENTAGE_TEXT, L"0%");
        }

        return FAILED(hr) ? IDERROR : IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnCacheAcquireProgress(
        __in_z LPCWSTR /*wzPackageOrContainerId*/,
        __in_z_opt LPCWSTR /*wzPayloadId*/,
        __in DWORD64 /*dw64Progress*/,
        __in DWORD64 /*dw64Total*/,
        __in DWORD dwOverallPercentage
        )
    {
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };

        hr = ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallPercentage);
        if (SUCCEEDED(hr))
        {
            hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PERCENTAGE_TEXT, wzProgress);
        }

        return FAILED(hr) ? IDERROR : IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        HRESULT hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PROGRESS_TEXT, L"Installing...");
        if (SUCCEEDED(hr))
        {
            hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PERCENTAGE_TEXT, L"0%");
        }

        return FAILED(hr) ? IDERROR : IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnExecuteProgress(
        __in_z LPCWSTR /*wzPackageId*/,
        __in DWORD /*dwProgressPercentage*/,
        __in DWORD dwOverallProgressPercentage
        )
    {
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };

        hr = ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
        if (SUCCEEDED(hr))
        {
            hr = ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PERCENTAGE_TEXT, wzProgress);
        }

        return IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnError(
        __in LPCWSTR /*wzPackageId*/,
        __in DWORD /*dwCode*/,
        __in_z LPCWSTR /*wzError*/,
        __in DWORD /*dwUIHint*/
        )
    {
        // TODO: Handle error.
        return IDERROR;
    }

    virtual STDMETHODIMP_(int) OnExecutePackageComplete(
        __in LPCWSTR /*wzPackageId*/,
        __in HRESULT /*hrExitCode*/,
        __in BOOTSTRAPPER_APPLY_RESTART /*restart*/
        )
    {
        return IDNOACTION; // Always return no action here and we'll force the reboot later.
    }

    virtual STDMETHODIMP_(int) OnApplyComplete(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        BOOL fRestart = FALSE;

        ThemeSetTextControl(m_pTheme, PREQ_CONTROL_PROGRESS_TEXT, L"Installed");
        SetState(PREQ_STATE_APPLIED);

        // Failure or quiet or passive display just close at the end, no questions asked.
        if (BOOTSTRAPPER_DISPLAY_NONE == m_command.display || BOOTSTRAPPER_DISPLAY_PASSIVE == m_command.display)
        {
            fRestart = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
        {
            int nResult = ::MessageBoxW(m_hWnd, L"A restart is required to continue the installation. Restart now?", L"Restart Required", MB_YESNO | MB_ICONQUESTION);
            fRestart = (IDYES == nResult);
        }

        // Save this to enable us to detect failed apply phase
        m_hrFinal = hrStatus;

        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        return fRestart ? IDRESTART : IDOK;
    }

    virtual STDMETHODIMP_(int) OnResolveSource(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in_z LPCWSTR wzLocalSource,
        __in_z_opt LPCWSTR wzDownloadSource
        )
    {
        int nResult = IDNO;
        LPWSTR sczCaption = NULL;
        LPWSTR sczText = NULL;

        LPCWSTR wzId = wzPayloadId ? wzPayloadId : wzPackageOrContainerId;
        LPCWSTR wzContainerOrPayload = wzPayloadId ? L"payload" : L"container";
        StrAllocFormatted(&sczCaption, L"Resolve Source for %ls: %ls", wzContainerOrPayload, wzId);
        if (wzDownloadSource)
        {
            StrAllocFormatted(&sczText, L"The %ls has a download url: %ls\nWould you like to download?", wzContainerOrPayload, wzDownloadSource);

            nResult = ::MessageBoxW(m_hWnd, sczText, sczCaption, MB_YESNOCANCEL | MB_ICONQUESTION);
        }

        if (IDYES == nResult)
        {
            nResult = IDDOWNLOAD;
        }
        else if (IDNO == nResult)
        {
            // Prompt to change the source location.
            OPENFILENAMEW ofn = { };
            WCHAR wzFile[MAX_PATH] = { };

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = m_hWnd;
            ofn.lpstrFile = wzFile;
            ofn.nMaxFile = countof(wzFile);
            ofn.lpstrFilter = L"All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            ofn.lpstrTitle = sczCaption;

            if (::GetOpenFileNameW(&ofn))
            {
                HRESULT hr = m_pCore->SetLocalSource(wzPackageOrContainerId, wzPayloadId, ofn.lpstrFile);
                nResult = SUCCEEDED(hr) ? IDRETRY : IDERROR;
            }
            else
            {
                nResult = IDCANCEL;
            }
        }

        ReleaseStr(sczText);
        ReleaseStr(sczCaption);
        return nResult;
    }

private: // privates
    static DWORD WINAPI UiThreadProc(
        __in LPVOID pvContext
        )
    {
        HRESULT hr = S_OK;
        CPreqUserExperience* pThis = static_cast<CPreqUserExperience*>(pvContext);
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
        pThis->m_pCore->Quit(pThis->m_hrFinal);

        // uninitialize COM
        if (fComInitialized)
        {
            ::CoUninitialize();
        }

        return hr;
    }

    HRESULT ReadNetfxPackageId()
    {
        HRESULT hr = S_OK;
        DWORD cchNetfxPackageId = 20;

        hr = StrAlloc(&this->m_sczNetfxPackageId, cchNetfxPackageId);
        ExitOnFailure(hr, "Failed to allocate string for NETFX package id.");

        hr = m_pCore->GetVariableString(L"MbaNetfxPackageId", this->m_sczNetfxPackageId, &cchNetfxPackageId);
        if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
        {
            hr = StrAlloc(&this->m_sczNetfxPackageId, cchNetfxPackageId);
            ExitOnFailure(hr, "Failed to allocate string for NETFX package id.");

            hr = m_pCore->GetVariableString(L"MbaNetfxPackageId", this->m_sczNetfxPackageId, &cchNetfxPackageId);
        }
        ExitOnRootFailure(hr, "Failed to get NETFX package id from engine.");

    LExit:
        return hr;
    }

    HRESULT CreateMainWindow()
    {
        HRESULT hr = S_OK;
        WNDCLASSW wc = { };
        DWORD dwWindowStyle = 0;
        LPWSTR sczThemePath = NULL;

        // load theme relative to mbapreq.dll.
        hr = PathRelativeToModule(&sczThemePath, L"preqthm.xml", m_hModule);
        ExitOnFailure(hr, "Failed to combine module path with preqthm.xml.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        ExitOnFailure(hr, "Failed to load theme from preqthm.xml.");

        // Parse any command line arguments that the engine did not process.
        if (m_command.wzCommandLine && *m_command.wzCommandLine)
        {
            hr = ProcessCommandLine(m_command.wzCommandLine);
            ExitOnFailure(hr, "Unknown commandline parameters.");
        }

        hr = ThemeLoadLocFromFile(m_pTheme, L"preq_en-us.wxl", m_hModule);
        ExitOnFailure(hr, "Failed to localize from default language.");

        // Register the window class and create the window.
        wc.style = 0;
        wc.lpfnWndProc = CPreqUserExperience::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = m_hModule;
        wc.hIcon = reinterpret_cast<HICON>(m_pTheme->hIcon);
        wc.hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        wc.hbrBackground = m_pTheme->rgFonts[m_pTheme->dwFontId].hBackground;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = PREQ_WINDOW_CLASS;
        if (!::RegisterClassW(&wc))
        {
            ExitWithLastError(hr, "Failed to register window.");
        }

        m_fRegistered = TRUE;

        // Calculate the window style based on the theme style and command display value.
        dwWindowStyle = m_pTheme->dwStyle;
        if (BOOTSTRAPPER_DISPLAY_NONE == m_command.display)
        {
            dwWindowStyle &= ~WS_VISIBLE;
        }

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->wzCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
        ExitOnNullWithLastError(m_hWnd, hr, "Failed to create window.");

    LExit:
        ReleaseStr(sczThemePath);

        if (FAILED(hr))
        {
            DestroyMainWindow();
        }

        return hr;
    }

    void DestroyMainWindow()
    {
        if (m_hWnd)
        {
            ::CloseWindow(m_hWnd);
            m_hWnd = NULL;
        }

        if (m_fRegistered)
        {
            ::UnregisterClassW(PREQ_WINDOW_CLASS, m_hModule);
            m_fRegistered = FALSE;
        }
    }

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
            }
        }

    LExit:
        ReleaseStr(sczName);

        ::LocalFree(argv);

        return hr;
    }


    static LRESULT CALLBACK WndProc(
        __in HWND hWnd,
        __in UINT uMsg,
        __in WPARAM wParam,
        __in LPARAM lParam
        )
    {
        LRESULT lres = 0;
#pragma warning(suppress:4312)
        CPreqUserExperience* pUX = reinterpret_cast<CPreqUserExperience*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_NCCREATE:
            {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pUX = reinterpret_cast<CPreqUserExperience*>(lpcs->lpCreateParams);
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

        case WM_QUERYENDSESSION:
            // Deny Restart Manager requests to shutdown.
            if (ENDSESSION_CLOSEAPP == static_cast<DWORD>(lParam))
            {
                return FALSE;
            }
            break;

        case WM_CLOSE:
            if (PREQ_STATE_APPLIED > pUX->m_state)
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

        case WM_PREQ_DETECT_PACKAGES:
            pUX->OnDetect();
            return 0;

        case WM_PREQ_PLAN_PACKAGES:
            pUX->OnPlan(static_cast<BOOTSTRAPPER_ACTION>(lParam));
            return 0;

        case WM_PREQ_APPLY_PACKAGES:
            pUX->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case PREQ_CONTROL_EULA_LINK:
                pUX->OnClickEula();
                break;

            case PREQ_CONTROL_INSTALL_BUTTON:
                pUX->OnClickInstallButton();
                break;

            case PREQ_CONTROL_CLOSE_BUTTON:
                pUX->OnClickCloseButton();
                break;

            default:
                break;
            }
            return 0;
        }

        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    BOOL OnCreate(
        __in HWND hWnd
        )
    {
        HRESULT hr = S_OK;

        hr = ThemeLoadControls(m_pTheme, hWnd, NULL, 0);
        ExitOnFailure(hr, "Failed to load theme controls.");

        // Okay, we're ready for packages now.
        ::PostMessageW(hWnd, WM_PREQ_DETECT_PACKAGES, 0, 0);

    LExit:
        return SUCCEEDED(hr);
    }

    void OnDetect()
    {
        HRESULT hr = S_OK;

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pCore->Detect();
        ExitOnFailure(hr, "Failed to start detecting chain.");

        SetState(PREQ_STATE_DETECTING);

    LExit:
        return;
    }

    void OnPlan(
        __in BOOTSTRAPPER_ACTION action
        )
    {
        HRESULT hr = S_OK;

        hr = m_pCore->Plan(action);
        ExitOnFailure(hr, "Failed to start planning packages.");

        SetState(PREQ_STATE_PLANNING);

    LExit:
        return;
    }

    void OnApply()
    {
        HRESULT hr = S_OK;

        hr = m_pCore->Apply(m_hWnd);
        ExitOnFailure(hr, "Failed to start applying packages.");

        SetState(PREQ_STATE_APPLYING);

    LExit:
        return;
    }

    void OnClickEula()
    {
        HRESULT hr = S_OK;
        LPWSTR sczEulaPath = NULL;

        hr = PathRelativeToModule(&sczEulaPath, L"eula.rtf", m_hModule);
        ExitOnFailure(hr, "Failed to create path to EULA.");

        hr = ShelExec(sczEulaPath, NULL, L"open", NULL, SW_SHOWNORMAL, NULL);
        ExitOnFailure(hr, "Failed to launch EULA.");

    LExit:
        ReleaseStr(sczEulaPath);
        return;
    }

    void OnClickInstallButton()
    {
        ::PostMessageW(m_hWnd, WM_PREQ_PLAN_PACKAGES, 0, BOOTSTRAPPER_ACTION_INSTALL);
    }

    void OnClickCloseButton()
    {
        m_fCanceled = TRUE;
        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }

    void SetState(
        __in PREQ_STATE state
        )
    {
        if (m_state != state)
        {
            m_state = state;

            int nShowEula = (PREQ_STATE_PLANNING > m_state ) ? SW_SHOW : SW_HIDE;
            int nShowInstall = (PREQ_STATE_PLANNING > m_state) ? SW_SHOW : SW_HIDE;
            int nShowProgress = (PREQ_STATE_DETECTED < m_state) ? SW_SHOW : SW_HIDE;
            int nShowPercentage = (PREQ_STATE_DETECTED < m_state && PREQ_STATE_APPLIED != m_state) ? SW_SHOW : SW_HIDE;
            int nShowClose = SW_SHOW;

            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_INTRODUCTION_TEXT].hWnd, nShowEula);
            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_EULA_LABEL_TEXT].hWnd, nShowEula);
            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_EULA_LINK].hWnd, nShowEula);

            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_PROGRESS_TEXT].hWnd, nShowProgress);
            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_PERCENTAGE_TEXT].hWnd, nShowPercentage);

            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_INSTALL_BUTTON].hWnd, nShowInstall);
            ::ShowWindow(m_pTheme->rgControls[PREQ_CONTROL_CLOSE_BUTTON].hWnd, nShowClose);
        }
    }


public:
    CPreqUserExperience(
        __in HMODULE hModule,
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND* pCommand
        ) : CBalBaseBootstrapperApplication(pCommand->restart)
    {
        m_hUiThread = NULL;
        m_dwThreadId = 0;

        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        m_pTheme = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;
        m_hwndHover = NULL;

        m_fCanceled = FALSE;
        m_state = PREQ_STATE_INITIALIZING;
        m_hrFinal = S_OK;

        m_sczNetfxPackageId = NULL;

        pEngine->AddRef();
        m_pCore = pEngine;
    }

    ~CPreqUserExperience()
    {
        DestroyMainWindow();

        if (m_pTheme)
        {
            ThemeFree(m_pTheme);
            m_pTheme = NULL;
        }

        ReleaseNullStr(m_sczNetfxPackageId);
        ReleaseObject(m_pCore);
    }

private:
    HANDLE m_hUiThread;
    DWORD m_dwThreadId;

    HMODULE m_hModule;
    BOOTSTRAPPER_COMMAND m_command;
    IBootstrapperEngine* m_pCore;
    THEME* m_pTheme;
    BOOL m_fRegistered;
    HWND m_hWnd;
    HWND m_hwndHover;

    BOOL m_fCanceled;
    PREQ_STATE m_state;
    HRESULT m_hrFinal;

    LPWSTR m_sczNetfxPackageId;
};


HRESULT CreateBootstrapperApplication(
    __in HMODULE hModule,
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication **ppApplication
    )
{
    HRESULT hr = S_OK;
    CPreqUserExperience* pUX = NULL;

    pUX = new CPreqUserExperience(hModule, pEngine, pCommand);
    ExitOnNull(pUX, hr, E_OUTOFMEMORY, "Failed to create new standard bootstrapper application object.");

    *ppApplication = pUX;
    pUX = NULL;

LExit:
    ReleaseObject(pUX);
    return hr;
}
