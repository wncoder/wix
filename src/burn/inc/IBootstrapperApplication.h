//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperApplication.h" company="Microsoft">
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
// 
// <summary>
// IBootstrapperApplication implemented by a bootstrapper application and used by bootstrapper engine.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


enum BOOTSTRAPPER_DISPLAY
{
    BOOTSTRAPPER_DISPLAY_UNKNOWN,
    BOOTSTRAPPER_DISPLAY_EMBEDDED,
    BOOTSTRAPPER_DISPLAY_NONE,
    BOOTSTRAPPER_DISPLAY_PASSIVE,
    BOOTSTRAPPER_DISPLAY_FULL,
};


enum BOOTSTRAPPER_RESTART
{
    BOOTSTRAPPER_RESTART_UNKNOWN,
    BOOTSTRAPPER_RESTART_NEVER,
    BOOTSTRAPPER_RESTART_PROMPT,
    BOOTSTRAPPER_RESTART_AUTOMATIC,
    BOOTSTRAPPER_RESTART_ALWAYS,
};


enum BOOTSTRAPPER_RESUME_TYPE
{
    BOOTSTRAPPER_RESUME_TYPE_NONE,
    BOOTSTRAPPER_RESUME_TYPE_INVALID,        // resume information is present but invalid
    BOOTSTRAPPER_RESUME_TYPE_UNEXPECTED,     // relaunched after an unexpected interruption
    BOOTSTRAPPER_RESUME_TYPE_REBOOT_PENDING, // reboot has not taken place yet
    BOOTSTRAPPER_RESUME_TYPE_REBOOT,         // relaunched after reboot
    BOOTSTRAPPER_RESUME_TYPE_SUSPEND,        // relaunched after suspend
    BOOTSTRAPPER_RESUME_TYPE_ARP,            // launched from ARP
};


enum BOOTSTRAPPER_RELATED_OPERATION
{
    BOOTSTRAPPER_RELATED_OPERATION_NONE,
    BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE,
    BOOTSTRAPPER_RELATED_OPERATION_MINOR_UPDATE,
    BOOTSTRAPPER_RELATED_OPERATION_MAJOR_UPGRADE,
    BOOTSTRAPPER_RELATED_OPERATION_REMOVE,
    BOOTSTRAPPER_RELATED_OPERATION_REPAIR,
};


enum BOOTSTRAPPER_CACHE_OPERATION
{
    BOOTSTRAPPER_CACHE_OPERATION_COPY,
    BOOTSTRAPPER_CACHE_OPERATION_DOWNLOAD,
    BOOTSTRAPPER_CACHE_OPERATION_EXTRACT,
};


enum BOOTSTRAPPER_APPLY_RESTART
{
    BOOTSTRAPPER_APPLY_RESTART_NONE,
    BOOTSTRAPPER_APPLY_RESTART_REQUIRED,
    BOOTSTRAPPER_APPLY_RESTART_INITIATED,
};


struct BOOTSTRAPPER_COMMAND
{
    BOOTSTRAPPER_ACTION action;
    BOOTSTRAPPER_DISPLAY display;
    BOOTSTRAPPER_RESTART restart;

    LPWSTR wzCommandLine;
    int nCmdShow;

    BOOTSTRAPPER_RESUME_TYPE resumeType;
    HWND hwndSplashScreen;
};


DECLARE_INTERFACE_IID_(IBootstrapperApplication, IUnknown, "53C31D56-49C0-426B-AB06-099D717C67FE")
{
    STDMETHOD(OnStartup)() = 0;

    // OnShutdown - called after the application quits the engine.
    //
    // Return:
    //  IDRESTART instructs the engine to restart. The engine will not launch again after the machine
    //            is rebooted. Ignored if reboot was already initiated by OnExecutePackageComplete().
    //  All other return codes are ignored.
    STDMETHOD_(int, OnShutdown)() = 0;

    STDMETHOD_(int, OnDetectBegin)(
        __in DWORD cPackages
        ) = 0;

    STDMETHOD_(int, OnDetectRelatedBundle)(
        __in_z LPCWSTR wzBundleId,
        __in_z LPCWSTR wzBundleTag,
        __in BOOL fPerMachine,
        __in DWORD64 dw64Version,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        ) = 0;

    STDMETHOD_(int, OnDetectPackageBegin)(
        __in_z LPCWSTR wzPackageId
        ) = 0;

    STDMETHOD_(int, OnDetectRelatedMsiPackage)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzProductCode,
        __in BOOL fPerMachine,
        __in DWORD64 dw64Version,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        ) = 0;

    STDMETHOD_(int, OnDetectTargetMsiPackage)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzProductCode,
        __in BOOTSTRAPPER_PACKAGE_STATE patchState
        ) = 0;

    STDMETHOD_(int, OnDetectMsiFeature)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzFeatureId,
        __in BOOTSTRAPPER_FEATURE_STATE state
        ) = 0;

    STDMETHOD_(void, OnDetectPackageComplete)(
        __in_z LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state
        ) = 0;

    STDMETHOD_(void, OnDetectComplete)(
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(int, OnPlanBegin)(
        __in DWORD cPackages
        ) = 0;

    STDMETHOD_(int, OnPlanRelatedBundle)(
        __in_z LPCWSTR wzBundleId,
        __inout BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        ) = 0;

    STDMETHOD_(int, OnPlanPackageBegin)(
        __in_z LPCWSTR wzPackageId,
        __inout BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        ) = 0;

    STDMETHOD_(int, OnPlanTargetMsiPackage)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzProductCode,
        __inout BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        ) = 0;

    STDMETHOD_(int, OnPlanMsiFeature)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzFeatureId,
        __inout BOOTSTRAPPER_FEATURE_STATE* pRequestedState
        ) = 0;

    STDMETHOD_(void, OnPlanPackageComplete)(
        __in_z LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_PACKAGE_STATE state,
        __in BOOTSTRAPPER_REQUEST_STATE requested,
        __in BOOTSTRAPPER_ACTION_STATE execute,
        __in BOOTSTRAPPER_ACTION_STATE rollback
        ) = 0;

    STDMETHOD_(void, OnPlanComplete)(
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(int, OnApplyBegin)() = 0;

    STDMETHOD_(int, OnElevate)() = 0;

    STDMETHOD_(int, OnProgress)(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallPercentage
        ) = 0;

    STDMETHOD_(int, OnError)(
        __in_z_opt LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        ) = 0;

    STDMETHOD_(int, OnRegisterBegin)() = 0;

    STDMETHOD_(void, OnRegisterComplete)(
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(int, OnCacheBegin)() = 0;

    STDMETHOD_(int, OnCachePackageBegin)(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cCachePayloads,
        __in DWORD64 dw64PackageCacheSize
        )  = 0;

    STDMETHOD_(int, OnCacheAcquireBegin)(
        __in_z_opt LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in BOOTSTRAPPER_CACHE_OPERATION operation,
        __in_z LPCWSTR wzSource
        ) = 0;

    STDMETHOD_(int, OnCacheAcquireProgress)(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in DWORD64 dw64Progress,
        __in DWORD64 dw64Total,
        __in DWORD dwOverallPercentage
        ) = 0;

    // OnResolveSource - called when a payload or container cannot be found locally.
    //
    // Parameters:
    //  wzPayloadId will be NULL when resolving a container.
    //  wzDownloadSource will be NULL if the container or payload does not provide a DownloadURL.
    //
    // Return:
    //  IDRETRY instructs the engine to try the local source again.
    //  IDDOWNLOAD instructs the engine to try the download source.
    //  All other return codes result in an error.
    //
    // Notes:
    //  It is expected the UX may call IBurnCore::SetLocalSource() or IBurnCore::SetDownloadSource()
    //  to update the source location before returning IDRETRY or IDDOWNLOAD.
    STDMETHOD_(int, OnResolveSource)(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in_z LPCWSTR wzLocalSource,
        __in_z_opt LPCWSTR wzDownloadSource
        ) = 0;

    STDMETHOD_(int, OnCacheAcquireComplete)(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(int, OnCacheVerifyBegin)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzPayloadId
        ) = 0;

    STDMETHOD_(int, OnCacheVerifyComplete)(
        __in_z LPCWSTR wzPackageId,
        __in_z LPCWSTR wzPayloadId,
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(void, OnCachePackageComplete)(
        __in_z LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        )  = 0;

    STDMETHOD_(void, OnCacheComplete)(
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(int, OnExecuteBegin)(
        __in DWORD cExecutingPackages
        ) = 0;

    STDMETHOD_(int, OnExecutePackageBegin)(
        __in_z LPCWSTR wzPackageId,
        __in BOOL fExecute
        ) = 0;

    STDMETHOD_(int, OnExecuteProgress)(
        __in_z LPCWSTR wzPackageId,
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallPercentage
        ) = 0;

    STDMETHOD_(int, OnExecuteMsiMessage)(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        ) = 0;

    STDMETHOD_(int, OnExecuteFilesInUse)(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in_ecount_z(cFiles) LPCWSTR* rgwzFiles
        ) = 0;

    // OnExecutePackageComplete - called when a package execution is complete.
    //
    // Parameters:
    //  restart will indicate whether this package requires a reboot or initiated the reboot already.
    //
    // Return:
    //  IDRETRY instructs the engine to try the execution of the package again. Ignored if hrStatus
    //          is a success.
    //  IDRESTART instructs the engine to stop processing the chain and restart. The engine will
    //            launch again after the machine is rebooted.
    //  IDSUSPEND instructs the engine to stop processing the chain and suspend the current state.
    //  All other return codes are ignored.
    STDMETHOD_(int, OnExecutePackageComplete)(
        __in_z LPCWSTR wzPackageId,
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        ) = 0;

    STDMETHOD_(void, OnExecuteComplete)(
        __in HRESULT hrStatus
        ) = 0;

    STDMETHOD_(void, OnUnregisterBegin)() = 0;

    STDMETHOD_(void, OnUnregisterComplete)(
        __in HRESULT hrStatus
        ) = 0;

    // OnApplyComplete - called after the plan has been applied.
    //
    // Parameters:
    //  restart will indicate whether any package required a reboot or initiated the reboot already.
    //
    // Return:
    //  IDRESTART instructs the engine to restart. The engine will not launch again after the machine
    //            is rebooted. Ignored if reboot was already initiated by OnExecutePackageComplete().
    //  All other return codes are ignored.
    STDMETHOD_(int, OnApplyComplete)(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        ) = 0;
};


extern "C" typedef HRESULT (WINAPI *PFN_BOOTSTRAPPER_APPLICATION_CREATE)(
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication** ppApplication
    );
extern "C" typedef void (WINAPI *PFN_BOOTSTRAPPER_APPLICATION_DESTROY)();
