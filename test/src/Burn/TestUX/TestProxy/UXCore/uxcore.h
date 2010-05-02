#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="uxcore.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Definition of the UXCore
// </summary>
//-------------------------------------------------------------------------------------------------

#include <mscoree.h>

#import <mscorlib.tlb> raw_interfaces_only \
    rename("Assert", "Mscorlib_Assert") \
    rename("ReportEvent", "Mscorlib_ReportEvent")
using namespace mscorlib;

enum MANAGED_UX_FUNCTIONS
{
    MUX_Initialize = 0,   
    MUX_FUNCTION_COUNT // NOTE: This must be last, and enums valued 0..n
};

enum INSTALL_MODE
{
    INSTALL_FULL_DISPLAY = 0,
    INSTALL_MIN_DISPLAY,
    UNINSTALL_FULL_DISPLAY,
    UNINSTALL_MIN_DISPLAY,
	REPAIR_FULL_DISPLAY,
	REPAIR_MIN_DISPLAY,
    MODIFY_FULL_DISPLAY // modify requires full display
};

enum WM_STDUX
{
    WM_STDUX_DETECT_PACKAGES = WM_APP + 1,
    WM_STDUX_PLAN_PACKAGES,
    WM_STDUX_APPLY_PACKAGES,
    WM_STDUX_ELEVATE_PROCESS,
	WM_STDUX_INITIALIZE_MUX,
};

class CChainerUXCore : public IBurnUserExperience
{
private:

    ICorRuntimeHost* m_pClrHost;
    _AppDomain* m_pAppDomain;
    _MethodInfo* m_managedMethods[MUX_FUNCTION_COUNT];

    HRESULT m_initialized;

    HMODULE m_hModule;
    BURN_COMMAND m_command;
	BOOL m_cancelInstall;
	BOOL m_installCanceled;
	INT m_nCmdResult;
    IBurnCore* m_pCore;
	IXMLDOMDocument* m_xmlDoc;
	IXMLDOMNode* m_xmlNode;
	LONG m_cref;

	HWND m_hWnd;

    LPCWSTR m_managedProxyAssembly;
    LPCWSTR m_managedProxyClass;
    LPCWSTR m_managedSetupAssembly;
    LPCWSTR m_managedSetupClass;  

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

    // IBurnUserExperience
    STDMETHODIMP Initialize(
        __in IBurnCore* pCore,
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType
        );

    STDMETHODIMP Run();
    STDMETHODIMP_(void) Uninitialize();
    STDMETHODIMP_(int) OnDetectBegin(__in DWORD cPackages);
    STDMETHODIMP_(int) OnDetectPackageBegin(__in_z LPCWSTR wzPackageId);
    STDMETHODIMP_(void) OnDetectPackageComplete(__in LPCWSTR wzPackageId,
                                                __in HRESULT hrStatus,
                                                __in PACKAGE_STATE state);
    STDMETHODIMP_(void) OnDetectComplete(__in HRESULT hrStatus);

	STDMETHODIMP_(int) OnPlanBegin(__in DWORD cPackages);
	STDMETHODIMP_(int) OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        );
    STDMETHODIMP_(int) OnPlanPackageBegin(__in_z LPCWSTR wzPackageId,
										  __inout_z REQUEST_STATE* pRequestedState);
	 STDMETHODIMP_(int) OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        );
    STDMETHODIMP_(void) OnPlanPackageComplete(__in LPCWSTR wzPackageId,
                                              __in HRESULT hrStatus,
                                              __in PACKAGE_STATE state,
                                              __in REQUEST_STATE requested,
                                              __in ACTION_STATE execute,
                                              __in ACTION_STATE rollback);
    STDMETHODIMP_(void) OnPlanComplete(__in HRESULT hrStatus);

	STDMETHODIMP_(int) OnApplyBegin();
    STDMETHODIMP_(void) OnApplyComplete(__in HRESULT hrStatus);

	STDMETHODIMP_(int) OnProgress(__in DWORD dwProgressPercentage,
                                  __in DWORD dwOverallProgressPercentage);
	STDMETHODIMP_(int) OnCacheBegin();
	STDMETHODIMP_(int) OnCachePackageBegin(
        __in LPCWSTR wzPackageId,
        __in DWORD64 dw64PackageCacheSize
        );
	STDMETHODIMP_(void) OnCachePackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT hrStatus
        );
	STDMETHODIMP_(int) OnDownloadPayloadBegin(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName
        );

    STDMETHODIMP_(void) OnDownloadPayloadComplete(
        __in LPCWSTR wzPayloadId,
        __in LPCWSTR wzPayloadFileName,
        __in HRESULT hrStatus
        );

    STDMETHODIMP_(int) OnDownloadProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallPercentage
        );

    STDMETHODIMP_(int) OnExecuteProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallPercentage
        );

	STDMETHODIMP_(void) OnCacheComplete(__in HRESULT hrStatus);
	STDMETHODIMP_(int) OnRegisterBegin();
    STDMETHODIMP_(void) OnRegisterComplete(__in HRESULT hrStatus);
    STDMETHODIMP_(void) OnUnregisterBegin();
    STDMETHODIMP_(void) OnUnregisterComplete(__in HRESULT hrStatus);
	STDMETHODIMP_(int) OnExecuteBegin(__in DWORD cExecutingPackages);
	STDMETHODIMP_(int) OnExecutePackageBegin(__in LPCWSTR wzPackageId,
                                             __in BOOL fExecute);
	STDMETHODIMP_(void) OnExecutePackageComplete(__in LPCWSTR wzPackageId,
                                                 __in HRESULT hrExitCode);
	STDMETHODIMP_(void) OnExecuteComplete(__in HRESULT hrStatus);

	STDMETHODIMP_(int) OnError( __in LPCWSTR wzPackageId,
								__in DWORD dwCode,
								__in_z LPCWSTR wzError,
								__in DWORD dwUIHint);

	STDMETHODIMP_(int) OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        );

	STDMETHODIMP_(int) OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        );

	STDMETHODIMP_(BOOL) OnRestartRequired();

	STDMETHODIMP_(int) ResolveSource(
        __in    LPCWSTR wzPackageId ,
        __in    LPCWSTR wzPackageOrContainerPath
        );


	STDMETHODIMP_(BOOL) CanPackagesBeDownloaded(void);


	void SetBurnAction(BURN_ACTION burnAction);

    // Constructor/Destructor
    CChainerUXCore(HMODULE hModule,
                   BURN_COMMAND* pCommand, 
                   LPCWSTR managedSetupAssembly, 
                   LPCWSTR managedSetupClass);   
    CChainerUXCore(HMODULE hModule,
                   BURN_COMMAND* pCommand, 
                   LPCWSTR managedProxyAssembly, 
                   LPCWSTR managedProxyClass, 
                   LPCWSTR managedSetupAssembly, 
                   LPCWSTR managedSetupClass 
                   );
    virtual ~CChainerUXCore();

	static IBurnCore* gBurnCore;

private:

    // constructor helper
    void Init(HMODULE hModule,
              BURN_COMMAND* pCommand, 
              LPCWSTR managedSetupAssembly, 
              LPCWSTR managedSetupClass
             );
    void Init(HMODULE hModule,
              BURN_COMMAND* pCommand, 
              LPCWSTR managedProxyAssembly, 
              LPCWSTR managedProxyClass, 
              LPCWSTR managedSetupAssembly, 
              LPCWSTR managedSetupClass 
              );

    // methods
    BOOL HasBeenInitialized();
    BOOL UsingManagedUX();   
    
    HRESULT InitializeUX();

	HRESULT CreateMainWindow();

	void DestroyNativeWindow(HWND hwnd);

	static LRESULT CALLBACK UXWndProc(HWND hWnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam);

	HRESULT DAPI InitializeNativeUX(HMODULE hModule,                              
                                WNDPROC wndProc,                              
                                HWND* hWnd,
                                LPVOID lpParam);
    
    void ShutDownManagedHost();
    HRESULT InitializeManagedUX(LPWSTR sczModuleDirectory); 
   
    bool InvokeInitializeMethod();
   
    bool GetMethods(_AppDomain* const pAppDomain);
    bool GetMethod(_Assembly* const pAssembly, 
                   const BSTR* const bstrClass,
                   const WCHAR* const szMethod, 
                   _MethodInfo** ppMethod);      

    void UninitializeManagedUX();
};

HRESULT CreateUserExperience(_In_ HMODULE hModule,
                             _In_ BURN_COMMAND* pCommand,
                             _Out_ IBurnUserExperience **ppUX);

HRESULT __stdcall GetVariableNumeric(
        __in_z LPCWSTR wzVariable,
        __out LONGLONG* pllValue
        );

HRESULT __stdcall GetVariableString(
        __in_z LPCWSTR wzVariable,
        __out_ecount_opt(*pcchValue) LPWSTR wzValue,
        __inout DWORD* pcchValue
        );

HRESULT __stdcall GetVariableVersion(
        __in_z LPCWSTR wzVariable,
        __out DWORD64* pqwValue
        );

HRESULT __stdcall SetVariableNumeric(
        __in_z LPCWSTR wzVariable,
        __in LONGLONG llValue
        );

HRESULT __stdcall SetVariableString(
        __in_z LPCWSTR wzVariable,
        __in_z LPCWSTR wzValue
        );

HRESULT __stdcall SetVariableVersion(
        __in_z LPCWSTR wzVariable,
        __in DWORD64 qwValue
        );

HRESULT __stdcall FormatString(
        __in_z LPCWSTR wzIn,
        __out_ecount_opt(*pcchOut) LPWSTR wzOut,
        __inout DWORD* pcchOut
        );

HRESULT __stdcall EscapeString(
        __in_z LPCWSTR wzIn,
        __out_ecount_opt(*pcchOut) LPWSTR wzOut,
        __inout DWORD* pcchOut
        );

HRESULT __stdcall EvaluateCondition(
        __in_z LPCWSTR wzCondition,
        __out BOOL* pf
        );

HRESULT __stdcall Log(
        __in BURN_LOG_LEVEL level,
        __in_z LPCWSTR wzMessage
        );

HRESULT __stdcall Elevate(
        __in_opt HWND hwndParent
        );

HRESULT __stdcall Detect();

HRESULT __stdcall Plan(
        __in BURN_ACTION action
        );

HRESULT __stdcall Apply(
        __in_opt HWND hwndParent
        );

HRESULT __stdcall Suspend();

HRESULT __stdcall Reboot();

HRESULT __stdcall SetSource(
        __in    LPCWSTR wzSourcePath
        );







