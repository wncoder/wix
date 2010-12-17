//-------------------------------------------------------------------------------------------------
// <copyright file="WixStandardBootstrapperApplication.cpp" company="Microsoft">
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

static const LPCWSTR WIXSTDBA_WINDOW_CLASS = L"WixStdBA";
static const LPCWSTR WIXSTDBA_VARIABLE_OPTIONS_DLL_NAME = L"OptionsDll";
static const LPCWSTR WIXSTDBA_VARIABLE_INSTALL_FOLDER = L"InstallFolder";
static const LPCWSTR WIXSTDBA_VARIABLE_EULA_RTF_PATH = L"EulaFile";
static const LPCWSTR WIXSTDBA_VARIABLE_EULA_LINK_TARGET = L"EulaHyperlinkTarget";
static const LPCWSTR WIXSTDBA_VARIABLE_PROGRESS_ACTION = L"ProgressAction";
static const LPCWSTR WIXSTDBA_VARIABLE_PROGRESS_PACKAGE_NAME = L"ProgressPackageName";
static const LPCWSTR WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH = L"LaunchTarget";

enum WIXSTDBA_STATE
{
    WIXSTDBA_STATE_REFRESH,
    WIXSTDBA_STATE_INITIALIZING,
    WIXSTDBA_STATE_INITIALIZED,
    WIXSTDBA_STATE_DETECTING,
    WIXSTDBA_STATE_DETECTED,
    WIXSTDBA_STATE_PLANNING,
    WIXSTDBA_STATE_PLANNED,
    WIXSTDBA_STATE_APPLYING,
    WIXSTDBA_STATE_CACHING,
    WIXSTDBA_STATE_CACHED,
    WIXSTDBA_STATE_EXECUTING,
    WIXSTDBA_STATE_EXECUTED,
    WIXSTDBA_STATE_APPLIED,
    WIXSTDBA_STATE_FAILED,
};

enum WM_WIXSTDBA
{
    WM_WIXSTDBA_DETECT_PACKAGES = WM_APP + 100,
    WM_WIXSTDBA_PLAN_PACKAGES,
    WM_WIXSTDBA_APPLY_PACKAGES,
    WM_WIXSTDBA_MODAL_ERROR,
};

// This enum must be kept in the same order as the vrgwzPageNames array.
enum WIXSTDBA_PAGE
{
    WIXSTDBA_PAGE_LOADING,
    WIXSTDBA_PAGE_HELP,
    WIXSTDBA_PAGE_INSTALL,
    WIXSTDBA_PAGE_OPTIONS,
    WIXSTDBA_PAGE_MODIFY,
    WIXSTDBA_PAGE_PROGRESS,
    WIXSTDBA_PAGE_PROGRESS_PASSIVE,
    WIXSTDBA_PAGE_SUCCESS,
    WIXSTDBA_PAGE_FAILURE,
    COUNT_WIXSTDBA_PAGE,
};

// This array must be kept in the same order as the WIXSTDBA_PAGE enum.
static LPCWSTR vrgwzPageNames[] = {
    L"Loading",
    L"Help",
    L"Install",
    L"Options",
    L"Modify",
    L"Progress",
    L"ProgressPassive",
    L"Success",
    L"Failure",
};

enum WIXSTDBA_CONTROL
{
    // Non-paged controls
    WIXSTDBA_CONTROL_CLOSE_BUTTON = THEME_FIRST_ASSIGN_CONTROL_ID,
    WIXSTDBA_CONTROL_MINIMIZE_BUTTON,

    // Welcome page
    WIXSTDBA_CONTROL_INSTALL_BUTTON,
    WIXSTDBA_CONTROL_OPTIONS_BUTTON,
    WIXSTDBA_CONTROL_EULA_RICHEDIT,
    WIXSTDBA_CONTROL_EULA_LINK,
    WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX,
    WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON,

    // Options page
    WIXSTDBA_CONTROL_FOLDER_EDITBOX,
    WIXSTDBA_CONTROL_BROWSE_BUTTON,
    WIXSTDBA_CONTROL_OK_BUTTON,
    WIXSTDBA_CONTROL_CANCEL_BUTTON,

    // Modify page
    WIXSTDBA_CONTROL_REPAIR_BUTTON,
    WIXSTDBA_CONTROL_UNINSTALL_BUTTON,
    WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON,

    // Progress page
    WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR,
    WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT,
    WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR,
    WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT,
    WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR,
    WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT,
    WIXSTDBA_CONTROL_PROGRESS_MESSAGE_TEXT,
    WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON,

    // Success page
    WIXSTDBA_CONTROL_LAUNCH_BUTTON,
    WIXSTDBA_CONTROL_RESTART_BUTTON,
    WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON,

    // Failure page
    WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT,
    WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON,
};

static THEME_ASSIGN_CONTROL_ID vrgInitControls[] = {
    { WIXSTDBA_CONTROL_CLOSE_BUTTON, L"CloseButton" },
    { WIXSTDBA_CONTROL_MINIMIZE_BUTTON, L"MinimizeButton" },

    { WIXSTDBA_CONTROL_INSTALL_BUTTON, L"InstallButton" },
    { WIXSTDBA_CONTROL_OPTIONS_BUTTON, L"OptionsButton" },
    { WIXSTDBA_CONTROL_EULA_RICHEDIT, L"EulaRichedit" },
    { WIXSTDBA_CONTROL_EULA_LINK, L"EulaHyperlink" },
    { WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX, L"EulaAcceptCheckbox" },
    { WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON, L"WelcomeCancelButton" },

    { WIXSTDBA_CONTROL_FOLDER_EDITBOX, L"FolderEditbox" },
    { WIXSTDBA_CONTROL_BROWSE_BUTTON, L"BrowseButton" },
    { WIXSTDBA_CONTROL_OK_BUTTON, L"OptionsOkButton" },
    { WIXSTDBA_CONTROL_CANCEL_BUTTON, L"OptionsCancelButton" },

    { WIXSTDBA_CONTROL_REPAIR_BUTTON, L"RepairButton" },
    { WIXSTDBA_CONTROL_UNINSTALL_BUTTON, L"UninstallButton" },
    { WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON, L"ModifyCancelButton" },

    { WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR, L"CacheProgressbar" },
    { WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT, L"CacheProgressText" },
    { WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR, L"ExecuteProgressbar" },
    { WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT, L"ExecuteProgressText" },
    { WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR, L"OverallProgressbar" },
    { WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT, L"OverallProgressText" },
    { WIXSTDBA_CONTROL_PROGRESS_MESSAGE_TEXT, L"ProgressMessageText" },
    { WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON, L"ProgressCancelButton" },

    { WIXSTDBA_CONTROL_LAUNCH_BUTTON, L"LaunchButton" },
    { WIXSTDBA_CONTROL_RESTART_BUTTON, L"RestartButton" },
    { WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON, L"SuccessCancelButton" },

    { WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT, L"FailureMessageText" },
    { WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON, L"FailureCancelButton" },
};

class CWixStandardBootstrapperApplication : public CBalBaseBootstrapperApplication
{
public: // IBootstrapperApplication
    virtual STDMETHODIMP OnStartup()
    {
        HRESULT hr = S_OK;
        DWORD dwUIThreadId = 0;

        // create UI thread
        m_hUiThread = ::CreateThread(NULL, 0, UiThreadProc, this, 0, &dwUIThreadId);
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

        // wait for UI thread to terminate
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ReleaseHandle(m_hUiThread);
        }

        // If a restart was required.
        if (m_fRestartRequired)
        {
            // If the user allowed the restart then obviously we should take the
            // restart. If we did not show UI to ask the user then assume the restart
            // is allowed. Finally, if the command-line said take a reboot automatically
            // then take it because we need it.
            if (m_fAllowRestart || BOOTSTRAPPER_DISPLAY_FULL > m_command.display || BOOTSTRAPPER_RESTART_PROMPT < m_command.restart)
            {
                nResult = IDRESTART;
            }
        }

        return nResult;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR /*wzBundleId*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        // TODO: Show messagebox if we are downgrading.
        return BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation || CheckCanceled() ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedMsiPackage(
        __in_z LPCWSTR /*wzPackageId*/,
        __in LPCWSTR /*wzProductCode*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        return BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation || CheckCanceled() ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(void) OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_DETECTED, hrStatus);

        if (BOOTSTRAPPER_DISPLAY_FULL > m_command.display)
        {
            if (SUCCEEDED(hrStatus))
            {
                ::PostMessageW(m_hWnd, WM_WIXSTDBA_PLAN_PACKAGES, 0, m_command.action);
            }
            else
            {
                ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
            }
        }
    }


    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_PLANNED, hrStatus);

        if (SUCCEEDED(hrStatus))
        {
            ::PostMessageW(m_hWnd, WM_WIXSTDBA_APPLY_PACKAGES, 0, 0);
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL > m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }
    }


    virtual STDMETHODIMP_(int) OnApplyComplete(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        m_fRestartRequired = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart && BOOTSTRAPPER_RESTART_NEVER < m_command.restart);
        SetState(WIXSTDBA_STATE_APPLIED, hrStatus);

        // Clear our action variables.
        m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_ACTION, NULL);
        m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_PACKAGE_NAME, NULL);

        // embedded or quiet or passive display just close at the end, no questions asked.
        if (BOOTSTRAPPER_DISPLAY_FULL > m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }

        return IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnCacheAcquireProgress(
        __in_z LPCWSTR /*wzPackageOrContainerId*/,
        __in_z_opt LPCWSTR /*wzPayloadId*/,
        __in DWORD64 /*dw64Progress*/,
        __in DWORD64 /*dw64Total*/,
        __in DWORD dwOverallPercentage
        )
    {
        WCHAR wzProgress[5] = { };

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallPercentage);
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT, wzProgress);

        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR, dwOverallPercentage);

        return CheckCanceled() ? IDCANCEL : IDNOACTION;
    }


    virtual STDMETHODIMP_(void) OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_CACHED, hrStatus);
    }


    virtual STDMETHODIMP_(int) OnError(
        __in LPCWSTR /*wzPackageId*/,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        int nResult = IDNOACTION;

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
             HRESULT hr = m_pEngine->SendEmbeddedError(dwCode, wzError, dwUIHint, &nResult);
             if (FAILED(hr))
             {
                 nResult = IDERROR;
             }
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            nResult = ::MessageBoxW(m_hWnd, wzError, m_pTheme->wzCaption, dwUIHint);
        }
        else // just cancel when quiet or passive.
        {
            nResult = IDABORT;
        }

        return nResult;
    }


    virtual STDMETHODIMP_(int) OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };
        int nResult = IDNOACTION;

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
            hr = m_pEngine->SendEmbeddedProgress(dwProgressPercentage, dwOverallProgressPercentage, &nResult);
            BalExitOnFailure(hr, "Failed to send embedded progress.");
        }
        else
        {
            ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT, wzProgress);

            ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR, dwOverallProgressPercentage);
        }

    LExit:
        return FAILED(hr) ? IDERROR : CheckCanceled() ? IDCANCEL : nResult;
    }


    virtual STDMETHODIMP_(int) OnExecutePackageBegin(
        __in_z LPCWSTR wzPackageId,
        __in BOOL /*fExecute*/
        )
    {
        if (wzPackageId && *wzPackageId)
        {
            m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_PACKAGE_NAME, wzPackageId);
        }

        return CheckCanceled() ? IDCANCEL : IDNOACTION;
    }


    virtual int __stdcall  OnExecuteProgress(
        __in_z LPCWSTR /*wzPackageId*/,
        __in DWORD /*dwProgressPercentage*/,
        __in DWORD dwOverallProgressPercentage
        )
    {
        WCHAR wzProgress[5] = { };

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT, wzProgress);

        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR, dwOverallProgressPercentage);

        return CheckCanceled() ? IDCANCEL : IDNOACTION;
    }


    virtual STDMETHODIMP_(void) OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_EXECUTED, hrStatus);
    }


    virtual STDMETHODIMP_(int) OnResolveSource(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in_z LPCWSTR /*wzLocalSource*/,
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

            nResult = ::MessageBoxW(m_hWnd, sczText, sczCaption, MB_YESNOCANCEL | MB_ICONASTERISK);
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
        return CheckCanceled() ? IDCANCEL : nResult;
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
        CWixStandardBootstrapperApplication* pThis = (CWixStandardBootstrapperApplication*)pvContext;
        BOOL fComInitialized = FALSE;
        BOOL fRet = FALSE;
        MSG msg = { };

        // Initialize COM and theme.
        hr = ::CoInitialize(NULL);
        BalExitOnFailure(hr, "Failed to initialize COM.");
        fComInitialized = TRUE;

        hr = ThemeInitialize(pThis->m_hModule);
        BalExitOnFailure(hr, "Failed to initialize theme manager.");

        // Create main window.
        hr = pThis->CreateMainWindow();
        BalExitOnFailure(hr, "Failed to create main window.");

        // Okay, we're ready for packages now.
        pThis->SetState(WIXSTDBA_STATE_INITIALIZED, hr);
        ::PostMessageW(pThis->m_hWnd, WM_WIXSTDBA_DETECT_PACKAGES, 0, 0);

        // message pump
        while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
        {
            if (-1 == fRet)
            {
                hr = E_UNEXPECTED;
                BalExitOnFailure(hr, "Unexpected return value from message pump.");
            }
            else // TODO: bring this back when support loading a custom options dll, if (!::IsDialogMessageW(m_hwndOptionsDialog, &msg))
            {
                if (!ThemeTranslateAccelerator(pThis->m_pTheme, msg.hwnd, &msg))
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessageW(&msg);
                }
            }
        }

    LExit:
        // destroy main window
        pThis->DestroyMainWindow();

        // initiate engine shutdown
        pThis->m_pEngine->Quit(FAILED(hr) ? hr : pThis->m_hrFinal);

        ThemeUninitialize();

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
        LPWSTR sczCaption = NULL;

        // load theme relative to stdux.dll.
        hr = PathRelativeToModule(&sczThemePath, L"thm.xml", m_hModule);
        BalExitOnFailure(hr, "Failed to combine module path with thm.xml.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        BalExitOnFailure(hr, "Failed to load theme from thm.xml.");

        hr = ProcessCommandLine();
        BalExitOnFailure(hr, "Unknown commandline parameters.");

        if (NULL != m_sczLanguage)
        {
            hr = ThemeLoadLocFromFile(m_pTheme, m_sczLanguage, m_hModule);
            BalExitOnFailure(hr, "Failed to localize from /lang.");
        }
        else
        {
            hr = ThemeLoadLocFromFile(m_pTheme, L"thm.wxl", m_hModule);
            BalExitOnFailure(hr, "Failed to localize from fallback language.");
        }

        // Register the window class and create the window.
        wc.lpfnWndProc = CWixStandardBootstrapperApplication::WndProc;
        wc.hInstance = m_hModule;
        wc.hIcon = reinterpret_cast<HICON>(m_pTheme->hIcon);
        wc.hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        wc.hbrBackground = m_pTheme->rgFonts[m_pTheme->dwFontId].hBackground;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = WIXSTDBA_WINDOW_CLASS;
        if (!::RegisterClassW(&wc))
        {
            ExitWithLastError(hr, "Failed to register window.");
        }

        m_fRegistered = TRUE;

        // Calculate the window style based on the theme style and command display value.
        dwWindowStyle = m_pTheme->dwStyle;
        if (BOOTSTRAPPER_DISPLAY_NONE >= m_command.display)
        {
            dwWindowStyle &= ~WS_VISIBLE;
        }

        // Update the caption if there are any formated strings in it.
        hr = BalFormatString(m_pTheme->wzCaption, &sczCaption);
        if (SUCCEEDED(hr))
        {
            ThemeUpdateCaption(m_pTheme, sczCaption);
        }

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->wzCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
        ExitOnNullWithLastError(m_hWnd, hr, "Failed to create window.");

        hr = S_OK;

    LExit:
        ReleaseStr(sczCaption);
        ReleaseStr(sczThemePath);
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
        if (::IsWindow(m_hWnd))
        {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }

        if (m_fRegistered)
        {
            ::UnregisterClassW(WIXSTDBA_WINDOW_CLASS, m_hModule);
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
    HRESULT ProcessCommandLine()
    {
        HRESULT hr = S_OK;
        int argc = 0;
        LPWSTR* argv = NULL;

        if (m_command.wzCommandLine && *m_command.wzCommandLine)
        {
            argv = ::CommandLineToArgvW(m_command.wzCommandLine, &argc);
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
                        BalExitOnFailure(hr, "Failed to copy language.");

                        hr = StrAllocConcat(&m_sczLanguage, L".wxl", 0);
                        BalExitOnFailure(hr, "Failed to concatenate wxl extension.");
                    }
                }
            }
        }

    LExit:
        if (argv)
        {
            ::LocalFree(argv);
        }

        return hr;
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
#pragma warning(suppress:4312)
        CWixStandardBootstrapperApplication* pBA = reinterpret_cast<CWixStandardBootstrapperApplication*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_NCCREATE:
            {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pBA = reinterpret_cast<CWixStandardBootstrapperApplication*>(lpcs->lpCreateParams);
#pragma warning(suppress:4244)
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pBA));
            }
            break;

        case WM_NCDESTROY:
            {
            LRESULT lres = ThemeDefWindowProc(pBA ? pBA->m_pTheme : NULL, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            return lres;
            }

        case WM_CREATE:
            if (!pBA->OnCreate(hWnd))
            {
                return -1;
            }
            break;

        case WM_QUERYENDSESSION:
            if (ENDSESSION_CLOSEAPP == static_cast<DWORD>(lParam)) // deny Restart Manager requests to shutdown.
            {
                return FALSE;
            }
            break;

        case WM_CLOSE:
            // If the user chose not to close, do *not* let the default window proc handle the message.
            if (!pBA->OnClose())
            {
                return 0;
            }
            break;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;

        case WM_WIXSTDBA_DETECT_PACKAGES:
            pBA->OnDetect();
            return 0;

        case WM_WIXSTDBA_PLAN_PACKAGES:
            pBA->OnPlan(static_cast<BOOTSTRAPPER_ACTION>(lParam));
            return 0;

        case WM_WIXSTDBA_APPLY_PACKAGES:
            pBA->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case WIXSTDBA_CONTROL_EULA_LINK:
                pBA->OnClickEulaLink();
                return 0;

            case WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX:
                pBA->OnClickAcceptCheckbox();
                return 0;

            case WIXSTDBA_CONTROL_OPTIONS_BUTTON:
                pBA->OnClickOptionsButton();
                return 0;

            case WIXSTDBA_CONTROL_BROWSE_BUTTON:
                pBA->OnClickOptionsBrowseButton();
                return 0;

            case WIXSTDBA_CONTROL_OK_BUTTON:
                pBA->OnClickOptionsOkButton();
                return 0;

            case WIXSTDBA_CONTROL_CANCEL_BUTTON:
                pBA->OnClickOptionsCancelButton();
                return 0;

            case WIXSTDBA_CONTROL_INSTALL_BUTTON:
                pBA->OnClickInstallButton();
                return 0;

            case WIXSTDBA_CONTROL_REPAIR_BUTTON:
                pBA->OnClickRepairButton();
                return 0;

            case WIXSTDBA_CONTROL_UNINSTALL_BUTTON:
                pBA->OnClickUninstallButton();
                return 0;

            case WIXSTDBA_CONTROL_LAUNCH_BUTTON:
                pBA->OnClickLaunchButton();
                return 0;

            case WIXSTDBA_CONTROL_RESTART_BUTTON:
                pBA->OnClickRestartButton();
                return 0;

            case WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON: // progress cancel is special because it will cause rollback.
                pBA->OnClose();
                return 0;

            // The other cancel buttons just close the window.
            case WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_CLOSE_BUTTON:
                pBA->OnClickCloseButton();
                return 0;
            }
            break;
        }

        return ThemeDefWindowProc(pBA ? pBA->m_pTheme : NULL, hWnd, uMsg, wParam, lParam);
    }


    //
    // OnCreate - finishes loading the theme.
    //
    BOOL OnCreate(
        __in HWND hWnd
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczLicensePath = NULL;

        hr = ThemeLoadControls(m_pTheme, hWnd, vrgInitControls, countof(vrgInitControls));
        BalExitOnFailure(hr, "Failed to load theme controls.");

        C_ASSERT(COUNT_WIXSTDBA_PAGE == countof(vrgwzPageNames));
        C_ASSERT(countof(m_rgdwPageIds) == countof(vrgwzPageNames));

        ThemeGetPageIds(m_pTheme, vrgwzPageNames, m_rgdwPageIds, countof(m_rgdwPageIds));

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_LINK))
        {
            BOOL fEulaLink = BalStringVariableExists(WIXSTDBA_VARIABLE_EULA_LINK_TARGET);
            if (!fEulaLink)
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_ERROR, "Failed to find EULA hyperlink control target from variable '%ls'", WIXSTDBA_VARIABLE_EULA_LINK_TARGET);
            }
        }

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT))
        {
            hr = BalGetStringVariable(WIXSTDBA_VARIABLE_EULA_RTF_PATH, &sczLicensePath);
            if (SUCCEEDED(hr))
            {
                hr = ThemeLoadRichEditFromFile(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT, sczLicensePath, m_hModule);
            }

            if (FAILED(hr))
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_ERROR, "Failed to load file into EULA richedit control from variable '%ls' value: %ls", WIXSTDBA_VARIABLE_EULA_RTF_PATH, sczLicensePath);
                hr = S_OK;
            }
        }

    LExit:
        ReleaseStr(sczLicensePath);
        return SUCCEEDED(hr);
    }


    //
    // OnDetect - start the processing of packages.
    //
    void OnDetect()
    {
        HRESULT hr = S_OK;

        // If the splash screen is still around, close it since if there is a UI it's showing by now.
        if (::IsWindow(m_command.hwndSplashScreen))
        {
             ::PostMessageW(m_command.hwndSplashScreen, WM_CLOSE, 0, 0);
        }

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pEngine->Detect();
        BalExitOnFailure(hr, "Failed to start detecting chain.");

    LExit:
        SetState(WIXSTDBA_STATE_DETECTING, hr);
        return;
    }


    //
    // OnPlan - plan the detected changes.
    //
    void OnPlan(
        __in BOOTSTRAPPER_ACTION action
        )
    {
        HRESULT hr = S_OK;

        // TODO: need to localize the following strings eventually.
        switch (action)
        {
        case BOOTSTRAPPER_ACTION_INSTALL:
            m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_ACTION, L"Installing");
            break;
        case BOOTSTRAPPER_ACTION_REPAIR:
            m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_ACTION, L"Repairing");
            break;
        case BOOTSTRAPPER_ACTION_UNINSTALL:
            m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_PROGRESS_ACTION, L"Uninstalling");
            break;
        }

        hr = m_pEngine->Plan(action);
        BalExitOnFailure(hr, "Failed to start planning packages.");

    LExit:
        SetState(WIXSTDBA_STATE_PLANNING, hr);
        return;
    }


    //
    // OnApply - apply the packages.
    //
    void OnApply()
    {
        HRESULT hr = S_OK;

        hr = m_pEngine->Apply(m_hWnd);
        BalExitOnFailure(hr, "Failed to start applying packages.");

    LExit:
        SetState(WIXSTDBA_STATE_APPLYING, hr);
        return;
    }


    //
    // OnClose - called when the window is trying to be closed.
    //
    BOOL OnClose()
    {
        // Force the cancel if we are not showing UI or we're already on the success or failure page.
        // TODO: make prompt localizable string.
        BOOL fClose = PromptCancel(m_hWnd, BOOTSTRAPPER_DISPLAY_FULL != m_command.display || WIXSTDBA_STATE_APPLIED <= m_state, L"Are you sure you want to cancel?", m_pTheme->wzCaption);
        if (fClose)
        {
            // TODO: disable all the cancel buttons.
        }

        return fClose;
    }


    //
    // OnClickAcceptCheckbox - allow the install to continue.
    //
    void OnClickAcceptCheckbox()
    {
        BOOL fAcceptedLicense = ThemeIsControlChecked(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX);
        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_INSTALL_BUTTON, fAcceptedLicense);
    }


    //
    // OnClickOptionsButton - show the options page.
    //
    void OnClickOptionsButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczOptionsDllName = NULL;
        LPWSTR sczOptionsDllPath = NULL;

        hr = BalGetStringVariable(WIXSTDBA_VARIABLE_OPTIONS_DLL_NAME, &sczOptionsDllName);
        if (SUCCEEDED(hr))
        {
            hr = PathRelativeToModule(&sczOptionsDllPath, sczOptionsDllName, m_hModule);
            BalExitOnFailure(hr, "Failed to get options dialog path.");

            // TODO: load options DLL and get window handle.
        }
        else
        {
            m_fShowOptionsPage = TRUE;
            SetState(WIXSTDBA_STATE_REFRESH, S_OK);
        }

    LExit:
        ReleaseStr(sczOptionsDllPath);
        ReleaseStr(sczOptionsDllName);
        return;
    }

    //
    // OnClickOptionsBrowseButton - browse for install folder on the options page.
    //
    void OnClickOptionsBrowseButton()
    {
        WCHAR wzPath[MAX_PATH] = { };
        BROWSEINFOW browseInfo = { };
        PIDLIST_ABSOLUTE pidl = NULL;

        browseInfo.hwndOwner = m_hWnd;
        browseInfo.pszDisplayName = wzPath;
        browseInfo.lpszTitle = m_pTheme->wzCaption;
        browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        pidl = ::SHBrowseForFolderW(&browseInfo);
        if (pidl && ::SHGetPathFromIDListW(pidl, wzPath))
        {
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, wzPath);
        }

        if (pidl)
        {
            ::CoTaskMemFree(pidl);
        }

        return;
    }

    //
    // OnClickOptionsOkButton - accept the changes made by the options page.
    //
    void OnClickOptionsOkButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczPath = NULL;
        THEME_PAGE* pPage = NULL;

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX))
        {
            hr = ThemeGetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, &sczPath);
            ExitOnFailure(hr, "Failed to get text from folder edit box.");

            // TODO: verify the path is valid.

            hr = m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_INSTALL_FOLDER, sczPath);
            ExitOnFailure(hr, "Failed to set the install folder.");
        }

        // Loop through all the checkbox controls with names and set a Burn variable
        // with that name to true or false.
        pPage = ThemeGetPage(m_pTheme, m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS]);
        if (pPage)
        {
            for (DWORD i = 0; i < pPage->cControlIndices; ++i)
            {
                THEME_CONTROL* pControl = m_pTheme->rgControls + pPage->rgdwControlIndices[i];
                if (THEME_CONTROL_TYPE_CHECKBOX == pControl->type && pControl->wzName && *pControl->wzName)
                {
                    BOOL bChecked = ThemeIsControlChecked(m_pTheme, pControl->wId);
                    m_pEngine->SetVariableNumeric(pControl->wzName, bChecked ? 1 : 0);
                }
            }
        }

    LExit:
        m_fShowOptionsPage = FALSE;
        SetState(WIXSTDBA_STATE_REFRESH, S_OK);
        return;
    }


    //
    // OnClickOptionsCacelButton - discard the changes made by the options page.
    //
    void OnClickOptionsCancelButton()
    {
        m_fShowOptionsPage = FALSE;
        SetState(WIXSTDBA_STATE_REFRESH, S_OK);
        return;
    }


    //
    // OnClickInstallButton - start the install by planning the packages.
    //
    void OnClickInstallButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_INSTALL);
    }


    //
    // OnClickRepairButton - start the repair.
    //
    void OnClickRepairButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_REPAIR);
    }


    //
    // OnClickUninstallButton - start the uninstall.
    //
    void OnClickUninstallButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_UNINSTALL);
    }


    //
    // OnClickCloseButton - close the application.
    //
    void OnClickCloseButton()
    {
        ::SendMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }


    //
    // OnClickEulaLink - show the end user license agreement.
    //
    void OnClickEulaLink()
    {
        HRESULT hr = S_OK;
        LPWSTR sczLicenseUrl = NULL;
        LPWSTR sczLicensePath = NULL;
        URI_PROTOCOL protocol = URI_PROTOCOL_UNKNOWN;

        hr = BalGetStringVariable(WIXSTDBA_VARIABLE_EULA_LINK_TARGET, &sczLicenseUrl);
        BalExitOnFailure(hr, "Failed to get URL to EULA.");

        hr = UriProtocol(sczLicenseUrl, &protocol);
        if (FAILED(hr) || URI_PROTOCOL_UNKNOWN == protocol)
        {
            hr = PathRelativeToModule(&sczLicensePath, sczLicenseUrl, m_hModule);
        }

        hr = ShelExec(sczLicensePath ? sczLicensePath : sczLicenseUrl, NULL, L"open", NULL, SW_SHOWDEFAULT, NULL);
        BalExitOnFailure(hr, "Failed to launch URL to EULA.");

    LExit:
        ReleaseStr(sczLicensePath);
        ReleaseStr(sczLicenseUrl);
        return;
    }


    //
    // OnClickLaunchButton - launch the app from the success page.
    //
    void OnClickLaunchButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczLaunchTarget = NULL;

        hr = BalGetStringVariable(WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH, &sczLaunchTarget);
        BalExitOnFailure1(hr, "Failed to get launch target variable '%ls'.", WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);

        hr = ShelExec(sczLaunchTarget, NULL, L"open", NULL, SW_SHOWDEFAULT, NULL);
        BalExitOnFailure1(hr, "Failed to launch target: %ls", sczLaunchTarget);

        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);

    LExit:
        ReleaseStr(sczLaunchTarget);
        return;
    }


    //
    // OnClickRestartButton - allows the restart and closes the app.
    //
    void OnClickRestartButton()
    {
        m_fAllowRestart = TRUE;
        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);

        return;
    }


    //
    // SetState
    //
    void SetState(
        __in WIXSTDBA_STATE state,
        __in HRESULT hrStatus
        )
    {
        DWORD dwOldPageId = 0;
        DWORD dwNewPageId = 0;
        LPWSTR sczText = NULL;
        LPWSTR sczUnformattedText = NULL;

        ::EnterCriticalSection(&m_csState);

        if (FAILED(hrStatus))
        {
            m_hrFinal = hrStatus;
        }

        if (FAILED(m_hrFinal))
        {
            state = WIXSTDBA_STATE_FAILED;
        }

        if (WIXSTDBA_STATE_REFRESH == state || m_state != state)
        {
            DeterminePageId(m_state, &dwOldPageId);
            DeterminePageId(state, &dwNewPageId);

            if (WIXSTDBA_STATE_REFRESH == state)
            {
                if (m_fShowOptionsPage)
                {
                    dwNewPageId = m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS];
                }
                else
                {
                    dwNewPageId = dwOldPageId;
                    dwOldPageId = m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS];
                }
            }
            else
            {
                m_state = state;
            }

            if (dwOldPageId != dwNewPageId)
            {
                // Enable disable controls per-page.
                if (m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL] == dwNewPageId) // on the "Install" page, ensure the install button is enabled/disabled correctly.
                {
                    BOOL fAcceptedLicense = !ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX) || ThemeIsControlChecked(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_INSTALL_BUTTON, fAcceptedLicense);

                    // If there is an "Options" page and the "Options" button exists then enable the button.
                    BOOL fOptionsEnbaled = m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] && ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_OPTIONS_BUTTON);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_OPTIONS_BUTTON, fOptionsEnbaled);
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] == dwNewPageId)
                {
                    HRESULT hr = BalGetStringVariable(WIXSTDBA_VARIABLE_INSTALL_FOLDER, &sczUnformattedText);
                    if (SUCCEEDED(hr))
                    {
                        BalFormatString(sczUnformattedText, &sczText);
                        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, sczText);
                    }
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_SUCCESS] == dwNewPageId) // on the "Success" page, check if the restart or launch button should be enabled.
                {
                    if (m_fRestartRequired)
                    {
                        BOOL fShowRestartButton = FALSE;
                        if (BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
                        {
                            fShowRestartButton = TRUE;
                        }

                        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_RESTART_BUTTON, fShowRestartButton);
                        ThemeShowControl(m_pTheme, WIXSTDBA_CONTROL_RESTART_BUTTON, fShowRestartButton ? SW_SHOW : SW_HIDE);
                    }
                    else if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON))
                    {
                        BOOL fLaunchTargetExists = BalStringVariableExists(WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);
                        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON, fLaunchTargetExists);
                    }
                }

                // Process each control for special handling in the new page.
                THEME_PAGE* pPage = ThemeGetPage(m_pTheme, dwNewPageId);
                if (pPage)
                {
                    for (DWORD i = 0; i < pPage->cControlIndices; ++i)
                    {
                        THEME_CONTROL* pControl = m_pTheme->rgControls + pPage->rgdwControlIndices[i];

                        // If we are on the options page and this is a named checkbox control, try to set its default
                        // state to the state of a matching named Burn variable.
                        if (m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] == dwNewPageId && THEME_CONTROL_TYPE_CHECKBOX == pControl->type && pControl->wzName && *pControl->wzName)
                        {
                            LONGLONG llValue = 0;
                            HRESULT hr = m_pEngine->GetVariableNumeric(pControl->wzName, &llValue);

                            ThemeSendControlMessage(m_pTheme, pControl->wId, BM_SETCHECK, SUCCEEDED(hr) && llValue ? BST_CHECKED : BST_UNCHECKED, 0);
                        }

                        // Format the text in each of the new page's controls (if they have any text).
                        if (pControl->wzText && *pControl->wzText)
                        {
                            HRESULT hr = BalFormatString(pControl->wzText, &sczText);
                            if (SUCCEEDED(hr))
                            {
                                ThemeSetTextControl(m_pTheme, pControl->wId, sczText);
                            }
                        }
                    }
                }

                ThemeShowPage(m_pTheme, dwOldPageId, SW_HIDE);
                ThemeShowPage(m_pTheme, dwNewPageId, SW_SHOW);
            }
        }

        ::LeaveCriticalSection(&m_csState);

        ReleaseStr(sczText);
        ReleaseStr(sczUnformattedText);
    }


    void DeterminePageId(
        __in WIXSTDBA_STATE state,
        __out DWORD* pdwPageId
        )
    {
        if (BOOTSTRAPPER_DISPLAY_PASSIVE == m_command.display)
        {
            *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] ? m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] : m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS];
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            switch (state)
            {
            case WIXSTDBA_STATE_INITIALIZING:
                *pdwPageId = 0;
                break;

            case WIXSTDBA_STATE_INITIALIZED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_LOADING];
                break;

            case WIXSTDBA_STATE_DETECTING: __fallthrough;
            case WIXSTDBA_STATE_DETECTED:
                switch (m_command.action)
                {
                case BOOTSTRAPPER_ACTION_HELP:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_HELP] ? m_rgdwPageIds[WIXSTDBA_PAGE_HELP] : m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL];
                    break;

                case BOOTSTRAPPER_ACTION_INSTALL:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL];
                    break;

                case BOOTSTRAPPER_ACTION_MODIFY: __fallthrough;
                case BOOTSTRAPPER_ACTION_REPAIR: __fallthrough;
                case BOOTSTRAPPER_ACTION_UNINSTALL:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_MODIFY];
                    break;
                }
                break;

            case WIXSTDBA_STATE_PLANNING: __fallthrough;
            case WIXSTDBA_STATE_PLANNED: __fallthrough;
            case WIXSTDBA_STATE_APPLYING: __fallthrough;
            case WIXSTDBA_STATE_CACHING: __fallthrough;
            case WIXSTDBA_STATE_CACHED: __fallthrough;
            case WIXSTDBA_STATE_EXECUTING: __fallthrough;
            case WIXSTDBA_STATE_EXECUTED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS];
                break;

            case WIXSTDBA_STATE_APPLIED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_SUCCESS];
                break;

            case WIXSTDBA_STATE_FAILED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_FAILURE];
                break;
            }
        }
    }


public:
    //
    // Constructor - intitialize member variables.
    //
    CWixStandardBootstrapperApplication(
        __in HMODULE hModule,
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND* pCommand
        ) : CBalBaseBootstrapperApplication(pEngine, pCommand->restart)
    {
        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        m_pTheme = NULL;
        memset(m_rgdwPageIds, 0, sizeof(m_rgdwPageIds));
        m_hUiThread = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;

        ::InitializeCriticalSection(&m_csState);
        m_fShowOptionsPage = FALSE;
        m_state = WIXSTDBA_STATE_INITIALIZING;
        m_hrFinal = S_OK;

        m_fRestartRequired = FALSE;
        m_fAllowRestart = FALSE;

        m_sczLanguage = NULL;
        m_sczFamilyBundleId = NULL;

        pEngine->AddRef();
        m_pEngine = pEngine;
    }


    //
    // Destructor - release member variables.
    //
    ~CWixStandardBootstrapperApplication()
    {
        AssertSz(!::IsWindow(m_hWnd), "Window should have been destroyed before destructor.");
        AssertSz(!m_pTheme, "Theme should have been released before destuctor.");

        ::DeleteCriticalSection(&m_csState);

        ReleaseStr(m_sczFamilyBundleId);
        ReleaseNullObject(m_pEngine);
    }

private:
    HMODULE m_hModule;
    BOOTSTRAPPER_COMMAND m_command;
    IBootstrapperEngine* m_pEngine;

    THEME* m_pTheme;
    DWORD m_rgdwPageIds[countof(vrgwzPageNames)];
    HANDLE m_hUiThread;
    BOOL m_fRegistered;
    HWND m_hWnd;

    CRITICAL_SECTION m_csState;
    BOOL m_fShowOptionsPage;
    WIXSTDBA_STATE m_state;
    HRESULT m_hrFinal;

    BOOL m_fRestartRequired;
    BOOL m_fAllowRestart;

    LPWSTR m_sczLanguage;
    LPWSTR m_sczFamilyBundleId;
};


//
// CreateUserExperience - creates a new IBurnUserExperience object.
//
HRESULT CreateBootstrapperApplication(
    __in HMODULE hModule,
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication** ppApplication
    )
{
    HRESULT hr = S_OK;
    CWixStandardBootstrapperApplication* pApplication = NULL;

    pApplication = new CWixStandardBootstrapperApplication(hModule, pEngine, pCommand);
    ExitOnNull(pApplication, hr, E_OUTOFMEMORY, "Failed to create new standard bootstrapper application object.");

    *ppApplication = pApplication;
    pApplication = NULL;

LExit:
    ReleaseObject(pApplication);
    return hr;
}
