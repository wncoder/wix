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
static const LPCWSTR PREQ_NETFX_PACKAGE_ID = L"MbaNetfxPackageId";

enum PREQBA_STATE
{
    PREQBA_STATE_INITIALIZING,
    PREQBA_STATE_INITIALIZED,
    PREQBA_STATE_DETECTING,
    PREQBA_STATE_DETECTED,
    PREQBA_STATE_PLANNING,
    PREQBA_STATE_PLANNED,
    PREQBA_STATE_APPLYING,
    PREQBA_STATE_APPLIED,
    PREQBA_STATE_FAILED,
};

enum WM_PREQBA
{
    WM_PREQBA_DETECT_PACKAGES = WM_APP + 1,
    WM_PREQBA_PLAN_PACKAGES,
    WM_PREQBA_APPLY_PACKAGES,
};

// This enum must be kept in the same order as the vrgwzPageNames array.
enum PREQBA_PAGE
{
    PREQBA_PAGE_WELCOME,
    PREQBA_PAGE_PROGRESS,
    PREQBA_PAGE_FAILURE,
    COUNT_PREQBA_PAGE,
};

// This array must be kept in the same order as the PREQBA_PAGE enum.
static LPCWSTR vrgwzPageNames[] = {
    L"Welcome",
    L"Progress",
    L"Failure",
};

enum PREQBA_CONTROL
{
    PREQBA_CONTROL_WELCOME_EULA_ACCEPT_CHECKBOX = THEME_FIRST_ASSIGN_CONTROL_ID,
    PREQBA_CONTROL_WELCOME_EULA_LINK,
    PREQBA_CONTROL_WELCOME_INSTALL_BUTTON,
    PREQBA_CONTROL_WELCOME_CANCEL_BUTTON,

    PREQBA_CONTROL_PROGRESS_BAR,
    PREQBA_CONTROL_PROGRESS_PACKAGE_NAME,
    PREQBA_CONTROL_PROGRESS_CANCEL_BUTTON,

    PREQBA_CONTROL_FAILED_TEXT,
    PREQBA_CONTROL_FAILED_LOG_LINK,
    PREQBA_CONTROL_FAILED_CANCEL_BUTTON,
};

static THEME_ASSIGN_CONTROL_ID vrgInitControls[] = {
    { PREQBA_CONTROL_WELCOME_EULA_ACCEPT_CHECKBOX, L"EulaAcceptCheckbox" },
    { PREQBA_CONTROL_WELCOME_EULA_LINK, L"EulaHyperlink" },
    { PREQBA_CONTROL_WELCOME_INSTALL_BUTTON, L"InstallButton" },
    { PREQBA_CONTROL_WELCOME_CANCEL_BUTTON, L"WelcomeCancelButton" },

    { PREQBA_CONTROL_PROGRESS_BAR, L"ProgressBar" },
    { PREQBA_CONTROL_PROGRESS_PACKAGE_NAME, L"ProgressPackageName" },
    { PREQBA_CONTROL_PROGRESS_CANCEL_BUTTON, L"ProgressCancelButton" },

    { PREQBA_CONTROL_FAILED_TEXT, L"FailureText" },
    { PREQBA_CONTROL_FAILED_LOG_LINK, L"FailureLogHyperlink" },
    { PREQBA_CONTROL_FAILED_CANCEL_BUTTON, L"FailureCancelButton" },
};

class CPreqUserExperience : public CBalBaseBootstrapperApplication
{
public: // IBootstrapperApplication overrides
    virtual STDMETHODIMP OnStartup()
    {
        HRESULT hr = S_OK;

        hr = BalGetStringVariable(PREQ_NETFX_PACKAGE_ID, &this->m_sczNetfxPackageId);
        BalExitOnFailure(hr, "Failed to read the NETFX Package identifier from engine variable.");

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
        int nResult = IDNOACTION;

        // Wait for UX thread to terminate.
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ::CloseHandle(m_hUiThread);
        }

        // If a restart was required.
        if (m_fRestartRequired)
        {
            // If the user allowed the restart then obviously we should take the
            // restart. If we did not show UI to ask the user then assume the restart
            // is allowed. Finally, if the command-line said take a reboot automatically
            // then take it because we need it.
            if (BOOTSTRAPPER_RESTART_NEVER != m_command.restart &&
                (m_fAllowRestart || BOOTSTRAPPER_DISPLAY_FULL > m_command.display || BOOTSTRAPPER_RESTART_PROMPT < m_command.restart))
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites scheduled a restart. The bootstrapper application will be reloaded after the computer is restarted.");
                nResult = IDRESTART;
            }
            else
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "A restart was required by the prerequisites. The bootstrapper application will be reloaded after the computer is restarted.");
            }
        }
        else if (m_fInstalledPrereqs)
        {
            BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites were successfully installed. The bootstrapper application will be reloaded.");
            nResult = IDRELOAD_BOOTSTRAPPER;
        }
        else
        {
            BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites were not successfully installed, error: 0x%x. The bootstrapper application will be not reloaded.", m_hrFinal);
        }

        return nResult;
    }

    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR /*wzBundleId*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION /*operation*/
        )
    {
        return CheckCanceled() ? IDCANCEL : IDNOACTION; // Ignore the related bundles since we're only interested in pre-reqs.
    }

    virtual STDMETHODIMP_(int) OnDetectRelatedMsiPackage(
        __in_z LPCWSTR /*wzPackageId*/,
        __in LPCWSTR /*wzProductCode*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION /*operation*/
        )
    {
        return CheckCanceled() ? IDCANCEL : IDNOACTION; // Ignore the related packages since we're only interested in pre-reqs.
    }

    virtual STDMETHODIMP_(void) OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state
        )
    {
        // TODO: Handle case where pre-reqs are actually on the machine. This bootstrapper application shouldn't be loaded if things were okay...
    }

    virtual STDMETHODIMP_(void) OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(PREQBA_STATE_DETECTED, hrStatus);

        // If we succeeded and we're not showing UI (where the user can click the Install button) then
        // go start the install automatically.
        if (SUCCEEDED(hrStatus) && BOOTSTRAPPER_DISPLAY_FULL != m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_PREQBA_PLAN_PACKAGES, 0, m_command.action);
        }
    }

    virtual STDMETHODIMP_(int) OnPlanRelatedBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        )
    {
        *pRequestedState = BOOTSTRAPPER_REQUEST_STATE_NONE; // do not touch related bundles since we're only installing pre-reqs.
        return CheckCanceled() ? IDCANCEL : IDOK;
    }

    virtual STDMETHODIMP_(int) OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout BOOTSTRAPPER_REQUEST_STATE *pRequestState
        )
    {
        // If we're planning to install a pre-req, install it. Skip everything else.
        if (BOOTSTRAPPER_ACTION_INSTALL == m_command.action && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, this->m_sczNetfxPackageId, -1))
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        }
        else
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
        }

        return CheckCanceled() ? IDCANCEL : IDOK;
    }

    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(PREQBA_STATE_PLANNED, hrStatus);

        if (SUCCEEDED(hrStatus))
        {
            ThemeSetProgressControl(m_pTheme, PREQBA_CONTROL_PROGRESS_BAR, 0);

            ::PostMessageW(m_hWnd, WM_PREQBA_APPLY_PACKAGES, 0, 0);
        }
    }

    virtual STDMETHODIMP_(int) OnCacheAcquireProgress(
        __in_z LPCWSTR /*wzPackageOrContainerId*/,
        __in_z_opt LPCWSTR /*wzPayloadId*/,
        __in DWORD64 /*dw64Progress*/,
        __in DWORD64 /*dw64Total*/,
        __in DWORD dwOverallProgressPercentage
        )
    {
    //    HRESULT hr = S_OK;
    //    WCHAR wzProgress[5] = { };

    //    hr = ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallPercentage);
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = ThemeSetTextControl(m_pTheme, PREQBA_CONTROL_PERCENTAGE_TEXT, wzProgress);
    //    }
    //    BalExitOnFailure(hr, "Failed to update cache percentage text.");

    //LExit:
        ThemeSetProgressControl(m_pTheme, PREQBA_CONTROL_PROGRESS_BAR, dwOverallProgressPercentage / 2);

        return CheckCanceled() ? IDCANCEL : IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnExecuteProgress(
        __in_z LPCWSTR /*wzPackageId*/,
        __in DWORD /*dwProgressPercentage*/,
        __in DWORD dwOverallProgressPercentage
        )
    {
    //    HRESULT hr = S_OK;
    //    WCHAR wzProgress[5] = { };

    //    hr = ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = ThemeSetTextControl(m_pTheme, PREQBA_CONTROL_PERCENTAGE_TEXT, wzProgress);
    //    }
    //    BalExitOnFailure(hr, "Failed to update execute percentage text.");

    //LExit:
        ThemeSetProgressControl(m_pTheme, PREQBA_CONTROL_PROGRESS_BAR, 50 + dwOverallProgressPercentage / 2);

        return CheckCanceled() ? IDCANCEL : IDNOACTION;
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

    virtual STDMETHODIMP_(int) OnApplyComplete(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        m_fRestartRequired = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart);

        //ThemeSetTextControl(m_pTheme, PREQBA_CONTROL_PROGRESS_TEXT, L"Installed");
        if (SUCCEEDED(hrStatus))
        {
            ThemeSetProgressControl(m_pTheme, PREQBA_CONTROL_PROGRESS_BAR, 100);
            m_fInstalledPrereqs = TRUE;
        }

        SetState(PREQBA_STATE_APPLIED, hrStatus);

        if (m_fRestartRequired && BOOTSTRAPPER_DISPLAY_FULL == m_command.display && BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
        {
            int nResult = ::MessageBoxW(m_hWnd, L"A restart is required to continue the installation. Restart now?", L"Restart Required", MB_YESNO | MB_ICONEXCLAMATION);
            m_fAllowRestart = (IDYES == nResult);
        }

        if (PREQBA_STATE_FAILED != m_state)
        {
            ::SendMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }

        return IDNOACTION;
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
                HRESULT hr = m_pEngine->SetLocalSource(wzPackageOrContainerId, wzPayloadId, ofn.lpstrFile);
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
        BalExitOnFailure(hr, "Failed to initialize COM.");
        fComInitialized = TRUE;

        // initialize theme util
        hr = ThemeInitialize(pThis->m_hModule);
        BalExitOnFailure(hr, "Failed to initialize theme manager.");

        // create main window
        hr = pThis->CreateMainWindow();
        BalExitOnFailure(hr, "Failed to create main window.");

        // Okay, we're ready for packages now.
        pThis->SetState(PREQBA_STATE_INITIALIZED, hr);
        ::PostMessageW(pThis->m_hWnd, WM_PREQBA_DETECT_PACKAGES, 0, 0);

        // message pump
        while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
        {
            if (-1 == fRet)
            {
                hr = E_UNEXPECTED;
                BalExitOnFailure(hr, "Unexpected return value from message pump.");
            }
            else
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
        }

    LExit:
        if (FAILED(hr) && SUCCEEDED(pThis->m_hrFinal))
        {
            pThis->m_hrFinal = hr;
        }

        // destroy main window
        pThis->DestroyMainWindow();

        // initiate engine shutdown
        pThis->m_pEngine->Quit(pThis->m_hrFinal);

        ThemeUninitialize();

        // uninitialize COM
        if (fComInitialized)
        {
            ::CoUninitialize();
        }

        return hr;
    }

    HRESULT CreateMainWindow()
    {
        HRESULT hr = S_OK;
        WNDCLASSW wc = { };
        DWORD dwWindowStyle = 0;
        LPWSTR sczThemePath = NULL;

        // load theme relative to mbapreq.dll.
        hr = PathRelativeToModule(&sczThemePath, L"mbapreq.thm", m_hModule);
        BalExitOnFailure(hr, "Failed to combine module path with mbapreq.thm.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        BalExitOnFailure(hr, "Failed to load theme from: mbapreq.thm.");

        // Parse any command line arguments that the engine did not process.
        if (m_command.wzCommandLine && *m_command.wzCommandLine)
        {
            hr = ProcessCommandLine(m_command.wzCommandLine);
            BalExitOnFailure(hr, "Unknown commandline parameters.");
        }

        hr = ThemeLoadLocFromFile(m_pTheme, L"mbapreq.wxl", m_hModule);
        BalExitOnFailure(hr, "Failed to localize from default language file: mbapreq.wxl.");

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

        case WM_NCDESTROY:
            {
            LRESULT lres = ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            return lres;
            }

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
            // If the user chose not to close, do not let the default window proc handle the message.
            if (!pUX->OnClose())
            {
                return 0;
            }
            break;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;

        case WM_PREQBA_DETECT_PACKAGES:
            pUX->OnDetect();
            return 0;

        case WM_PREQBA_PLAN_PACKAGES:
            pUX->OnPlan(static_cast<BOOTSTRAPPER_ACTION>(lParam));
            return 0;

        case WM_PREQBA_APPLY_PACKAGES:
            pUX->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case PREQBA_CONTROL_WELCOME_EULA_ACCEPT_CHECKBOX:
                pUX->OnClickAcceptCheckbox();
                return 0;

            case PREQBA_CONTROL_WELCOME_EULA_LINK:
                pUX->OnClickEula();
                break;

            case PREQBA_CONTROL_WELCOME_INSTALL_BUTTON:
                pUX->OnClickInstallButton();
                break;

            case PREQBA_CONTROL_WELCOME_CANCEL_BUTTON: __fallthrough;
            case PREQBA_CONTROL_PROGRESS_CANCEL_BUTTON: __fallthrough;
            case PREQBA_CONTROL_FAILED_CANCEL_BUTTON: __fallthrough;
                ::SendMessageW(hWnd, WM_CLOSE, 0, 0);
                break;

            default:
                break;
            }
            return 0;
        }

        return ThemeDefWindowProc(pUX ? pUX->m_pTheme : NULL, hWnd, uMsg, wParam, lParam);
    }

    BOOL OnCreate(
        __in HWND hWnd
        )
    {
        HRESULT hr = S_OK;

        hr = ThemeLoadControls(m_pTheme, hWnd, vrgInitControls, countof(vrgInitControls));
        BalExitOnFailure(hr, "Failed to load theme controls.");

        C_ASSERT(COUNT_PREQBA_PAGE == countof(vrgwzPageNames));
        C_ASSERT(countof(m_rgdwPageIds) == countof(vrgwzPageNames));

        ThemeGetPageIds(m_pTheme, vrgwzPageNames, m_rgdwPageIds, countof(m_rgdwPageIds));

    LExit:
        return SUCCEEDED(hr);
    }

    void OnDetect()
    {
        HRESULT hr = S_OK;

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pEngine->Detect();
        BalExitOnFailure(hr, "Failed to start detecting chain.");

    LExit:
        SetState(PREQBA_STATE_DETECTING, hr);
        return;
    }

    void OnPlan(
        __in BOOTSTRAPPER_ACTION action
        )
    {
        HRESULT hr = S_OK;

        hr = m_pEngine->Plan(action);
        BalExitOnFailure(hr, "Failed to start planning packages.");

    LExit:
        SetState(PREQBA_STATE_PLANNING, hr);
        return;
    }

    void OnApply()
    {
        HRESULT hr = S_OK;

        hr = m_pEngine->Apply(m_hWnd);
        BalExitOnFailure(hr, "Failed to start applying packages.");

    LExit:
        SetState(PREQBA_STATE_APPLYING, hr);
        return;
    }

    void OnClickAcceptCheckbox()
    {
        BOOL fAcceptedLicense = ThemeIsControlChecked(m_pTheme, PREQBA_CONTROL_WELCOME_EULA_ACCEPT_CHECKBOX);
        ThemeControlEnable(m_pTheme, PREQBA_CONTROL_WELCOME_INSTALL_BUTTON, fAcceptedLicense);
    }

    void OnClickEula()
    {
        HRESULT hr = S_OK;
        LPWSTR sczEulaPath = NULL;

        hr = PathRelativeToModule(&sczEulaPath, L"eula.rtf", m_hModule);
        BalExitOnFailure(hr, "Failed to create path to EULA.");

        hr = ShelExec(sczEulaPath, NULL, L"open", NULL, SW_SHOWNORMAL, NULL);
        BalExitOnFailure(hr, "Failed to launch EULA.");

    LExit:
        ReleaseStr(sczEulaPath);
        return;
    }

    void OnClickInstallButton()
    {
        ::PostMessageW(m_hWnd, WM_PREQBA_PLAN_PACKAGES, 0, BOOTSTRAPPER_ACTION_INSTALL);
    }

    void OnClickCloseButton()
    {
        ::SendMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }

    BOOL OnClose()
    {
        // Force the cancel if we are not showing UI or we're already on the success or failure page.
        // TODO: make prompt localizable string.
        BOOL fClose = PromptCancel(m_hWnd, BOOTSTRAPPER_DISPLAY_FULL != m_command.display || PREQBA_STATE_APPLIED <= m_state, L"Are you sure you want to cancel?", m_pTheme->wzCaption);
        if (fClose)
        {
            // TODO: disable all the cancel buttons.
        }

        return fClose;
    }

    void SetState(
        __in PREQBA_STATE state,
        __in HRESULT hrStatus
        )
    {
        DWORD dwOldPageId = 0;
        DWORD dwNewPageId = 0;

        if (FAILED(hrStatus))
        {
            m_hrFinal = hrStatus;
        }

        if (FAILED(m_hrFinal))
        {
            state = PREQBA_STATE_FAILED;
        }

        if (m_state != state)
        {
            DeterminePageId(m_state, &dwOldPageId);
            DeterminePageId(state, &dwNewPageId);

            m_state = state;

            if (dwOldPageId != dwNewPageId)
            {
                // Enable disable controls per-page.
                if (m_rgdwPageIds[PREQBA_PAGE_WELCOME] == dwNewPageId) // on the "Welcome" page, ensure the install button is enabled/disabled correctly.
                {
                    BOOL fAcceptedLicense = ThemeIsControlChecked(m_pTheme, PREQBA_CONTROL_WELCOME_EULA_ACCEPT_CHECKBOX);
                    ThemeControlEnable(m_pTheme, PREQBA_CONTROL_WELCOME_INSTALL_BUTTON, fAcceptedLicense);
                }

                ThemeShowPage(m_pTheme, dwOldPageId, SW_HIDE);
                ThemeShowPage(m_pTheme, dwNewPageId, SW_SHOW);
            }
        }
    }

    void DeterminePageId(
        __in PREQBA_STATE state,
        __out DWORD* pdwPageId
        )
    {
        switch (state)
        {
        case PREQBA_STATE_INITIALIZING:
            *pdwPageId = 0;
            break;

        case PREQBA_STATE_INITIALIZED: __fallthrough;
        case PREQBA_STATE_DETECTING: __fallthrough;
        case PREQBA_STATE_DETECTED:
            *pdwPageId = m_rgdwPageIds[PREQBA_PAGE_WELCOME];
            break;

        case PREQBA_STATE_PLANNING: __fallthrough;
        case PREQBA_STATE_PLANNED: __fallthrough;
        case PREQBA_STATE_APPLYING: __fallthrough;
        case PREQBA_STATE_APPLIED:
            *pdwPageId = m_rgdwPageIds[PREQBA_PAGE_PROGRESS];
            break;

        case PREQBA_STATE_FAILED:
            *pdwPageId = m_rgdwPageIds[PREQBA_PAGE_FAILURE];
            break;
        }
    }


public:
    CPreqUserExperience(
        __in HMODULE hModule,
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND* pCommand
        ) : CBalBaseBootstrapperApplication(pEngine, pCommand->restart)
    {
        m_hUiThread = NULL;
        m_dwThreadId = 0;

        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        m_pTheme = NULL;
        memset(m_rgdwPageIds, 0, sizeof(m_rgdwPageIds));
        m_fRegistered = FALSE;
        m_hWnd = NULL;
        m_hwndHover = NULL;

        m_fCanceled = FALSE;
        m_state = PREQBA_STATE_INITIALIZING;
        m_hrFinal = S_OK;

        m_fInstalledPrereqs = FALSE;
        m_fRestartRequired = FALSE;
        m_fAllowRestart = FALSE;

        m_sczNetfxPackageId = NULL;

        pEngine->AddRef();
        m_pEngine = pEngine;
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
        ReleaseObject(m_pEngine);
    }

private:
    HANDLE m_hUiThread;
    DWORD m_dwThreadId;

    HMODULE m_hModule;
    BOOTSTRAPPER_COMMAND m_command;
    IBootstrapperEngine* m_pEngine;
    THEME* m_pTheme;
    DWORD m_rgdwPageIds[countof(vrgwzPageNames)];
    BOOL m_fRegistered;
    HWND m_hWnd;
    HWND m_hwndHover;

    BOOL m_fCanceled;
    PREQBA_STATE m_state;
    HRESULT m_hrFinal;

    BOOL m_fInstalledPrereqs;
    BOOL m_fRestartRequired;
    BOOL m_fAllowRestart;

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
