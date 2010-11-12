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
static const LPCWSTR WIXSTDBA_VARIABLE_EULA_RTF_PATH = L"EulaFile";
static const LPCWSTR WIXSTDBA_VARIABLE_EULA_LINK_TARGET = L"EulaHyperlinkTarget";
static const LPCWSTR WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH = L"LaunchTarget";

enum WIXSTDBA_STATE
{
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
    WIXSTDBA_STATE_ERROR,
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
        // wait for UX thread to terminate
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ReleaseHandle(m_hUiThread);
        }

        return IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR /*wzBundleId*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        // TODO: Show messagebox if we are downgrading.
        return BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedMsiPackage(
        __in_z LPCWSTR /*wzPackageId*/,
        __in LPCWSTR /*wzProductCode*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        return BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation ? IDCANCEL : IDOK;
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
        BOOL fRestart = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart && BOOTSTRAPPER_RESTART_NEVER < m_command.restart);

        SetState(WIXSTDBA_STATE_APPLIED, hrStatus);

        if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            // If a restart is expected but we are showing full UI and expected to prompt, check with the user if it is okay
            // to actually restart now.
            if (fRestart && BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
            {
                int nResult = ::MessageBoxW(m_hWnd, L"A restart is required to complete this action. Restart now?", L"Restart Required", MB_YESNO | MB_ICONQUESTION);
                fRestart = (IDYES == nResult);
            }
        }
        else // embedded or quiet or passive display just close at the end, no questions asked.
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }

        return fRestart ? IDRESTART : IDOK;
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

        return m_fCanceled ? IDCANCEL : IDNOACTION;
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
            nResult = ::MessageBoxW(m_hWnd, wzError, L"Error", dwUIHint);
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
            ExitOnFailure(hr, "Failed to send embedded progress.");
        }
        else
        {
            ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT, wzProgress);

            ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR, dwOverallProgressPercentage);
        }

    LExit:
        return FAILED(hr) ? IDERROR : m_fCanceled ? IDCANCEL : nResult;
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

        return m_fCanceled ? IDCANCEL : IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnExecuteMsiMessage(
        __in_z LPCWSTR /*wzPackageId*/,
        __in INSTALLMESSAGE /*mt*/,
        __in UINT /*uiFlags*/,
        __in_z LPCWSTR /*wzMessage*/
        )
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnExecuteMsiFilesInUse(
        __in_z LPCWSTR /*wzPackageId*/,
        __in DWORD /*cFiles*/,
        __in LPCWSTR* /*rgwzFiles*/
        )
    {
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT /*hrExitCode*/,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        HRESULT hr = S_OK;
        BOOL fRestart = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart && BOOTSTRAPPER_RESTART_NEVER < m_command.restart);
        LPWSTR sczRestartPackage = NULL;

        // If a restart is expected but we are showing full UI and expected to prompt, check with the user if it is okay
        // to actually restart now.
        if (fRestart && BOOTSTRAPPER_DISPLAY_FULL == m_command.display && BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
        {
            hr = StrAllocFormatted(&sczRestartPackage, L"Package %ls requires a restart. Restart now?", wzPackageId);
            ExitOnFailure(hr, "Failed to format restart package.");

            int nResult = ::MessageBoxW(m_hWnd, sczRestartPackage, L"Restart Required", MB_YESNO | MB_ICONQUESTION);
            fRestart = (IDYES == nResult);
        }

    LExit:
        ReleaseStr(sczRestartPackage);
        return fRestart ? IDRESTART : IDOK;
    }


    virtual STDMETHODIMP_(void) OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_EXECUTED, hrStatus);
    }


    virtual int __stdcall OnResolveSource(
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
        ExitOnFailure(hr, "Failed to initialize COM.");
        fComInitialized = TRUE;

        hr = ThemeInitialize(pThis->m_hModule);
        ExitOnFailure(hr, "Failed to initialize theme manager.");

        // Create main window.
        hr = pThis->CreateMainWindow();
        ExitOnFailure(hr, "Failed to create main window.");

        // Okay, we're ready for packages now.
        pThis->SetState(WIXSTDBA_STATE_INITIALIZED, hr);
        ::PostMessageW(pThis->m_hWnd, WM_WIXSTDBA_DETECT_PACKAGES, 0, 0);

        // If the splash screen is around, close it since we're showing our UI now.
        if (::IsWindow(pThis->m_command.hwndSplashScreen))
        {
             ::PostMessageW(pThis->m_command.hwndSplashScreen, WM_CLOSE, 0, 0);
        }

        // message pump
        while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
        {
            if (-1 == fRet)
            {
                hr = E_UNEXPECTED;
                ExitOnFailure(hr, "Unexpected return value from message pump.");
            }
            else if (!::IsDialogMessageW(msg.hwnd, &msg))
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

        // load theme relative to stdux.dll.
        hr = PathRelativeToModule(&sczThemePath, L"thm.xml", m_hModule);
        ExitOnFailure(hr, "Failed to combine module path with thm.xml.");

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        ExitOnFailure(hr, "Failed to load theme from thm.xml.");

        hr = ProcessCommandLine();
        ExitOnFailure(hr, "Unknown commandline parameters.");

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

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->wzCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
        ExitOnNullWithLastError(m_hWnd, hr, "Failed to create window.");

    LExit:
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
                        ExitOnFailure(hr, "Failed to copy language.");

                        hr = StrAllocConcat(&m_sczLanguage, L".wxl", 0);
                        ExitOnFailure(hr, "Failed to concatenate wxl extension.");
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

        case WM_CLOSE:
            if (WIXSTDBA_STATE_EXECUTED > pBA->m_state)
            {
                // Check with the user to verify they want to cancel.
                //pBA->PromptCancel(hWnd);
            }
            pBA->m_fCanceled = TRUE; // TODO: let the prompt set this instead.

            // If the user chose not to cancel, do not let the default window proc
            // handle the message.
            if (!pBA->m_fCanceled)
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
                break;

            case WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX:
                pBA->OnClickAcceptCheckbox();
                break;

            case WIXSTDBA_CONTROL_INSTALL_BUTTON:
                pBA->OnClickInstallButton();
                break;

            case WIXSTDBA_CONTROL_REPAIR_BUTTON:
                pBA->OnClickRepairButton();
                break;

            case WIXSTDBA_CONTROL_UNINSTALL_BUTTON:
                pBA->OnClickUninstallButton();
                break;

            case WIXSTDBA_CONTROL_LAUNCH_BUTTON:
                pBA->OnClickLaunchButton();
                break;

            case WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON:
            case WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON:
            case WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON:
            case WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON:
            case WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON:
            case WIXSTDBA_CONTROL_CLOSE_BUTTON:
                pBA->OnClickCloseButton();
                break;

            default:
                break;
            }
            return 0;
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
        ExitOnFailure(hr, "Failed to load theme controls.");

        C_ASSERT(COUNT_WIXSTDBA_PAGE == countof(vrgwzPageNames));
        C_ASSERT(countof(m_rgdwPageIds) == countof(vrgwzPageNames));

        ThemeGetPageIds(m_pTheme, vrgwzPageNames, m_rgdwPageIds, countof(m_rgdwPageIds));

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_LINK))
        {
            BOOL fEulaLink = BalStringVariableExists(m_pEngine, WIXSTDBA_VARIABLE_EULA_LINK_TARGET);
            if (!fEulaLink)
            {
                BalLog(m_pEngine, BOOTSTRAPPER_LOG_LEVEL_ERROR, "Failed to find EULA hyperlink control target from variable '%ls'", WIXSTDBA_VARIABLE_EULA_LINK_TARGET);
            }
        }

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT))
        {
            hr = BalGetStringVariable(m_pEngine, WIXSTDBA_VARIABLE_EULA_RTF_PATH, &sczLicensePath);
            if (SUCCEEDED(hr))
            {
                hr = ThemeLoadRichEditFromFile(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT, sczLicensePath, m_hModule);
            }

            if (FAILED(hr))
            {
                BalLog(m_pEngine, BOOTSTRAPPER_LOG_LEVEL_ERROR, "Failed to load file into EULA richedit control from variable '%ls' value: %ls", WIXSTDBA_VARIABLE_EULA_RTF_PATH, sczLicensePath);
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

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pEngine->Detect();
        ExitOnFailure(hr, "Failed to start detecting chain.");

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

        hr = m_pEngine->Plan(action);
        ExitOnFailure(hr, "Failed to start planning packages.");

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
        ExitOnFailure(hr, "Failed to start applying packages.");

    LExit:
        SetState(WIXSTDBA_STATE_APPLYING, hr);
        return;
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
        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }


    //
    // OnClickEulaLink - show the end user license agreement.
    //
    void OnClickEulaLink()
    {
        HRESULT hr = S_OK;
        WCHAR wzLicenseUrl[1024] = { };
        DWORD cchLicenseUrl = countof(wzLicenseUrl);

        hr = m_pEngine->GetVariableString(L"EulaUrl", wzLicenseUrl, &cchLicenseUrl);
        ExitOnFailure(hr, "Failed to get URL to EULA.");

        hr = ShelExec(wzLicenseUrl, NULL, L"open", NULL, SW_SHOWDEFAULT, NULL);
        ExitOnFailure(hr, "Failed to launch URL to EULA.");

    LExit:
        return;
    }


    //
    // OnClickLaunchButton - launch the app from the success page.
    //
    void OnClickLaunchButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczLaunchTarget = NULL;

        hr = BalGetStringVariable(m_pEngine, WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH, &sczLaunchTarget);
        ExitOnFailure1(hr, "Failed to get launch target variable '%ls'.", WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);

        hr = ShelExec(sczLaunchTarget, NULL, L"open", NULL, SW_SHOWDEFAULT, NULL);
        ExitOnFailure1(hr, "Failed to launch target: %ls", sczLaunchTarget);

    LExit:
        ReleaseStr(sczLaunchTarget);
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

        if (FAILED(hrStatus))
        {
            m_hrFinal = hrStatus;
            m_state = WIXSTDBA_STATE_FAILED;
        }

        if (m_state != state)
        {
            DeterminePageId(m_state, &dwOldPageId);
            DeterminePageId(state, &dwNewPageId);

            m_state = state;

            if (dwOldPageId != dwNewPageId)
            {
                // Enable disable controls per-page.
                if (m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL] == dwNewPageId) // on the "Install" page, ensure the install button is enabled/disabled correctly.
                {
                    BOOL fAcceptedLicense = !ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX) || ThemeIsControlChecked(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_INSTALL_BUTTON, fAcceptedLicense);
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_SUCCESS] == dwNewPageId) // on the "Success" page, check if the launch button should be enabled.
                {
                    if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON))
                    {
                        BOOL fLaunchTargetExists = BalStringVariableExists(m_pEngine, WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);
                        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON, fLaunchTargetExists);
                    }
                }

                ThemeShowPage(m_pTheme, dwOldPageId, SW_HIDE);
                ThemeShowPage(m_pTheme, dwNewPageId, SW_SHOW);
            }
        }
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
                case BOOTSTRAPPER_ACTION_REPAIR:
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

            case WIXSTDBA_STATE_ERROR: __fallthrough;
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
        ) : CBalBaseBootstrapperApplication(pCommand->restart)
    {
        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        m_pTheme = NULL;
        memset(m_rgdwPageIds, 0, sizeof(m_rgdwPageIds));
        m_hUiThread = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;

        m_state = WIXSTDBA_STATE_INITIALIZING;
        m_hrFinal = S_OK;
        m_fCanceled = FALSE;

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

    WIXSTDBA_STATE m_state;
    HRESULT m_hrFinal;
    BOOL m_fCanceled;

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
