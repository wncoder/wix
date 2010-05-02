//-------------------------------------------------------------------------------------------------
// <copyright file="BurnView.h" company="Microsoft">
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
// 
// <summary>
//    CBurnView adapts Burn core/engine functionality to UX.
//    It implements IProgressObsever that CBurnController uses to communicate to UX.
//    Will be adding functionality to this classw with increased support of IBurnUserExperience methods.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "IBurnUserExperience.h"
#include "Interfaces\IDataProviders.h"
#include "Interfaces\IController.h"
#include "Interfaces\IProgressObserver.h"

namespace IronMan
{
class CBurnView : public IBurnView
{
    IBurnUserExperience& m_burnUX;
    ILogger& m_logger;
    BOOL m_fExecutingRollback;
    BOOL m_fUxInitialized;
    BOOL m_fPlanReady;
    BOOL m_fDetectComplete;
    BOOL m_fCancelled;
    BOOL m_fRebootRequired;
    BOOL m_fRebootInitiated;
    BOOL m_fElevate;

    DWORD m_cExecutingPackages;
    DWORD m_cExecutedPackages;

    BURN_ACTION m_action;

public:
    virtual ~CBurnView() 
    {
    }

    CBurnView(IBurnUserExperience& burnUX, ILogger& logger)
        : m_burnUX(burnUX)
        , m_logger(logger)
        , m_fExecutingRollback(FALSE)
        , m_fUxInitialized(FALSE)
        , m_fPlanReady(FALSE)
        , m_fDetectComplete(FALSE)
        , m_fCancelled(FALSE)
        , m_fRebootRequired(FALSE)
        , m_fRebootInitiated(FALSE)
        , m_fElevate(FALSE)
        , m_cExecutingPackages(0)
        , m_cExecutedPackages(0)
        , m_action(BURN_ACTION_UNKNOWN)
    {
    }

    // IBurnView
    virtual HRESULT __stdcall Initialize(
        __in IBurnCore* pCore, 
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType
        )
    {
        HRESULT hr = m_burnUX.Initialize(pCore, 0, resumeType);
        m_fUxInitialized = SUCCEEDED(hr);
        return hr;
    }

    virtual int __stdcall OnProgress(unsigned char detail, unsigned char overall)
    {
        int nResult = m_burnUX.OnProgress(detail, overall);
        UpdateViewState(nResult);
        return nResult;
    }

    virtual int __stdcall OnDownloadPayloadBegin(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName
        )
    {
        return m_burnUX.OnDownloadPayloadBegin(wzPayloadId, wzPayloadFileName);
    }

    virtual void __stdcall OnDownloadPayloadComplete(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName,
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnDownloadPayloadComplete(wzPayloadId, wzPayloadFileName, hrStatus);
    }

    virtual int __stdcall OnDownloadProgress(unsigned char detail, unsigned char overall)
    {
        return m_burnUX.OnDownloadProgress(detail, overall);
    }

    virtual int __stdcall OnExecuteProgress(unsigned char detail, unsigned char overall)
    {
        return m_burnUX.OnExecuteProgress(detail, overall);
    }

    virtual BOOL __stdcall OnRestartRequired()
    {
        return m_burnUX.OnRestartRequired();
    }

    virtual void __stdcall Uninitialize()
    {
        m_burnUX.Uninitialize();
        m_fUxInitialized = FALSE;
    }

    virtual int __stdcall OnDetectBegin(
        __in DWORD cPackages
        )
    {
        // Reset m_fDetectComplete and m_fPlanReady, in case Detect is called again after Plan or Apply fails
        m_fDetectComplete = FALSE;
        m_fPlanReady = FALSE;

        int nResult = m_burnUX.OnDetectBegin(cPackages);
        UpdateViewState(nResult);
        return nResult;

    }

    virtual int __stdcall OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        )
    {
        return m_burnUX.OnDetectPriorBundle(wzBundleId);
    }

    virtual int __stdcall OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )
    {
        int nResult = m_burnUX.OnDetectPackageBegin(wzPackageId);
        UpdateViewState(nResult);
        return nResult;

    }

    virtual void __stdcall OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state
        )
    {
        m_burnUX.OnDetectPackageComplete(wzPackageId, hrStatus, state);
    }

    virtual void __stdcall OnDetectComplete(
        __in HRESULT hrStatus
    )
    {
        m_burnUX.OnDetectComplete(hrStatus);
        // if Detection did not succeed need to prevent Plan or Apply being run
        m_fDetectComplete = SUCCEEDED(hrStatus);
    }

    virtual int __stdcall OnPlanBegin(
        __in DWORD cPackages
        )
    {
        m_fPlanReady = FALSE;
        return m_burnUX.OnPlanBegin(cPackages);
    }

    virtual int __stdcall OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        return m_burnUX.OnPlanPriorBundle(wzBundleId, pRequestedState);
    }

    virtual int __stdcall OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        int nResult = m_burnUX.OnPlanPackageBegin(wzPackageId, pRequestedState);
        UpdateViewState(nResult);
        return nResult;

    }

    virtual void __stdcall OnPlanPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state,
        __in REQUEST_STATE requested,
        __in ACTION_STATE execute,
        __in ACTION_STATE rollback,
        __in BOOL fPerMachine
        )
    {
        if (fPerMachine)
        {
            m_fElevate = TRUE;
        }

        return m_burnUX.OnPlanPackageComplete(wzPackageId, hrStatus, state, requested, execute, rollback);
    }

    virtual void  __stdcall OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnPlanComplete(hrStatus);
        // if Plan did not succeed need to prevent Apply being run
        m_fPlanReady = SUCCEEDED(hrStatus);
    }

    virtual int __stdcall OnApplyBegin()
    {
        int nResult = m_burnUX.OnApplyBegin();
        UpdateViewState(nResult);
        return nResult;

    }

    virtual int __stdcall OnRegisterBegin()
    {
        int nResult = m_burnUX.OnRegisterBegin();
        UpdateViewState(nResult);
        return nResult;
    }

    virtual void __stdcall OnRegisterComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnRegisterComplete(hrStatus);
    }

    virtual int __stdcall OnUnregisterBegin()
    {
        m_burnUX.OnUnregisterBegin();
        return IDOK; // TODO: OnUnregisterBegin() can't be canceled so make this void.
    }

    virtual void __stdcall OnUnregisterComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnUnregisterComplete(hrStatus);
    }

    virtual int __stdcall OnCacheBegin()
    {
        int nResult = m_burnUX.OnCacheBegin();
        UpdateViewState(nResult);
        return nResult;
    }

    virtual void __stdcall OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnCacheComplete(hrStatus);
    }

    virtual int __stdcall OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD64 dw64PackageCacheSize
        )
    {
        return m_burnUX.OnCachePackageBegin(wzPackageId, dw64PackageCacheSize);
    }

    virtual void __stdcall OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnCachePackageComplete(wzPackageId, hrStatus);
    }

    virtual int __stdcall OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        m_cExecutingPackages = cExecutingPackages;
        int nResult = m_burnUX.OnExecuteBegin(cExecutingPackages);
        UpdateViewState(nResult);
        return nResult;
    }

    virtual int __stdcall  OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        ) 
    {
        int nResult = m_burnUX.OnExecutePackageBegin(wzPackageId, fExecute);
        UpdateViewState(nResult);
        return nResult;
    }

    virtual int __stdcall OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        int nResult = IDOK;

        if (wzError)
        {
            nResult = m_burnUX.OnError(wzPackageId, dwCode, wzError, dwUIHint);
        }
        else
        {
            LPWSTR lpszErrorMessage = NULL;

            DWORD cchErrorMessage = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpszErrorMessage), 0, NULL);
            if (!cchErrorMessage)
            {
                lpszErrorMessage = NULL;
            }

            nResult = m_burnUX.OnError(wzPackageId, dwCode, lpszErrorMessage, dwUIHint);

            if (lpszErrorMessage)
            {
                ::LocalFree(lpszErrorMessage);
            }
        }

        return nResult;
    }

    virtual int __stdcall OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )
    {
        return m_burnUX.OnExecuteMsiMessage(wzPackageId, mt, uiFlags, wzMessage);
    }

    virtual int __stdcall OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
    {
        return m_burnUX.OnExecuteMsiFilesInUse(wzPackageId, cFiles, rgwzFiles);
    }

    virtual void __stdcall OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrExitCode
        )
    {
        if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hrExitCode)
        {
            m_fRebootRequired = TRUE;
        }

        if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hrExitCode)
        {
            m_fRebootInitiated = TRUE;
        }

        if (SUCCEEDED(hrExitCode) || m_fExecutingRollback)
        {
            m_cExecutedPackages += m_fExecutingRollback ? - 1 : 1;
        }
        else
        {
            m_fExecutingRollback = TRUE;
        }

        return m_burnUX.OnExecutePackageComplete(wzPackageId, hrExitCode);
    }

    virtual void __stdcall OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnExecuteComplete(hrStatus);
    }

    virtual void __stdcall OnApplyComplete(
        __in HRESULT hrStatus
        )
    {
        m_burnUX.OnApplyComplete(hrStatus);
    }

    // BurnController methods

    virtual HRESULT GetViewState(BOOL* pfUserCancelled, BOOL* pfRebootRequired, BOOL* pfRebootInitiated) const
    {
        if (pfUserCancelled)
        {
            *pfUserCancelled = m_fCancelled;
        }

        if (pfRebootRequired)
        {
            *pfRebootRequired = m_fRebootRequired;
        }

        if (pfRebootInitiated)
        {
            *pfRebootInitiated = m_fRebootInitiated;
        }

        return S_OK;
    }

    virtual BOOL IsUxInitialized() const
    {
        return m_fUxInitialized;
    }

    virtual BOOL IsDetectComplete() const
    {
        return m_fDetectComplete;
    }

    virtual BOOL IsPlanReady() const
    {
        return m_fPlanReady;
    }

    virtual BOOL IsElevationRequired() const
    {
        return m_fElevate;
    }

    virtual void SetCurrentAction(BURN_ACTION action)
    {
        m_action = action;
    }

    virtual BURN_ACTION GetCurrentAction()
    {
        return m_action;
    }

private:
    void UpdateViewState(int nResult)
    {
        if (IDCANCEL == nResult)
        {
            m_fCancelled = TRUE;
        }
    }

    virtual int __stdcall ResolveSource (
        __in    LPCWSTR wzPackageID ,
        __in    LPCWSTR wzPackageOrContainePath
        )
    {
        return m_burnUX.ResolveSource(wzPackageID, wzPackageOrContainePath);
    }

    virtual BOOL __stdcall CanPackagesBeDownloaded(void)
    {
        return m_burnUX.CanPackagesBeDownloaded();
    }
};

class CNullBurnView : public IBurnView
{
public:
    ~CNullBurnView() 
    {
    }

    CNullBurnView()
    {
    }

    virtual HRESULT __stdcall Initialize(
        __in IBurnCore* pCore, 
        __in int nCmdShow,
        __in BURN_RESUME_TYPE suspendRequired
        )

    {
        return S_OK;
    }

    virtual int __stdcall OnProgress(unsigned char detail, unsigned char overall)
    {
        return IDOK;
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
    }

    virtual int __stdcall OnDownloadProgress(unsigned char detail, unsigned char overall)
    {
        return IDOK;
    }

    virtual int __stdcall OnExecuteProgress(unsigned char detail, unsigned char overall)
    {
        return IDOK;
    }

    virtual void __stdcall Uninitialize()
    {
    }

    virtual HRESULT __stdcall Run()
    {
        return S_OK;
    }

    virtual int __stdcall OnDetectBegin(
        __in DWORD cPackages
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )
    {
        return IDOK;
    }

    virtual void __stdcall OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state
        )
    {
    }

    virtual void __stdcall OnDetectComplete(
        __in HRESULT hrStatus
    )
    {
    }

    virtual int __stdcall OnPlanBegin(
        __in DWORD cPackages
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout_z REQUEST_STATE* pRequestedState
        )
    {
        return IDOK;
    }

    virtual void __stdcall OnPlanPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state,
        __in REQUEST_STATE requested,
        __in ACTION_STATE execute,
        __in ACTION_STATE rollback,
        __in BOOL fPerMachine
        )
    {
    }

    virtual void __stdcall OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
    }

    virtual int __stdcall OnApplyBegin()
    {
        return IDOK;
    }

    virtual int __stdcall OnRegisterBegin()
    {
        return IDOK;
    }

    virtual void __stdcall OnRegisterComplete(
        __in HRESULT hrStatus
        )
    {
    }

    virtual int __stdcall OnUnregisterBegin()
    {
        return IDOK;
    }

    virtual void __stdcall OnUnregisterComplete(
        __in HRESULT hrStatus
        )
    {
    }

    virtual int __stdcall OnCacheBegin()
    {
        return IDOK;
    }

    virtual void __stdcall OnCacheComplete(
        __in HRESULT hrStatus
        )
    {
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

    virtual int __stdcall OnExecuteBegin(
        __in DWORD cExecutingPackages
        )
    {
        return IDOK;
    }

    virtual int __stdcall  OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        ) 
    {
        return IDOK;
    }

    virtual int __stdcall OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
    {
        return IDOK;
    }

    virtual void __stdcall OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrExitCode
        )
    {
    }

    virtual void __stdcall OnExecuteComplete(
        __in HRESULT hrStatus
        )
    {
    }

    virtual int __stdcall OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        return IDOK;
    }

    virtual int __stdcall OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallPercentage
        )
    {
        return IDOK;
    }

    virtual BOOL __stdcall OnRestartRequired()
    {
        return FALSE;
    }

    virtual void __stdcall OnApplyComplete(
        __in HRESULT hrStatus
        )
    {
    }

    virtual HRESULT GetViewState(BOOL* pfUserCancelled, BOOL* pfRebootRequired, BOOL* pfRebootInitiated) const
    {
        return S_OK;
    }

    virtual BOOL IsElevationRequired() const
    {
        return FALSE;
    }

    virtual int __stdcall ResolveSource (
        __in    LPCWSTR wzPackageID ,
        __in    LPCWSTR wzPackageOrContainePath
        )
    {
        return IDCANCEL;
    }

    virtual void SetCurrentAction(BURN_ACTION action)
    {
    }

    virtual BURN_ACTION GetCurrentAction()
    {
        return BURN_ACTION_INSTALL;
    }

    virtual BOOL __stdcall CanPackagesBeDownloaded(void)
    {
        return TRUE;
    }

};
}