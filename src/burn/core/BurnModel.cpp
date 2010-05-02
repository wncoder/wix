//-------------------------------------------------------------------------------------------------
// <copyright file="BurnModel.cpp" company="Microsoft">
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
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

typedef struct _BURN_PLAN_CACHE_PAYLOAD
{
    DWORD iPackage;
    DWORD iPayload;
    DWORD64 dw64Size;
} BURN_PLAN_CACHE_PAYLOAD;

typedef struct _BURN_PLAN_EXECUTE_PACKAGE
{
    DWORD iPackage;
    BOOL fOwned;
    ACTION_STATE execute;
} BURN_PLAN_EXECUTE_PACKAGE;


class CBurnModel : public CUnknownImpl2<IBurnModel, IBurnModelTestable>
{
public: // IBurnModel
    //
    // GetBundleId - gets the bundle id loaded from the manifest.
    //
    STDMETHODIMP GetBundleId(
        __out_z LPWSTR* psczBundleId
        )
    {
        HRESULT hr = S_OK;

        hr = StrAllocString(psczBundleId, m_sczBundleId, 0);
        ExitOnFailure(hr, "Failed to get bundle id.");

    LExit:
        return hr;
    }


    //
    // GetCommand - gets the commands Burn should be executing.
    //
    STDMETHODIMP GetCommand(
        __out BURN_COMMAND* pCommand
        )
    {
        memcpy_s(pCommand, sizeof(BURN_COMMAND), &m_command, sizeof(m_command));
        return S_OK;
    }


    STDMETHODIMP_(void) SetCommandAction(
        __in BURN_ACTION action
        )
    {
        if (BURN_ACTION_HELP < action)
        {
            m_command.action = action;
        }
    }


    //
    // GetCommandLineParameters - gets the command line parameters passed in with the exe
    //
    STDMETHODIMP GetCommandLineParameters(
        __out_z LPWSTR* psczCommandLine
        )
    {
        HRESULT hr = S_OK;

        if (m_sczCommandLine)
        {
            hr = StrAllocString(psczCommandLine, m_sczCommandLine, 0);
            ExitOnFailure(hr, "Error getting command line.");
        }
        else
        {
            ExitFunction1(hr = S_FALSE);
        }

    LExit:
        return hr;
    }


    //
    // Mode - returns whether we are elevated, embedded or just running normal.
    //
    STDMETHODIMP_(BURN_MODE) Mode()
    {
        return m_mode;
    }

    STDMETHODIMP_(void) GetPackage(
        __in DWORD dwIndex,
        __out IBurnPackage** ppPackage
        )
    {
        AssertSz(dwIndex <= m_cPackages, "Package index out of bounds.");

        m_rgpPackages[dwIndex - 1]->AddRef();
        *ppPackage = m_rgpPackages[dwIndex - 1];
    }


    STDMETHODIMP GetPackageIndex(
        __in_z LPCWSTR wzId,
        __out DWORD* pdwIndex
        )
    {
        HRESULT hr = S_OK;
        DWORD i;

        for (i = 0; i < m_cPackages; ++i)
        {
            if (CSTR_EQUAL == m_rgpPackages[i]->CompareId(wzId))
            {
                *pdwIndex = i + 1; // remember, package indices are "1" based.
                break;
            }
        }

        if (i == m_cPackages)
        {
            hr = E_INVALIDARG;
            ExitOnRootFailure1(hr, "Failed to find package with id: %ls", wzId);
        }

    LExit:
        return hr;
    }


    STDMETHODIMP_(DWORD) PackageCount()
    {
        return m_cPackages;
    }


    STDMETHODIMP_(DWORD) UXPayloadCount()
    {
        return m_cUXPayloads;
    }


    STDMETHODIMP_(void) GetUXPayload(
        __in DWORD dwIndex,
        __out IBurnPayload** ppPayload
        )
    {
        AssertSz(dwIndex <= m_cUXPayloads, "UX payload index out of bounds.");

        m_rgpUXPayloads[dwIndex - 1]->AddRef();
        *ppPayload = m_rgpUXPayloads[dwIndex - 1];
    }


    STDMETHODIMP_(BOOL) IsRestartRequired()
    {
        BOOL fRestartRequired = FALSE;
        if (BURN_RESTART_NEVER != m_command.restart)
        {
            if (BURN_RESTART_ALWAYS == m_command.restart || (m_fRestartRequested && BURN_RESTART_AUTOMATIC == m_command.restart) || (m_fRestartAllowed && BURN_RESTART_PROMPT == m_command.restart))
            {
                fRestartRequired = TRUE;
            }
        }

        return fRestartRequired;
    }


    STDMETHODIMP_(BOOL) WasRestartRequested()
    {
        return m_fRestartRequested;
    }


    STDMETHOD_(void, SetRestartRequired)(
        __in BOOL fRestartRequired
        )
    {
        m_fRestartRequested = TRUE;
        m_fRestartAllowed = fRestartRequired;
    }

    //
    // Load - loads the embedded or external manifests.
    //
    STDMETHODIMP Load(
        __in_z_opt LPCWSTR sczBundleName
        )
    {
        HRESULT hr = S_OK;

        LPWSTR sczFileName = NULL;
        LPWSTR sczBundleId = NULL;
        DWORD cPackages = 0;
        IBurnPackage** rgpPackages = NULL;
        DWORD cUXPayloads = 0;
        IBurnPayload** rgpUXPayloads = NULL;
        BURN_REGISTRATION* pRegistration = NULL;

        ManifestInitialize();

        if (m_sczManifestFile)
        {
            // TODO
            // ManifestParseFromFile(...)
            ExitOnFailure(hr = E_NOTIMPL, "External manifests are not supported.");
        }
        else
        {
            if (!sczBundleName)
            {
                hr = PathForCurrentProcess(&sczFileName, NULL);
                ExitOnFailure(hr, "Failed to get path for current process.");
            }

            hr = ManifestParseFromEmbedded(sczBundleName ? sczBundleName : sczFileName, this, &sczBundleId, &rgpPackages, &cPackages, &rgpUXPayloads, &cUXPayloads, &this->m_searches, &this->m_properties, &pRegistration);
            ExitOnFailure(hr, "Failed to load embedded manifest");
        }

        hr = SearchesSort(&this->m_searches);
        ExitOnFailure(hr, "Failed to sort searches.");

        m_sczBundleId = sczBundleId;
        sczBundleId = NULL;

        m_cPackages = cPackages;
        m_rgpPackages = rgpPackages;
        rgpPackages = NULL;
        for (DWORD i = 0; i < m_cPackages; ++i)
        {
            m_rgpPackages[i]->SetIndex(i + 1); // remember package indicies are 1-based.
        }

        m_cUXPayloads = cUXPayloads;
        m_rgpUXPayloads = rgpUXPayloads;
        rgpUXPayloads = NULL;

        m_pRegistration = pRegistration;
        pRegistration = NULL;

    LExit:
        ManifestUninitialize();
        ReleaseStr(sczFileName);
        ReleaseStr(sczBundleId);
        ReleaseObjectArray(rgpPackages, cPackages);
        ReleaseObjectArray(rgpUXPayloads, cUXPayloads);
        if (pRegistration)
        {
            RegistrationFree(pRegistration);
        }

        return hr;
    }


    //
    // InitializeLogName - generate the chain-wide log filename based on command-line switch, manifest, or defaults.
    //
    STDMETHODIMP InitializeLogName(
        __in BURN_LOGGING_MODE logMode,
        __in_z_opt LPCWSTR wzLogPath
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczNewLogPath = NULL;
        LPWSTR sczLogDirectory = NULL;
        HANDLE hLogFile = INVALID_HANDLE_VALUE;

        // the chain log set by the /log command-line switch overrides the defaults set in the manifest
        if (m_sczLogFile)
        {
            Trace(REPORT_STANDARD, "Model log has already been initialized");
            hr = S_FALSE;
            ExitFunction();
        }

        if (wzLogPath)
        {
            // resolve %environment_variables%
            hr = FileResolvePath(wzLogPath, &sczNewLogPath);
            ExitOnFailure(hr, "Failed to resolve incoming log path.");
        }
        else
        {
            // the default chain log is <nameOfBundle>.exe.log in the current directory
            hr = PathForCurrentProcess(&sczNewLogPath, NULL);
            ExitOnFailure(hr, "Failed to get current process path for log.");

            hr = StrAllocString(&sczNewLogPath, FileFromPath(sczNewLogPath), 0);
            ExitOnFailure(hr, "Failed to copy current process file name for log.");

            hr = FileChangeExtension(sczNewLogPath, L".log", &sczNewLogPath);
            ExitOnFailure(hr, "Failed to rename current process log.");
        }

        // create the directory if one was provided
        hr = PathGetDirectory(sczNewLogPath, &sczLogDirectory);
        ExitOnFailure1(hr, "Failed to get directory from log path: %ls", wzLogPath);

        if (sczLogDirectory)
        {
            hr = DirEnsureExists(sczLogDirectory, NULL);
            ExitOnFailure(hr, "Failed to create log directory.");
        }

        // handle rename-mode collision-avoidance using PathCreateTempFile's templating
        // (the PathCreateTempFile template is <logBaseName>.%04x.<logExtension>);
        // otherwise we're appending/overwriting and LogInitialize handles that for us
        if (BURN_LOGGING_MODE_UNIQUE == logMode)
        {
            hr = FileAddSuffixToBaseName(FileFromPath(sczNewLogPath), L".%04x", &sczNewLogPath);
            ExitOnFailure(hr, "Failed to allocate renamed log template.");

            hr = PathCreateTempFile(sczLogDirectory, sczNewLogPath, SHRT_MAX, FILE_ATTRIBUTE_NORMAL, &sczNewLogPath, NULL);
            ExitOnFailure(hr, "Failed to create log file.");
        }

        m_sczLogFile = sczNewLogPath;
        sczNewLogPath = NULL;

    LExit:
        ReleaseStr(sczLogDirectory);
        ReleaseStr(sczNewLogPath);
        ReleaseFile(hLogFile);

        return hr;
    }


    //
    // GetProperties - returns the property collection of the model
    //
    STDMETHODIMP_(BURN_PROPERTIES*) GetProperties()
    {
        return &this->m_properties;
    }


    //
    // GetSearches - returns the search collection of the model
    //
    STDMETHODIMP_(BURN_SEARCHES*) GetSearches()
    {
        return &this->m_searches;
    }


    STDMETHODIMP_(void) PlanReset()
    {
        m_fPlanned = FALSE;
        m_fRegisterBundlePerMachine = FALSE;
        m_fPlanRegistrationRequired = FALSE;

        ReleaseNullMem(m_prgCachePayloads);
        m_cMaxCachePayloads = 0;
        m_cCachePayloads = 0;

        ReleaseNullMem(m_prgExecutePackages);
        m_cMaxExecutePackages = 0;
        m_cExecutePackages = 0;

        ReleaseNullBuffer(m_pbElevatedPlan);
        m_cbElevatedPlan = 0;
        m_cElevatedPlanPackages = 0;
    }


    STDMETHODIMP_(void) PlanReady()
    {
        m_fPlanned = TRUE;
    }


    STDMETHODIMP_(BOOL) PlanIsReady()
    {
        return m_fPlanned;
    }


    STDMETHODIMP_(BOOL) PlanIsPerMachine()
    {
        // The plan is per-machine if there are any elevated packages planned.
        return 0 < m_cElevatedPlanPackages;
    }


    STDMETHODIMP_(BOOL) PlanRegisterBundlePerMachine()
    {
        return m_fRegisterBundlePerMachine;
    }


    STDMETHODIMP_(BOOL) PlanRequiresRegistration()
    {
        return m_fPlanRegistrationRequired;
    }


    STDMETHODIMP_(BOOL) PlanRequiresUnregistration()
    {
        return (BURN_ACTION_UNINSTALL == m_command.action);
    }


    STDMETHODIMP PlanCache(
        __in IBurnPackage* pPackage,
        __in IBurnPayload* pPayload
        )
    {
        HRESULT hr = S_OK;

        MEM_ENSURE_ARRAY_SIZE(BURN_PLAN_CACHE_PAYLOAD, m_prgCachePayloads, m_cCachePayloads, m_cMaxCachePayloads, 5, hr, "Failed to allocate cache payload plan.");

        m_prgCachePayloads[m_cCachePayloads].iPackage = pPackage->Index();
        m_prgCachePayloads[m_cCachePayloads].iPayload = pPayload->Index();
        m_prgCachePayloads[m_cCachePayloads].dw64Size = pPayload->Size();
        ++m_cCachePayloads;

        // If we're going to cache something and we have registration then we need to be registered.
        if (m_pRegistration)
        {
            m_fPlanRegistrationRequired = TRUE;
        }

    LExit:
        return hr;
    }


    STDMETHODIMP_(DWORD) PlanCacheCount()
    {
        return m_cCachePayloads;
    }


    STDMETHODIMP_(void) PlanCacheTotalInfo(
        __out DWORD* pcCachePackages,
        __out DWORD* pcCachePayloads,
        __out DWORD64* pdw64CacheSize
        )
    {
        DWORD iPreviousPackage = 0;

        *pcCachePackages = 0;
        *pcCachePayloads = m_cCachePayloads;
        *pdw64CacheSize = 0;

        for (DWORD i = 0; i < m_cCachePayloads; ++i)
        {
            if (m_prgCachePayloads[i].iPackage != iPreviousPackage)
            {
                ++*pcCachePackages;
                iPreviousPackage = m_prgCachePayloads[i].iPackage;
            }

            *pdw64CacheSize += m_prgCachePayloads[i].dw64Size;
        }
    }


    STDMETHOD_(void, PlanCacheGetPackage)(
        __in DWORD iPlanCachePackage,
        __out IBurnPackage** ppPackage
        )
    {
        DWORD iLastPackage = 0;
        DWORD cPackages = 0;
        for (DWORD i = 0; i < m_cCachePayloads; ++i)
        {
            if (m_prgCachePayloads[i].iPackage != iLastPackage)
            {
                iLastPackage = m_prgCachePayloads[i].iPackage;
                ++cPackages;

                if (iPlanCachePackage < cPackages)
                {
                    break;
                }
            }
        }

        AssertSz(iPlanCachePackage + 1 == cPackages, "Failed to find package index in plan of cached packages.");
        this->GetPackage(iLastPackage, ppPackage);
    }


    STDMETHODIMP_(void) PlanCachePackageTotalInfo(
        __in IBurnPackage* pPackage,
        __out DWORD* pcCachePayloads,
        __out DWORD64* pdw64CacheSize
        )
    {
        DWORD iPackage = pPackage->Index();

        *pcCachePayloads = 0;
        *pdw64CacheSize = 0;

        for (DWORD i = 0; i < m_cCachePayloads; ++i)
        {
            if (m_prgCachePayloads[i].iPackage == iPackage)
            {
                ++*pcCachePayloads;
                *pdw64CacheSize += m_prgCachePayloads[i].dw64Size;
            }
        }
    }


    STDMETHOD_(void, PlanCacheGetPayload)(
        __in IBurnPackage* pPackage,
        __in DWORD iCachePayload,
        __out IBurnPayload** ppPayload
        )
    {
        DWORD iPackage = pPackage->Index();
        DWORD iMatchedPackage = 0;
        for (DWORD i = 0; i < m_cCachePayloads; ++i)
        {
            if (m_prgCachePayloads[i].iPackage == iPackage)
            {
                if (iMatchedPackage == iCachePayload)
                {
                    pPackage->GetPayload(m_prgCachePayloads[i].iPayload, ppPayload);
                    break;
                }

                ++iMatchedPackage;
            }
        }

        AssertSz(*ppPayload, "Failed to find payload.");
    }


    STDMETHODIMP PlanExecute(
        __in IBurnPackage* pPackage
        )
    {
        HRESULT hr = S_OK;
        ACTION_STATE execute = ACTION_STATE_NONE;

        MEM_ENSURE_ARRAY_SIZE(BURN_PLAN_EXECUTE_PACKAGE, m_prgExecutePackages, m_cExecutePackages, m_cMaxExecutePackages, 5, hr, "Failed to allocate execute package plan.");

        m_prgExecutePackages[m_cExecutePackages].iPackage = pPackage->Index();
        ++m_cExecutePackages;

        // Per-machine packages always get their plan information added to the elevated plan since
        // this info will have to be passed over to the elevated process when applying.
        if (pPackage->IsPerMachine())
        {
            hr = BuffWriteNumber(&m_pbElevatedPlan, &m_cbElevatedPlan, pPackage->Index());
            ExitOnFailure(hr, "Failed to write the package index to the elevated information.");

            hr = pPackage->SerializeElevatedState(&m_pbElevatedPlan, &m_cbElevatedPlan);
            ExitOnFailure(hr, "Failed to serialize elevated information while planning per-machine package.");

            ++m_cElevatedPlanPackages;
        }

        // For uninstallable packages when the bundle will be registered, we need to check a few more things
        // since uninstallable packages affect the plan more.
        if (m_pRegistration && pPackage->IsUninstallable())
        {
            // If this package is per-machine then the bundle must be per-machine.
            // TODO: Should this be true?
            if (pPackage->IsPerMachine())
            {
                m_fRegisterBundlePerMachine = TRUE;
            }

            // If this package is being installed then the bundle must be registered.
            pPackage->GetState(NULL, NULL, NULL, &execute, NULL);
            if (ACTION_STATE_INSTALL <= execute)
            {
                m_fPlanRegistrationRequired = TRUE;
            }
        }

    LExit:
        return hr;
    }


    STDMETHODIMP_(DWORD) PlanExecuteCount()
    {
        return m_cExecutePackages;
    }


    STDMETHODIMP_(void) PlanExecuteGetPackage(
        __in DWORD iExecutePackage,
        __out IBurnPackage** ppPackage
        )
    {
        AssertSz(iExecutePackage < m_cExecutePackages, "Invalid execute package index provided.");
        this->GetPackage(m_prgExecutePackages[iExecutePackage].iPackage, ppPackage);
    }


    STDMETHODIMP PlanGetElevatedPlan(
        __out_bcount(*pcbElevatedPlan) LPVOID* ppvElevatedPlan,
        __out DWORD* pcbElvatedPlan
        )
    {
        HRESULT hr = S_OK;
        BYTE* pb = NULL;
        SIZE_T cb = 0;

        hr = PropertySerialize(&this->m_properties, &pb, &cb);
        ExitOnFailure(hr, "Failed to serialize properties for elevated plan.");

        hr = BuffWriteNumber(&pb, &cb, m_cElevatedPlanPackages);
        ExitOnFailure(hr, "Failed to write number of elevated packages to elevated plan.");

        hr = BuffWriteStream(&pb, &cb, m_pbElevatedPlan, m_cbElevatedPlan);
        ExitOnFailure(hr, "Failed to write the elevated packages to the elevated plan.");

        *pcbElvatedPlan = static_cast<DWORD>(cb);
        *ppvElevatedPlan = static_cast<LPVOID>(pb);
        pb = NULL;

    LExit:
        ReleaseBuffer(pb);
        return hr;
    }


    STDMETHODIMP PlanSetElevatedPlan(
        __in_bcount(cbElevatedPlan) LPVOID pvElevatedPlan,
        __in DWORD cbElevatedPlan
        )
    {
        HRESULT hr = S_OK;
        BYTE* pbBuffer = static_cast<BYTE*>(pvElevatedPlan);
        SIZE_T iBuffer = 0;
        DWORD iPackage = 0;
        IBurnPackage* pPackage = NULL;

        BYTE* pbB = NULL;
        SIZE_T iB = 0;

        hr = PropertyDeserialize(&this->m_properties, pbBuffer, cbElevatedPlan, &iBuffer);
        ExitOnFailure(hr, "Failed to deserialize properties for elevated plan.");

        hr = BuffReadNumber(pbBuffer, cbElevatedPlan, &iBuffer, &m_cElevatedPlanPackages);
        ExitOnFailure(hr, "Failed to read number of elevated packages from elevated plan.");

        if (m_cPackages < m_cElevatedPlanPackages)
        {
            hr = E_INVALIDARG;
            ExitOnRootFailure(hr, "Too many elevated packages planned.");
        }

        hr = BuffReadStream(pbBuffer, cbElevatedPlan, &iBuffer, &pbB, &iB);
        ExitOnFailure(hr, "Failed to read stream.");

        pbBuffer = pbB;
        cbElevatedPlan = iB;
        iBuffer = 0;

        for (DWORD i = 0; i < m_cElevatedPlanPackages; ++i)
        {
            hr = BuffReadNumber(pbBuffer, cbElevatedPlan, &iBuffer, &iPackage);
            ExitOnFailure(hr, "Failed to read number of elevated packages from elevated plan.");

            if (0 == iPackage || m_cPackages < iPackage)
            {
                hr = E_INVALIDARG;
                ExitOnRootFailure(hr, "Invalid package index in elevated information.");
            }

            this->GetPackage(iPackage, &pPackage);

            hr = pPackage->DeserializeElevatedState(pbBuffer, cbElevatedPlan, &iBuffer);
            ExitOnFailure(hr, "Failed to deserialize package elevated information.");

            ReleaseNullObject(pPackage);
        }

    LExit:
        ReleaseBuffer(pbB);
        ReleaseObject(pPackage);
        return hr;
    }


public: // IBurnModelTestable
    STDMETHODIMP AddPackageForTestingPurposes(
        __in IBurnPackage* pPackage
        )
    {
        HRESULT hr = S_OK;

        // TODO: fix this since it can easily leak memory.
        ++m_cPackages;
        m_rgpPackages = static_cast<IBurnPackage**>(MemAlloc(sizeof(IBurnPackage*) * m_cPackages, TRUE));
        ExitOnNull(m_rgpPackages, hr, E_OUTOFMEMORY, "Failed to allocate packages.");

        pPackage->SetIndex(m_cPackages);

        pPackage->AddRef();
        m_rgpPackages[m_cPackages - 1] = pPackage;

    LExit:
        return hr;
    }


public: // internals
    //
    // Initialize - process the provided command line arguments.
    //
    HRESULT Initialize(
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
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"l", -1) ||
                    CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"log", -1))
                {
                    if (i + 1 >= argc)
                    {
                        ExitOnRootFailure(hr = E_INVALIDARG, "Must specify a path for log.");
                    }

                    ++i;

                    hr = StrAllocString(&m_sczLogFile, argv[i], 0);
                    ExitOnFailure(hr, "Failed to copy log file path.");
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"help", -1))
                {
                    m_command.action = BURN_ACTION_HELP;
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"quiet", -1))
                {
                    m_command.display = BURN_DISPLAY_NONE;

                    if (BURN_RESTART_UNKNOWN == m_command.restart)
                    {
                        m_command.restart = BURN_RESTART_AUTOMATIC;
                    }
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"passive", -1))
                {
                    m_command.display = BURN_DISPLAY_PASSIVE;

                    if (BURN_RESTART_UNKNOWN == m_command.restart)
                    {
                        m_command.restart = BURN_RESTART_AUTOMATIC;
                    }
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"norestart", -1))
                {
                    m_command.restart = BURN_RESTART_NEVER;
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"forcerestart", -1))
                {
                    m_command.restart = BURN_RESTART_ALWAYS;
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"promptrestart", -1))
                {
                    m_command.restart = BURN_RESTART_PROMPT;
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"uninstall", -1))
                {
                    if (BURN_ACTION_HELP != m_command.action)
                    {
                        m_command.action = BURN_ACTION_UNINSTALL;
                    }
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"package", -1) ||
                         CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"update", -1))
                {
                    if (BURN_ACTION_UNKNOWN == m_command.action)
                    {
                        m_command.action = BURN_ACTION_INSTALL;
                    }
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"burn.resumed", -1))
                {
                    if (i + 2 >= argc)
                    {
                        ExitOnRootFailure(hr = E_INVALIDARG, "Expected path to persisted properties.");
                    }

                    ++i;

                    hr = RegistrationLoadResume(this, argv[i]);
                    ExitOnFailure1(hr, "Failed to load resume file: %S", argv[i]);

                    m_command.fResumed = TRUE;
                }
                else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"burn.elevated", -1))
                {
                    if (i + 2 >= argc)
                    {
                        ExitOnRootFailure(hr = E_INVALIDARG, "Must specify the parent and child elevation tokens.");
                    }

                    m_mode = BURN_MODE_ELEVATED;

                    ++i;

                    hr = StrAllocString(&m_sczElevatedPipeName, argv[i], 0);
                    ExitOnFailure(hr, "Failed to copy elevated pipe name.");

                    ++i;

                    hr = StrAllocString(&m_sczElevatedToken, argv[i], 0);
                    ExitOnFailure(hr, "Failed to copy elevation token.");
                }
                else
                {
                    // Remember command-line switch to pass off to UX.
                    hr = StrAllocConcat(&m_sczCommandLine, &argv[i][0], 0);
                    ExitOnFailure(hr, "Failed to copy command line parameter.");

                    hr = StrAllocConcat(&m_sczCommandLine, L" ", 0);
                    ExitOnFailure(hr, "Failed to copy space.");
                }
            }
            else
            {
                // if it looks like PROPERTY=VALUE, store it as a global property.
                LPCWSTR wzEquals = wcschr(argv[i], '=');
                if (wzEquals)
                {
                    hr = StrAllocString(&sczName, argv[i], wzEquals - argv[i]);
                    ExitOnFailure(hr, "Failed to extract command-line property name.");

                    hr = PropertySetString(&this->m_properties, sczName, wzEquals + 1);
                    ExitOnFailure(hr, "Failed to set command-line property value.");

                    ReleaseNullStr(sczName);
                }
                else
                {
                    // Remember command-line switch to pass off to UX.
                    hr = StrAllocConcat(&m_sczCommandLine, argv[i], 0);
                    ExitOnFailure(hr, "Failed to copy command line parameter.");

                    hr = StrAllocConcat(&m_sczCommandLine, L" ", 0);
                    ExitOnFailure(hr, "Failed to copy space.");
                }
            }
        }

        // Set the defaults if nothing was set above.
        if (BURN_ACTION_UNKNOWN == m_command.action)
        {
            m_command.action = BURN_ACTION_INSTALL;
        }

        if (BURN_DISPLAY_UNKNOWN == m_command.display)
        {
            m_command.display = BURN_DISPLAY_FULL;
        }

        if (BURN_RESTART_UNKNOWN == m_command.restart)
        {
            m_command.restart = BURN_RESTART_PROMPT;
        }

        Trace3(REPORT_STANDARD, "CBurnModel::Initialize() - Command action = %d, display = %d, restart = %d", m_command.action, m_command.display, m_command.restart);

        // initialize built-in properties
        hr = PropertyInitializeBuiltIn(&this->m_properties);
        ExitOnFailure(hr, "Failed to initialize built-in properties.");

    LExit:
        ReleaseStr(sczName);

        ::LocalFree(argv);

        return hr;
    }

public:
    //
    // Constructor - intitialize member variables.
    //
    CBurnModel()
    {
        m_mode = BURN_MODE_NORMAL;
        m_sczElevatedPipeName = NULL;
        m_sczElevatedToken = NULL;

        memset(&m_command, 0, sizeof(m_command));
        m_sczCommandLine = NULL;
        m_sczBundleId = NULL;
        m_sczManifestFile = NULL;
        m_sczLogFile = NULL;

        m_rgpPackages = NULL;
        m_cPackages = 0;
        m_rgpUXPayloads = NULL;
        m_cUXPayloads = 0;

        memset(&this->m_properties, 0, sizeof(BURN_PROPERTIES));
        memset(&this->m_searches, 0, sizeof(BURN_SEARCHES));

        m_pRegistration = NULL;

        m_fPlanned = FALSE;
        m_fRegisterBundlePerMachine = FALSE;
        m_fPlanRegistrationRequired = FALSE;

        m_prgCachePayloads = NULL;
        m_cMaxCachePayloads = 0;
        m_cCachePayloads = 0;

        m_prgExecutePackages = NULL;
        m_cMaxExecutePackages = 0;
        m_cExecutePackages = 0;

        m_pbElevatedPlan = NULL;
        m_cbElevatedPlan = 0;
        m_cElevatedPlanPackages = 0;

        m_fRestartRequested = FALSE;
        m_fRestartAllowed = FALSE;
    }

    //
    // Destructor - release member variables.
    //
    ~CBurnModel()
    {
        ReleaseBuffer(m_pbElevatedPlan);
        ReleaseMem(m_prgExecutePackages);
        ReleaseMem(m_prgCachePayloads);

        if (m_rgpPackages)
        {
            for (DWORD i = 0; i < m_cPackages; ++i)
            {
                ReleaseObject(m_rgpPackages[i]);
            }
            MemFree(m_rgpPackages);
        }
        ReleaseObjectArray(m_rgpUXPayloads, m_cUXPayloads);

        ReleaseStr(m_sczElevatedToken);
        ReleaseStr(m_sczElevatedPipeName);
        ReleaseStr(m_sczLogFile);
        ReleaseStr(m_sczBundleId);
        ReleaseStr(m_sczManifestFile);
        ReleaseStr(m_sczCommandLine);
        PropertiesUninitialize(&this->m_properties);
        SearchesUninitialize(&this->m_searches);
        if (m_pRegistration)
        {
            RegistrationFree(m_pRegistration);
        }
    }

private:
    BURN_MODE m_mode;
    LPWSTR m_sczElevatedPipeName;
    LPWSTR m_sczElevatedToken;

    BURN_COMMAND m_command;
    LPWSTR m_sczCommandLine;
    LPWSTR m_sczLogFile;
    LPWSTR m_sczBundleId;
    LPWSTR m_sczManifestFile;

    IBurnPackage** m_rgpPackages;
    DWORD m_cPackages;

    IBurnPayload** m_rgpUXPayloads;
    DWORD m_cUXPayloads;

    BURN_PROPERTIES m_properties;
    BURN_SEARCHES m_searches;

    BURN_REGISTRATION* m_pRegistration;

    BOOL m_fPlanned;
    BOOL m_fRegisterBundlePerMachine;
    BOOL m_fPlanRegistrationRequired;

    BURN_PLAN_CACHE_PAYLOAD* m_prgCachePayloads;
    DWORD m_cMaxCachePayloads;
    DWORD m_cCachePayloads;

    BURN_PLAN_EXECUTE_PACKAGE* m_prgExecutePackages;
    DWORD m_cMaxExecutePackages;
    DWORD m_cExecutePackages;

    BYTE* m_pbElevatedPlan;
    SIZE_T m_cbElevatedPlan;
    DWORD m_cElevatedPlanPackages;

    BOOL m_fRestartRequested;
    BOOL m_fRestartAllowed;
};


//
// BurnCreateModel - creates a new IBurnModel object.
//
HRESULT BurnCreateModel(
    __in_z LPCWSTR wzCommandLine,
    __out IBurnModel **ppModel
    )
{
    HRESULT hr = S_OK;

    CBurnModel* pModel = new CBurnModel();
    ExitOnNull(pModel, hr, E_OUTOFMEMORY, "Failed to create new model object.");

    hr = pModel->Initialize(wzCommandLine);
    ExitOnFailure(hr, "Failed to initialize command line object.");

    *ppModel = pModel;
    pModel = NULL;

LExit:
    ReleaseObject(pModel);
    return hr;
}
