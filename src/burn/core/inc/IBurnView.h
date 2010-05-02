//-------------------------------------------------------------------------------------------------
// <copyright file="IBurnView.h" company="Microsoft">
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
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#define HRESULT_FROM_VIEW(n) IDOK == n || IDNOACTION == n ? S_OK : IDCANCEL == n || IDABORT == n ? HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT) : HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE)

namespace IronMan
{
struct IBurnView
{
    virtual HRESULT __stdcall Initialize(
        __in IBurnCore* pCore, 
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType) = 0;

    virtual int __stdcall OnProgress(unsigned char detail, unsigned char overall)  = 0;

    virtual int __stdcall OnDownloadPayloadBegin(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName
        ) = 0;

    virtual void __stdcall OnDownloadPayloadComplete(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName,
        __in HRESULT hrStatus
        ) = 0;

    virtual int __stdcall OnDownloadProgress(unsigned char detail, unsigned char overall) = 0;

    virtual int __stdcall OnExecuteProgress(unsigned char detail, unsigned char overall) = 0;

    virtual BOOL __stdcall OnRestartRequired() = 0;

    virtual void __stdcall Uninitialize()  = 0;

    virtual int __stdcall OnDetectBegin(
        __in DWORD cPackages
        )  = 0;

    virtual int __stdcall OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        ) = 0;

    virtual int __stdcall OnDetectPackageBegin(
        __in_z LPCWSTR wzPackageId
        )  = 0;

    virtual void __stdcall OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state
        )  = 0;

    virtual void __stdcall OnDetectComplete(
        __in HRESULT hrStatus
    )  = 0;

    virtual int __stdcall OnPlanBegin(
        __in DWORD cPackages
        )  = 0;

    virtual int __stdcall OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        ) = 0;

    virtual int __stdcall OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout_z REQUEST_STATE* pRequestedState
        )  = 0;

    virtual void __stdcall OnPlanPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in PACKAGE_STATE state,
        __in REQUEST_STATE requested,
        __in ACTION_STATE execute,
        __in ACTION_STATE rollback,
        __in BOOL fPerMachine
        )  = 0;

    virtual void __stdcall OnPlanComplete(
        __in HRESULT hrStatus
        )  = 0;

    virtual int __stdcall OnApplyBegin()  = 0;

    virtual int __stdcall OnRegisterBegin() = 0;

    virtual void __stdcall OnRegisterComplete(
        __in HRESULT hrStatus
        ) = 0;

    virtual int __stdcall OnUnregisterBegin() = 0;

    virtual void __stdcall OnUnregisterComplete(
        __in HRESULT hrStatus
        ) = 0;

    virtual int __stdcall OnCacheBegin() = 0;

    virtual void __stdcall OnCacheComplete(
        __in HRESULT hrStatus) = 0;

    virtual int __stdcall OnExecuteBegin(
        __in DWORD cExecutingPackages
        )  = 0;

    virtual int __stdcall  OnExecutePackageBegin(
        __in LPCWSTR wzPackageId,
        __in BOOL fExecute
        )  = 0;

    virtual int __stdcall OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        ) = 0;

    virtual void __stdcall OnExecutePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrExitCode
        ) = 0;

    virtual int __stdcall OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        ) = 0;

    virtual int __stdcall OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        ) = 0;

    virtual void __stdcall OnExecuteComplete(
        __in HRESULT hrStatus
        )  = 0;

    virtual void __stdcall OnApplyComplete(
        __in HRESULT hrStatus
        )  = 0;

    virtual HRESULT GetViewState(BOOL* pfUserCancelled, BOOL* pfRebootRequired, BOOL* pfRebootForced) const = 0;

    virtual BOOL IsElevationRequired() const = 0;

    virtual int __stdcall ResolveSource (
        __in    LPCWSTR wzPackageID ,
        __in    LPCWSTR wzPackageOrContainePath
        ) = 0;

    virtual BOOL __stdcall CanPackagesBeDownloaded(void) = 0;

    virtual int __stdcall OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD64 dw64PackageCacheSize
        )  = 0;

    virtual void __stdcall OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )  = 0;

    virtual void SetCurrentAction(
        BURN_ACTION action
        ) = 0;

    virtual BURN_ACTION GetCurrentAction() = 0;
};

}
