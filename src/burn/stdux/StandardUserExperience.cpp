//-------------------------------------------------------------------------------------------------
// <copyright file="StandardUserExperience.cpp" company="Microsoft">
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
    STDUX_CONTROL_CACHE_TEXT,
    STDUX_CONTROL_CACHE_PROGRESS_TEXT,
    STDUX_CONTROL_EXECUTE_TEXT,
    STDUX_CONTROL_EXECUTE_PROGRESS_TEXT,
    STDUX_CONTROL_OVERALL_PROGRESS_TEXT,

    STDUX_CONTROL_MESSAGE_TEXT,
    STDUX_CONTROL_OK_BUTTON,
    STDUX_CONTROL_CANCEL_BUTTON,
    STDUX_CONTROL_RETRY_BUTTON,
    STDUX_CONTROL_ABORT_BUTTON,
    STDUX_CONTROL_IGNORE_BUTTON,
    STDUX_CONTROL_YES_BUTTON,
    STDUX_CONTROL_NO_BUTTON,
};

enum WM_STDUX
{
    WM_STDUX_DETECT_PACKAGES = WM_APP + 1,
    WM_STDUX_PLAN_PACKAGES,
    WM_STDUX_APPLY_PACKAGES,
    WM_STDUX_MODAL_ERROR,
};


class CStandardUserExperience : public IBootstrapperApplication
{
public: // IUnknown
    virtual STDMETHODIMP QueryInterface(
        __in REFIID riid,
        __out LPVOID *ppvObject
        )
    {
        HRESULT hr = S_OK;

        ExitOnNull(ppvObject, hr, E_INVALIDARG, "Invalid argument ppvObject");
        *ppvObject = NULL;

        if (::IsEqualIID(__uuidof(IBootstrapperApplication), riid))
        {
            *ppvObject = static_cast<IBootstrapperApplication*>(this);
        }
        else if (::IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else // no interface for requested iid
        {
            ExitFunction1(hr = E_NOINTERFACE);
        }

        AddRef();
    LExit:
        return hr;
    }

    virtual STDMETHODIMP_(ULONG) AddRef()
    {
        return ::InterlockedIncrement(&this->m_cReferences);
    }

    virtual STDMETHODIMP_(ULONG) Release()
    {
        long l = ::InterlockedDecrement(&this->m_cReferences);
        if (0 < l)
        {
            return l;
        }

        delete this;
        return 0;
    }

public: // IBootstrapperApplication
    //
    // OnStartup - called when application is started.
    //
    virtual STDMETHODIMP OnStartup()
    {
        HRESULT hr = S_OK;
        DWORD dwUIThreadId = 0;

        WriteEvent("OnStartup()");

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

    virtual STDMETHODIMP_(int) OnShutdown()
    {
        WriteEvent("OnShutdown()");

        // wait for UX thread to terminate
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ::CloseHandle(m_hUiThread);
        }

        if (m_hModalWait)
        {
            ::CloseHandle(m_hModalWait);
            m_hModalWait = NULL;
        }

        return IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnDetectBegin(
        __in DWORD cPackages
        )
    {
        WriteEvent("OnDetectBegin() - cPackages: %u", cPackages);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )
    {
        WriteEvent("OnDetectPackageBegin() - wzPackageId: %ls", wzPackageId);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR wzBundleId,
        __in BOOL fPerMachine,
        __in DWORD64 dw64Version,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        WriteEvent("OnDetectRelatedBundle() - wzBundleId: %ls, fPerMachine: %u, dw64Version: %I64u, operation: %u", wzBundleId, fPerMachine, dw64Version, operation);
        if (BOOTSTRAPPER_ACTION_INSTALL == m_command.action && BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation)
        {
            return IDCANCEL;
        }
        else
        {
            return IDOK;
        }
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedMsiPackage(
        __in_z LPCWSTR wzPackageId,
        __in LPCWSTR wzProductCode,
        __in BOOL fPerMachine,
        __in DWORD64 dw64Version,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        WriteEvent("OnDetectRelatedMsiPackage() - wzPackageId: %ls, wzProductCode: %ls, fPerMachine: %u, dw64Version: %I64u, operation: %u", wzPackageId, wzProductCode, fPerMachine, dw64Version, operation);
        if (BOOTSTRAPPER_ACTION_INSTALL == m_command.action && BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation)
        {
            return IDCANCEL;
        }
        else
        {
            return IDOK;
        }
    }

    virtual STDMETHODIMP_(int) OnDetectTargetMsiPackage(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzProductCode,
        __in BOOTSTRAPPER_PACKAGE_STATE patchState
        )
    {
        WriteEvent("OnDetectTargetMsiPackage() - wzPackageId: %ls, wzProductCode: %ls, patchState: %u", wzPackageId, wzProductCode, patchState);
        return IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnDetectMsiFeature(
        __in LPCWSTR wzPackageId,
        __in LPCWSTR wzFeatureId,
        __in BOOTSTRAPPER_FEATURE_STATE state
        )
    {
        WriteEvent("OnDetectMsiFeature() - wzPackageId: %ls, wzFeatureId: %ls, state: %u", wzPackageId, wzFeatureId, state);
        return IDOK;
    }

    virtual STDMETHODIMP_(void) OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state
        )
    {
        WriteEvent("OnDetectPackageComplete() - wzPackageId: %ls, hrStatus: 0x%x, state: %u", wzPackageId, hrStatus, state);
    }


    virtual STDMETHODIMP_(void) OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnDetectComplete() - hrStatus: 0x%x", hrStatus);
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_DETECTED);
            ::PostMessageW(m_hWnd, WM_STDUX_PLAN_PACKAGES, 0, m_command.action);
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL != m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }
    }


    virtual STDMETHODIMP_(int) OnPlanBegin(
        __in DWORD cPackages
        )
    {
        WriteEvent("OnPlanBegin() - cPackages: %u", cPackages);
        return IDOK;
    }


    virtual int __stdcall OnPlanRelatedBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        )
    {
        WriteEvent("OnPlanRelatedBundle() - wzBundleId: %ls, pRequestedState: %d", wzBundleId, *pRequestedState);

        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout BOOTSTRAPPER_REQUEST_STATE *pRequestState
        )
    {
        WriteEvent("OnPlanPackageBegin() - wzPackageId: %ls, pRequestState: %d", wzPackageId, *pRequestState);

        // this is an opportunity to modify the REQUEST_STATE of this package
        // e.g. *pRequestedState = REQUEST_STATE_PRESENT;
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnPlanTargetMsiPackage(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzProductCode,
        __inout BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        )
    {
        WriteEvent("OnPlanTargetMsiPackage() - wzPackageId: %ls, wzProductCode: %ls, pRequestState: %d", wzPackageId, wzProductCode, *pRequestedState);
        return IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnPlanMsiFeature(
        __in LPCWSTR wzPackageId,
        __in LPCWSTR wzFeatureId,
        __inout BOOTSTRAPPER_FEATURE_STATE* pRequestedState
        )
    {
        WriteEvent("OnPlanMsiFeature() - wzPackageId: %ls, wzFeatureId: %ls, pRequestedState: %u", wzPackageId, wzFeatureId, *pRequestedState);
        return IDOK;
    }


    virtual STDMETHODIMP_(void) OnPlanPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state,
        __in BOOTSTRAPPER_REQUEST_STATE requested,
        __in BOOTSTRAPPER_ACTION_STATE execute,
        __in BOOTSTRAPPER_ACTION_STATE rollback
        )
    {
        WriteEvent("OnPlanPackageComplete() - wzPackageId: %ls, hrStatus: 0x%x, state: %u, requested: %u, execute: %u, rollback: %u", wzPackageId, hrStatus, state, requested, execute, rollback);
    }


    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnPlanComplete() - hrStatus: 0x%x", hrStatus);
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_PLANNED);
            ::PostMessageW(m_hWnd, WM_STDUX_APPLY_PACKAGES, 0, 0);
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL != m_command.display)
        {
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }
    }


    virtual STDMETHODIMP_(int) OnApplyBegin()
    {
        WriteEvent("OnApplyBegin()");
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnElevate()
    {
        WriteEvent("OnElevate()");
        return IDNOACTION;
    }

    virtual STDMETHODIMP_(int) OnRegisterBegin()
    {
        WriteEvent("OnRegisterBegin()");
        return IDOK;
    }


    virtual STDMETHODIMP_(void) OnRegisterComplete(
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnRegisterComplete() - hrStatus: 0x%x", hrStatus);
        return;
    }


    virtual STDMETHODIMP_(void) OnUnregisterBegin()
    {
        WriteEvent("OnUnregisterBegin()");
        return;
    }


    virtual STDMETHODIMP_(void) OnUnregisterComplete(
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnUnregisterComplete() - hrStatus: 0x%x", hrStatus);
        return;
    }


    virtual STDMETHODIMP_(int) OnApplyComplete(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        WriteEvent("OnApplyComplete() - hrStatus: 0x%x, restart: %d", hrStatus, restart);
        SetState(STDUX_STATE_APPLIED);

        BOOL fRestart = FALSE;

        if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
            {
                int nResult = ::MessageBoxW(m_hWnd, L"One or more packages required a restart. Restart now?", L"Restart Required", MB_YESNO | MB_ICONQUESTION);
                fRestart = (IDYES == nResult);
            }
        }
        else // embedded or quiet or passive display just close at the end, no questions asked.
        {
            fRestart = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart);
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }

        // Save this to enable us to detect failed apply phase
        m_hrFinal = hrStatus;

        return fRestart ? IDRESTART : IDOK;
    }


    virtual STDMETHODIMP_(int) OnCacheBegin()
    {
        WriteEvent("OnCacheBegin()"); //  - cPackages: %u, cTotalPayloads: %u, dw64TotalCacheSize: %I64u", cPackages, cTotalPayloads, dw64TotalCacheSize);
        return IDOK;
    }

    virtual STDMETHODIMP_(int) OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD cCachePayloads,
        __in DWORD64 dw64PackageCacheSize
        )
    {
        WriteEvent("OnCachePackageBegin() - wzPackageId: %ls, cCachePayloads: %u, dw64PackageCacheSize: %I64u", wzPackageId, cCachePayloads, dw64PackageCacheSize);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnCacheAcquireBegin(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in BOOTSTRAPPER_CACHE_OPERATION operation,
        __in_z LPCWSTR wzSource
        )
    {
        WriteEvent("OnCacheAcquireBegin() - wzPackageOrContainerId: %ls, wzPayloadId: %ls, operation: %u, wzSource: %ls", wzPackageOrContainerId, wzPayloadId, operation, wzSource);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnCacheAcquireProgress(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in DWORD64 dw64Progress,
        __in DWORD64 dw64Total,
        __in DWORD dwOverallPercentage
        )
    {
        WriteEvent("OnCacheAcquireProgress() - wzPackageOrContainerId: %ls, wzPayloadId: %ls, dw64Progress: %I64u, dw64Total: %I64u, dwOverallPercentage: %u", wzPackageOrContainerId, wzPayloadId, dw64Progress, dw64Total, dwOverallPercentage);
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };

        m_nModalResult = IDOK;

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallPercentage);

        hr = ThemeSetTextControl(m_pTheme, STDUX_CONTROL_CACHE_PROGRESS_TEXT, wzProgress);
        ExitOnFailure(hr, "Failed to set progress.");

    LExit:
        if (FAILED(hr) && IDNOACTION == m_nModalResult)
        {
            m_nModalResult = IDERROR;
        }

        return m_nModalResult;
    }


    virtual STDMETHODIMP_(int) OnCacheAcquireComplete(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnCacheAcquireComplete() - wzPackageOrContainerId: %ls, wzPayloadId: %ls, hrStatus: 0x%x", wzPackageOrContainerId, wzPayloadId, hrStatus);
        return IDOK;
    }


    virtual STDMETHODIMP_(int) OnCacheVerifyBegin(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzPayloadId
        )
    {
        WriteEvent("OnCacheVerifyBegin() - wzPackageId: %ls, wzPayloadId: %ls", wzPackageId, wzPayloadId);
        return IDNOACTION;
    }


    virtual STDMETHODIMP_(int) OnCacheVerifyComplete(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzPayloadId,
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnCacheVerifyComplete() - wzPackageId: %ls, wzPayloadId: %ls, hrStatus: 0x%x", wzPackageId, wzPayloadId, hrStatus);
        return IDNOACTION;
    }


    virtual void __stdcall OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnCachePackageComplete() - wzPackageId: %ls, hrStatus: 0x%x", wzPackageId, hrStatus);
    }


    virtual STDMETHODIMP_(void) OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
        WriteEvent("OnCacheComplete() - hrStatus: 0x%x", hrStatus);
        if (SUCCEEDED(hrStatus))
        {
            SetState(STDUX_STATE_CACHED);
        }
    }


    virtual STDMETHODIMP_(BOOL) OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        WriteEvent("OnExecuteBegin() - cExecutingPackages: %u", cExecutingPackages);
        return TRUE;
    }


    virtual STDMETHODIMP_(BOOL) OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        )
    {
        WriteEvent("OnExecutePackageBegin() - wzPackageId: %ls, fExecute: %d", wzPackageId, fExecute);
        return TRUE; // always execute.
    }


    virtual STDMETHODIMP_(int) OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        WriteEvent("OnError() - wzPackageId: %ls, dwCode: %u, wzError: %ls, dwUIHint: %u", wzPackageId, dwCode, wzError, dwUIHint);

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
             HRESULT hr = m_pCore->SendEmbeddedError(dwCode, wzError, dwUIHint, &m_nModalResult);
             if (FAILED(hr))
             {
                 m_nModalResult = IDERROR;
             }
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            ModalPrompt(STDUX_STATE_ERROR, dwUIHint, wzError);
        }
        else // just cancel when quiet or passive.
        {
            m_nModalResult = IDABORT;
        }

        return m_nModalResult;
    }


    virtual STDMETHODIMP_(int) OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        WriteEvent("OnProgress() - dwProgressPercentage: %u, dwOverallProgressPercentage: %u", dwProgressPercentage, dwOverallProgressPercentage);
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };

        m_nModalResult = IDOK;

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
            hr = m_pCore->SendEmbeddedProgress(dwProgressPercentage, dwOverallProgressPercentage, &m_nModalResult);
        }
        else
        {
            ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);

            //hr = ThemeSetTextControl(m_pTheme, STDUX_CONTROL_OVERALL_PROGRESS_TEXT, wzProgress);
            //ExitOnFailure(hr, "Failed to set progress.");
        }

    //LExit:
        if (FAILED(hr) && IDNOACTION == m_nModalResult)
        {
            m_nModalResult = IDERROR;
        }

        return m_nModalResult;
    }

    virtual int __stdcall  OnExecuteProgress(
        __in_z LPCWSTR wzPackageId,
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        WriteEvent("OnExecuteProgress() - wzPackageId: %ls, dwProgressPercentage: %u, dwOverallProgressPercentage: %u", wzPackageId, dwProgressPercentage, dwOverallProgressPercentage);
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };

        m_nModalResult = IDOK;

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);

        hr = ThemeSetTextControl(m_pTheme, STDUX_CONTROL_EXECUTE_PROGRESS_TEXT, wzProgress);
        ExitOnFailure(hr, "Failed to set progress.");

    LExit:
        if (FAILED(hr) && IDNOACTION == m_nModalResult)
        {
            m_nModalResult = IDERROR;
        }

        return m_nModalResult;
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
        __in HRESULT hrExitCode,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        WriteEvent("OnExecutePackageComplete() - wzPackageId: %ls, hrExitCode: 0x%x", wzPackageId, hrExitCode);

        HRESULT hr = S_OK;
        LPWSTR sczRestartPackage = NULL;
        BOOL fRestart = FALSE;

        if (BOOTSTRAPPER_DISPLAY_NONE == m_command.display || BOOTSTRAPPER_DISPLAY_PASSIVE == m_command.display)
        {
            fRestart = (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
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
        WriteEvent("OnExecuteComplete() - hrStatus: 0x%x", hrStatus);
        if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            SetState(STDUX_STATE_EXECUTED);
        }
    }


    virtual int __stdcall OnResolveSource(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in_z LPCWSTR wzLocalSource,
        __in_z_opt LPCWSTR wzDownloadSource
        )
    {
        WriteEvent("OnResolveSource() - wzPackageId: %ls, wzPayloadOrContainerId: %ls, wzLocalSource: %ls, wzDownloadSource: %ls", wzPackageOrContainerId, wzPayloadId, wzLocalSource, wzDownloadSource);
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
        pThis->m_pCore->Quit(pThis->m_hrFinal);

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
        if (BOOTSTRAPPER_DISPLAY_NONE == m_command.display || BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
            dwWindowStyle &= ~WS_VISIBLE;
        }

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->sczCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
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

        //::WaitForSingleObject(m_hModalWait, INFINITE);

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
            pUX->OnPlan(static_cast<BOOTSTRAPPER_ACTION>(lParam));
            return 0;

        case WM_STDUX_APPLY_PACKAGES:
            pUX->OnApply();
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            //case STDUX_CONTROL_TOU_LINK:
            //    pUX->OnClickTermsOfUse();
            //    break;

            //case STDUX_CONTROL_INSTALL_BUTTON:
            //    pUX->OnClickInstallButton();
            //    break;

            //case STDUX_CONTROL_REPAIR_BUTTON:
            //    pUX->OnClickRepairButton();
            //    break;

            //case STDUX_CONTROL_UNINSTALL_BUTTON:
            //    pUX->OnClickUninstallButton();
            //    break;

            //case STDUX_CONTROL_CLOSE_BUTTON:
            //    pUX->OnClickCloseButton();
            //    break;

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

        hr = ThemeLoadControls(m_pTheme, hWnd, NULL, 0);
        ExitOnFailure(hr, "Failed to load theme controls.");

        // If the splash screen is around, close it since we're showing our UI now.
        if (::IsWindow(m_command.hwndSplashScreen))
        {
             ::PostMessageW(m_command.hwndSplashScreen, WM_CLOSE, 0, 0);
        }

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
        __in BOOTSTRAPPER_ACTION action
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

            //int nShowTermsOfUse = (m_state < STDUX_STATE_EXECUTING) ? SW_SHOW : SW_HIDE;
            //int nShowInstall = (BOOTSTRAPPER_DISPLAY_FULL == m_command.display && BOOTSTRAPPER_ACTION_INSTALL == m_command.action && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            //int nShowRepair = (FALSE && BOOTSTRAPPER_DISPLAY_FULL == m_command.display && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            //int nShowUninstall = (BOOTSTRAPPER_DISPLAY_FULL == m_command.display && BOOTSTRAPPER_ACTION_UNINSTALL == m_command.action && m_state <= STDUX_STATE_DETECTED) ? SW_SHOW : SW_HIDE;
            //int nShowClose = (BOOTSTRAPPER_DISPLAY_FULL == m_command.display && STDUX_STATE_APPLIED == m_state) ? SW_SHOW : SW_HIDE;

            //int nShowMessage = ((STDUX_STATE_DETECTED < m_state && m_state <= STDUX_STATE_APPLIED) || STDUX_STATE_ERROR == m_state) ? SW_SHOW : SW_HIDE;
            //int nShowProgressBar = (STDUX_STATE_DETECTED < m_state && m_state <= STDUX_STATE_APPLIED) ? SW_SHOW : SW_HIDE;

            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_TOU_LINK].hWnd, nShowTermsOfUse);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_INSTALL_BUTTON].hWnd, nShowInstall);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_REPAIR_BUTTON].hWnd, nShowRepair);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_UNINSTALL_BUTTON].hWnd, nShowUninstall);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_CLOSE_BUTTON].hWnd, nShowClose);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_MESSAGE_TEXT].hWnd, nShowMessage);
            //::ShowWindow(m_pTheme->rgControls[STDUX_CONTROL_PROGRESS_BAR].hWnd, nShowProgressBar);
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


    void WriteEvent(
        __in_z __format_string LPCSTR szFormat,
        ...
        )
    {
        va_list args;
        va_start(args, szFormat);
        LogStringLineArgs(REPORT_STANDARD, szFormat, args);
        va_end(args);
    }


public:
    //
    // Constructor - intitialize member variables.
    //
    CStandardUserExperience(
        __in HMODULE hModule,
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND* pCommand
        )
    {
        m_hUiThread = NULL;

        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        m_cReferences = 1;
        m_pTheme = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;
        m_hwndHover = NULL;

        m_fCanceled = FALSE;
        m_state = STDUX_STATE_INITIALIZING;
        m_nModalResult = IDNOACTION;
        m_hModalWait = NULL;

        m_fAllowRestart = FALSE;
        m_sczLanguage = NULL;
        m_hrFinal = S_OK;

        m_sczFamilyBundleId = NULL;

        pEngine->AddRef();
        m_pCore = pEngine;
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

        if (m_hModalWait)
        {
            ::CloseHandle(m_hModalWait);
            m_hModalWait = NULL;
        }
        ReleaseStr(m_sczFamilyBundleId);
        ReleaseNullObject(m_pCore);
    }

private:
    long m_cReferences;

    HANDLE m_hUiThread;

    HMODULE m_hModule;
    BOOTSTRAPPER_COMMAND m_command;

    IBootstrapperEngine* m_pCore;
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
    CStandardUserExperience* pApplication = NULL;

    pApplication = new CStandardUserExperience(hModule, pEngine, pCommand);
    ExitOnNull(pApplication, hr, E_OUTOFMEMORY, "Failed to create new standard bootstrapper application object.");

    *ppApplication = pApplication;
    pApplication = NULL;

LExit:
    ReleaseObject(pApplication);
    return hr;
}
