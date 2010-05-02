//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description: CChainerUXCore implements the IBurnUserExperience in order
//              to provide an interface for a burnstub.exe created chainer
//              setup application
//
//              WARNING: ensure ...\Microsoft.Net\Framework\v2.0.50727 is 
//              found first in your PATH otherwise you may not be importing 
//              the correct version of mscorlib.tlb
//-----------------------------------------------------------------------------
#include "precomp.h"

#include "uxcore.h"


// needs to match MANAGED_UX_FUNCTIONS sans final enum MUX_FUNCTION_COUNT
LPCWSTR UXFunctionProxyNames[] =
{
	L"Initialize"	
};

//
// Initialize - ensure all the necessary objects are created/initialized.
//
STDMETHODIMP CChainerUXCore::Initialize(
        __in IBurnCore* pCore,
        __in int nCmdShow,
        __in BURN_RESUME_TYPE resumeType
        )
{
   HRESULT hr = S_OK;
	if( HasBeenInitialized() ) return S_OK;

	// check args...
	ExitOnNull(pCore, hr, E_INVALIDARG, "Failed to create main window: NULL IBurnCore*.");

	if( SW_HIDE > nCmdShow || SW_MAX < nCmdShow )
	{
		hr = E_INVALIDARG;
		ExitOnFailure(hr, "Failed to create main window: bad nCmdShow.");
	}

	// save IBurnCore pointer and setup the UX   
	m_pCore = pCore;
	gBurnCore = pCore;
	hr = InitializeUX();
	ExitOnFailure(hr, "Failed to create main window.");	

LExit:
	if(SUCCEEDED( hr ) )
	{
		m_initialized = S_OK;
		hr = m_pCore->Detect();
	}
	else
	{
		m_initialized = ERROR_APP_INIT_FAILURE;
	}
		   
	return hr;
}


void CChainerUXCore::SetBurnAction(BURN_ACTION burnAction)
{
	m_command.action = burnAction;
}

//
// Run - start pumping messages.
//
STDMETHODIMP CChainerUXCore::Run()
{
	if( !HasBeenInitialized() ) return m_initialized;

	HRESULT hr = S_OK;
	
LExit:
	return hr;
}

STDMETHODIMP_(void) CChainerUXCore::Uninitialize()
{
	if( HasBeenInitialized() ) 
	{
		
	}  
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectBegin(
	__in DWORD cPackages
	)
{
	int result = IDOK;

	if( !HasBeenInitialized() ) 
	{
		return IDABORT;
	}

	

	return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectPackageBegin(
	__in_z LPCWSTR wzPackageId
	)
{
	int result = IDOK;

	if( !HasBeenInitialized() ) 
	{
		return IDABORT;
	}

	

	return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnDetectPackageComplete(
	__in LPCWSTR wzPackageId,
	__in HRESULT hrStatus,
	__in PACKAGE_STATE state
	)
{
	if( HasBeenInitialized() )
	{
		
	}
}


STDMETHODIMP_(void) CChainerUXCore::OnDetectComplete(
	__in HRESULT hrStatus
	)
{
	if (HasBeenInitialized())
	{
		InvokeInitializeMethod();
	}
}

STDMETHODIMP_(int) CChainerUXCore::OnPlanBegin(
    __in DWORD cPackages
    )
{
    int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }

   
    return result;
}


STDMETHODIMP_(int) CChainerUXCore::OnPlanPackageBegin(
    __in_z LPCWSTR wzPackageId,
    __inout_z REQUEST_STATE* pRequestedState
    )
{
    int result = IDOK;

    if( !HasBeenInitialized()  ) 
    {
        return IDABORT;
    }

   

    return result;
}


STDMETHODIMP_(void) CChainerUXCore::OnPlanPackageComplete(
    __in LPCWSTR wzPackageId,
    __in HRESULT hrStatus,
    __in PACKAGE_STATE state,
    __in REQUEST_STATE requested,
    __in ACTION_STATE execute,
    __in ACTION_STATE rollback
    )
{
    if( HasBeenInitialized() )
    {
      
    }
}


STDMETHODIMP_(void) CChainerUXCore::OnPlanComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
        
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnApplyBegin()
{
    int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }

   

    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnApplyComplete(
    __in HRESULT hrStatus
    )
{
    if( HasBeenInitialized() )
    {
        
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnProgress(
    __in DWORD dwProgressPercentage,
    __in DWORD dwOverallProgressPercentage
    )
{    

    if( !HasBeenInitialized()) 
    {
        return IDERROR;
    }

   

    return m_nCmdResult;
}

STDMETHODIMP_(int) CChainerUXCore::OnCacheBegin()
{
	int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }
    


    return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnCachePackageBegin(
    __in LPCWSTR wzPackageId,
    __in DWORD64 dw64PackageCacheSize
    )
{
	int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }
    return result;

}

STDMETHODIMP_(int) CChainerUXCore::OnDownloadPayloadBegin(
	__in LPCWSTR wzPayloadId,
	__in LPCWSTR wzPayloadFileName
	)
{
	int result = IDOK;

	if( !HasBeenInitialized() ) 
	{
		return IDABORT;
	}
	return result;

}

STDMETHODIMP_(void) CChainerUXCore::OnDownloadPayloadComplete(
	__in LPCWSTR wzPayloadId,
	__in LPCWSTR wzPayloadFileName,
	__in HRESULT hrStatus
	)
{


}

STDMETHODIMP_(int) CChainerUXCore::OnDownloadProgress(
	__in DWORD dwProgressPercentage,
	__in DWORD dwOverallPercentage
	)
{
	int result = IDOK;

	if( !HasBeenInitialized() ) 
	{
		return IDABORT;
	}
	return result;

}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteProgress(
	__in DWORD dwProgressPercentage,
	__in DWORD dwOverallPercentage
	)
{
	int result = IDOK;

	if( !HasBeenInitialized() ) 
	{
		return IDABORT;
	}
	return result;

}

STDMETHODIMP_(void) CChainerUXCore::OnCachePackageComplete(
    __in LPCWSTR wzPackageId,
	__in HRESULT hrStatus
    
    )
{
	

}

STDMETHODIMP_(void) CChainerUXCore::OnCacheComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
       
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnDetectPriorBundle(
        __in_z LPCWSTR wzBundleId
        )
{
	int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }

    return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnPlanPriorBundle(
        __in_z LPCWSTR wzBundleId,
        __inout_z REQUEST_STATE* pRequestedState
        )
{
	int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }
    return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnRegisterBegin()
{
    int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }
    


    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnRegisterComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
        
    }
}

STDMETHODIMP_(void) CChainerUXCore::OnUnregisterBegin()
{
}

STDMETHODIMP_(void) CChainerUXCore::OnUnregisterComplete(
    __in HRESULT hrStatus
    )
{
    if (HasBeenInitialized())
    {
       
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteBegin(
    __in DWORD cExecutingPackages
    )
{
    int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }

   

    return result;
}

STDMETHODIMP_(int) CChainerUXCore::OnExecutePackageBegin(
    __in LPCWSTR wzPackageId,
    __in BOOL fExecute
    )
{
    int result = IDOK;

    if( !HasBeenInitialized() ) 
    {
        return IDABORT;
    }

    
    return result;
}

STDMETHODIMP_(void) CChainerUXCore::OnExecutePackageComplete(
    __in LPCWSTR wzPackageId,
    __in HRESULT hrExitCode
    )
{
    if( HasBeenInitialized() )
    {
       
    }
}


STDMETHODIMP_(void) CChainerUXCore::OnExecuteComplete(
    __in HRESULT hrStatus
    )
{
    if( HasBeenInitialized() )
    {
       
    }
}

STDMETHODIMP_(int) CChainerUXCore::OnError(
    __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
    )
{
   
    return IDABORT; // TODO
}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )

{

	return IDOK;

}

STDMETHODIMP_(int) CChainerUXCore::OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
    {
        return IDOK;
    }

STDMETHODIMP_(BOOL) CChainerUXCore::OnRestartRequired()
{
    return true; // TODO
}

STDMETHODIMP_(int) CChainerUXCore::ResolveSource(
        __in    LPCWSTR wzPackageId ,
        __in    LPCWSTR wzPackageOrContainerPath
        )
{
	return IDOK; // TODO
}

STDMETHODIMP_(BOOL) CChainerUXCore::CanPackagesBeDownloaded()
{
	return true; // TODO

}

BOOL CChainerUXCore::HasBeenInitialized()
{
	return m_initialized == S_OK;
}

BOOL CChainerUXCore::UsingManagedUX()
{	
	return true;  // always using the UX, it will hide itself in slient/passive modes.
}

//
// InitializeUX - preps interface based on BURN_DISPLAY setting
//
HRESULT CChainerUXCore::InitializeUX()
{
	HRESULT hr = S_OK;
	LPWSTR sczModuleDirectory = NULL;   

	// need to figure out what our working path is relative to the DLL
	WCHAR buffer[MAX_PATH];
	if(::GetModuleFileNameW(m_hModule, buffer, sizeof(buffer)/2))
	{
		hr = PathGetDirectory(buffer, &sczModuleDirectory);
		ExitOnFailure(hr, "Failed to get directory from module");
	}
	else
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unable to set retrieve module path");
	}

	if( SUCCEEDED(hr) )
	{
		hr = InitializeManagedUX(sczModuleDirectory);
		ExitOnFailure(hr, "Unable to create full display UX (managed) window.");
	}
	
	ExitOnFailure(hr, "Unable to create main window.");

LExit:  
	ReleaseStr(sczModuleDirectory);

	return hr;
}

void CChainerUXCore::ShutDownManagedHost()
{
	if( m_pAppDomain )
	{
		m_pClrHost->UnloadDomain(m_pAppDomain);
		ReleaseNullObject(m_pAppDomain);
	}

	if( m_pClrHost )
	{
		m_pClrHost->Stop();
		ReleaseNullObject(m_pClrHost);
	}
}

HRESULT CChainerUXCore::InitializeManagedUX(LPWSTR sczModuleDirectory)
{
	if( m_pClrHost ) return S_OK;
	HRESULT hr = S_OK;
	IUnknown *pIUnknown = NULL;
	IAppDomainSetup *pAppDomainSetup = NULL;
	BSTR appBase = NULL;

	ExitOnNull(sczModuleDirectory, hr, E_INVALIDARG, "Received NULL module directory path.");

	hr = CorBindToRuntimeEx(
		DOTNET20,
		WORKSTATION_BUILD, // because this runs on end-user PCs, not servers
		STARTUP_CONCURRENT_GC|STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN, 
		CLSID_CorRuntimeHost,
		IID_ICorRuntimeHost,
		(void**)&m_pClrHost);
	ExitOnFailure(hr, "Unable to initialize CLR for UX (managed) window.");

	hr = m_pClrHost->Start();
	ExitOnFailure(hr, "Unable to start hosted CLR for UX (managed) window.");

	hr = m_pClrHost->CreateDomainSetup(&pIUnknown);
	ExitOnFailure(hr, "Unable to get create domain setup for hosted CLR.");    

	hr = pIUnknown->QueryInterface(__uuidof(IAppDomainSetup),
		(void**) &pAppDomainSetup);
	ReleaseNullObject(pIUnknown);

	ExitOnFailure(hr, "Unable to retrieve IAppDomainSetup pointer for hosted CLR.");

	appBase = ::SysAllocString(sczModuleDirectory);
	ExitOnNull(appBase, hr, E_OUTOFMEMORY, "Unable to set ApplicationBase: Out of memory");

	hr = pAppDomainSetup->put_ApplicationBase(appBase);
	ExitOnFailure(hr, "Unable to set ApplicationBase.");

	hr = m_pClrHost->CreateDomainEx(L"ManagedInstallUX", pAppDomainSetup, NULL, &pIUnknown);
	ExitOnFailure(hr, "Unable to get create domain for hosted CLR.");

	hr = pIUnknown->QueryInterface(__uuidof(_AppDomain),
		(void**) &m_pAppDomain);
	ReleaseNullObject(pIUnknown);
	ExitOnFailure(hr, "Unable to retrieve _AppDomain pointer for hosted CLR.");

	if( !GetMethods(m_pAppDomain) )
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unable to retrieve pointers to managed methods.");
	}	

LExit:
	ReleaseBSTR(appBase)
	ReleaseNullObject(pAppDomainSetup);

	if( FAILED(hr) )
	{
		ShutDownManagedHost();
	}

	return hr;
}

bool CChainerUXCore::InvokeInitializeMethod()
{

	if( !m_managedMethods[MUX_Initialize])		
	{
		return false;
	}	

	HRESULT hr;
	VARIANT vResult;
	VariantInit(&vResult);
	LONG index = 0;
	VARIANT vNull;
	vNull.vt = VT_EMPTY;

	// okay, now invoke Initialize
	hr = m_managedMethods[MUX_Initialize]->Invoke_3(vNull, NULL, &vResult);
	ExitOnFailure(hr, "Unable to invoke MUX_Initialize().");

LExit:
	
	if (SUCCEEDED(hr))
	{
		bool result;
		if (vResult.boolVal == VARIANT_TRUE)
		{
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}
	else
	{
		return false;
	}
}

bool CChainerUXCore::GetMethods(_AppDomain* const pAppDomain)
{
	HRESULT hr = S_OK;
	BSTR bstrAssemblyName = NULL;
	BSTR bstrClassName = NULL;
	_Assembly* pAssembly = NULL;

	ExitOnNull(m_managedProxyAssembly, hr, E_POINTER, "Managed Proxy assembly pointer not set.");
	ExitOnNull(m_managedProxyClass, hr, E_POINTER, "Managed Proxy class  pointer not set.");
	ExitOnNull(pAppDomain, hr, E_INVALIDARG, "Did not receive a loaded AppDomain");

	bstrAssemblyName = ::SysAllocString(m_managedProxyAssembly);
	ExitOnNull(bstrAssemblyName, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for assembly name");

	bstrClassName = ::SysAllocString(m_managedProxyClass);
	ExitOnNull(bstrClassName, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for managed class name");

	hr = pAppDomain->Load_2(bstrAssemblyName, &pAssembly);
	ExitOnFailure(hr, "Unable to load Assembly");

	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		if( !GetMethod(pAssembly, 
			&bstrClassName, 
			UXFunctionProxyNames[i], 
			&m_managedMethods[i]) )
		{
			ExitOnFailure1(hr = E_UNEXPECTED, "Unable to obtain pointer to managed %s function.", UXFunctionProxyNames[i]);
		}
	}

LExit:
	ReleaseBSTR(bstrAssemblyName);
	ReleaseBSTR(bstrClassName);
	ReleaseObject(pAssembly);

	return SUCCEEDED(hr);
}

bool CChainerUXCore::GetMethod(_Assembly* const pAssembly, 
							   const BSTR* const bstrClass,
							   const WCHAR* const szMethod, 
							   _MethodInfo** ppMethod)
{
	HRESULT hr = S_OK;
	_Type* pType = NULL;
	BSTR bstrMethod = NULL;

	ExitOnNull(pAssembly, hr, E_INVALIDARG, "Did not receive a loaded Assembly.");
	ExitOnNull(bstrClass, hr, E_INVALIDARG, "Did not receive a Proxy Class name.");
	ExitOnNull(szMethod, hr, E_INVALIDARG, "Did not receive a Method name.");

	hr = pAssembly->GetType_2(*bstrClass, &pType);
	ExitOnFailure(hr, "Unable to get proxy class from managed assembly");
	ExitOnNull(pType, hr, E_POINTER, "Unable to get proxy class from managed assembly");

	bstrMethod = ::SysAllocString(szMethod);
	ExitOnNull(bstrMethod, hr, E_OUTOFMEMORY, "Out of memory: Unable to allocate BSTR for proxy function name");

	hr = pType->GetMethod_2(bstrMethod,
		(BindingFlags) (BindingFlags_Public | BindingFlags_Static), 
		ppMethod);
	ExitOnFailure(hr, "Unable to get proxy class method from managed assembly");
	ExitOnNull(*ppMethod, hr, E_POINTER, "Unable to get proxy class method from managed assembly");

LExit:
	ReleaseObject(pType);
	ReleaseBSTR(bstrMethod);

	return SUCCEEDED(hr);
}

void CChainerUXCore::UninitializeManagedUX()
{
	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		ReleaseNullObject(m_managedMethods[i]);
	}

	// once the managed object has been cleaned up, 
	// shutdown the host
	ShutDownManagedHost();
}


//
// Constructors
//

CChainerUXCore::CChainerUXCore(
							   __in HMODULE hModule,
							   __in BURN_COMMAND* pCommand,
							   __in LPCWSTR managedSetupAssembly,
							   __in LPCWSTR managedSetupClass   
							   )
{
	Init(hModule, pCommand, managedSetupAssembly, managedSetupClass);
}

CChainerUXCore::CChainerUXCore(HMODULE hModule,
							   BURN_COMMAND* pCommand, 
							   LPCWSTR managedProxyAssembly, 
							   LPCWSTR managedProxyClass, 
							   LPCWSTR managedSetupAssembly, 
							   LPCWSTR managedSetupClass
							   )
{
	Init(hModule, pCommand, managedProxyAssembly, managedProxyClass, managedSetupAssembly, managedSetupClass);
}

//
// Init (constructor helper) - intitialize member variables.
//
void CChainerUXCore::Init(
						  __in HMODULE hModule,
						  __in BURN_COMMAND* pCommand,
						  __in LPCWSTR managedSetupAssembly,
						  __in LPCWSTR managedSetupClass   
						  )
{
	Init(hModule,
		pCommand,
		L"ManagedBurnProxy",
		L"ManagedBurnProxy.ManagedSetupUXProxy",
		managedSetupAssembly,
		managedSetupClass
		);
}

void CChainerUXCore::Init(
						  __in HMODULE hModule,
						  __in BURN_COMMAND* pCommand,
						  __in LPCWSTR managedProxyAssembly, 
						  __in LPCWSTR managedProxyClass, 
						  __in LPCWSTR managedSetupAssembly,
						  __in LPCWSTR managedSetupClass

						  )
{
	m_initialized = ERROR_APP_INIT_FAILURE;

	m_pClrHost = NULL;
	m_pAppDomain = NULL;

	m_hModule = hModule;
	memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BURN_COMMAND));

	m_pCore = NULL;
	m_hWnd = NULL;
	m_installCanceled = FALSE;
	m_cancelInstall = FALSE;
	m_nCmdResult = IDNOACTION;

	if (managedProxyAssembly)
	{
		m_managedProxyAssembly = managedProxyAssembly;
	}
	else
	{
		m_managedProxyAssembly = NULL;
	}

	if (managedProxyClass)
	{
		m_managedProxyClass = managedProxyClass;
	}
	else
	{
		m_managedProxyClass = NULL;
	}

	if (managedSetupAssembly)
	{
		m_managedSetupAssembly = managedSetupAssembly;
	}
	else
	{
		m_managedSetupAssembly = NULL;
	}

	if (managedSetupClass)
	{
		m_managedSetupClass = managedSetupClass;
	}
	else
	{
		m_managedSetupClass = NULL;
	}   

	for(int i = 0; i < MUX_FUNCTION_COUNT; ++i)
	{
		m_managedMethods[i] = NULL;
	}
}

//
// Destructor - release member variables.
//
CChainerUXCore::~CChainerUXCore()
{
	if( UsingManagedUX() )
	{
		UninitializeManagedUX();
	}
}


BOOL RequiredDotNetVersionFound()
{
	BOOL found = FALSE;
	HKEY hKey;

	if( ERROR_SUCCESS == ::RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP",
		0, KEY_READ, &hKey) )
	{
		DWORD dwIndex = 0;
		const int BUF_LEN = 256;
		WCHAR buffer[BUF_LEN];

		while( !found )
		{
			DWORD len = BUF_LEN;
			for (int i = 0; i < BUF_LEN; ++i)
			{
				buffer[i] = 0;
			}

			if( ERROR_SUCCESS == ::RegEnumKeyEx(
				hKey,
				dwIndex,
				buffer,
				&len,
				NULL,
				NULL,
				NULL, 
				NULL) )
			{
				if( len > 0 && lstrlenW(buffer) > 0 )
				{
					if( lstrcmpW(buffer, DOTNET20) == 0 )
					{
						found = TRUE;
						break;
					}
				}
			}
			else
			{
				break;
			}

			dwIndex++;
		}
	}

	return found;
}

BOOL ManagedDependenciesInstalled()
{
	if( !RequiredDotNetVersionFound() ) 
	{
		return FALSE;
	}

	return TRUE;
}

IBurnCore* CChainerUXCore::gBurnCore = NULL;