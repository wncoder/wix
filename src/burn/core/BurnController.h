//-------------------------------------------------------------------------------------------------
// <copyright file="BurnController.h" company="Microsoft">
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
//    This class manages implements IBurnCore. Uses CBurnView to communicate to BurnUX
//    Uses IProvideDataToUi and INotifyController to perform engine actions.
//    Sequential download of payload and execution of the packages is supported now.
//    Simultaneous download and execution will be supported soon.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "IBurnCore.h"
#include "interfaces\IDataProviders.h"
#include "interfaces\IController.h"
#include "interfaces\IProgressObserver.h"
#include "BurnView.h"

namespace IronMan
{
    enum WM_BURN
    {
        WM_BURN_FIRST = WM_APP + 0xFFF, // this enum value must always be first.

        WM_BURN_VIEW_INITIALIZED,
        WM_BURN_DETECT,
        WM_BURN_PLAN,
        WM_BURN_APPLY,
        WM_BURN_SHUTDOWN,

        WM_BURN_LAST, // this enum value must always be last.
    };


    class CBurnController :
        public IMarshal,
        public IBurnCore
    {
        IProvideDataToUi& m_engine;
        INotifyController& m_controller;
        CBurnView& m_burnView;
        ILogger& m_logger;
        DWORD m_dwThreadId;
        BURN_ACTION m_action;
        HANDLE m_hServerProcess;
        HANDLE m_hServerPipe;

    public:
        ~CBurnController() 
        {
            CloseStore();
        }

        CBurnController(IProvideDataToUi& ipdtUI, INotifyController& controller, CBurnView& burnUX, ILogger& logger)
            : m_engine(ipdtUI)
            , m_controller(controller)
            , m_burnView(burnUX)
            , m_logger(logger)
            , m_dwThreadId(0)
            , m_action(BURN_ACTION_UNKNOWN)
            , m_hServerProcess(NULL)
            , m_hServerPipe(INVALID_HANDLE_VALUE)
        {
        }

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

            if  (__uuidof(IBurnCore) == riid)
            {
                AddRef();
                *ppvObject = reinterpret_cast<LPVOID>(static_cast<IBurnCore*>(this));
            }
            else if (IID_IMarshal == riid)
            {
                AddRef();
                *ppvObject = reinterpret_cast<LPVOID>(static_cast<IMarshal*>(this));
            }
            else if (IID_IUnknown == riid)
            {
                AddRef();
                *ppvObject = reinterpret_cast<LPVOID>(static_cast<LPUNKNOWN>(static_cast<IBurnCore*>(this)));
            }
            else
            {
                hr = E_NOINTERFACE;
            }

            return hr;
        }

        virtual STDMETHODIMP_(ULONG) AddRef()
        {
            // Lifetime management provided by the engine.
            return 1;
        }

        virtual STDMETHODIMP_(ULONG) Release()
        {
            // Lifetime management provided by the engine.
            return 1;
        }

        //
        // IMarshal
        //
        virtual STDMETHODIMP GetUnmarshalClass( 
            __in REFIID riid,
            __in_opt LPVOID pv,
            __in DWORD dwDestContext,
            __reserved LPVOID pvDestContext,
            __in DWORD mshlflags,
            __out LPCLSID pCid)
        {
            return E_NOTIMPL;
        }

        virtual STDMETHODIMP GetMarshalSizeMax( 
            __in REFIID riid,
            __in_opt LPVOID pv,
            __in DWORD dwDestContext,
            __reserved LPVOID pvDestContext,
            __in DWORD mshlflags,
            __out DWORD *pSize)
        {
            HRESULT hr = S_OK;

            // We only support marshaling the engine in-proc.
            if (__uuidof(IBurnCore) != riid)
            {
                hr = E_NOINTERFACE; 
                ExitOnFailure(hr, "Not marshaling the IBurnCore interface.");
            }

            if (0 == (MSHCTX_INPROC & dwDestContext))
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Not marshaling the IBurnCore interface in-proc.");
            }

            // E_INVALIDARG is not a supported return value.
            ExitOnNull(pSize, hr, E_FAIL, "The size output parameter is NULL.");

            // Specify enough size to marshal just the interface pointer across threads.
            *pSize = sizeof(LPVOID);

        LExit:
            return hr;
        }
        
        virtual STDMETHODIMP MarshalInterface( 
            __in IStream *pStm,
            __in REFIID riid,
            __in_opt LPVOID pv,
            __in DWORD dwDestContext,
            __reserved LPVOID pvDestContext,
            __in DWORD mshlflags)
        {
            HRESULT hr = S_OK;
            IBurnCore *pThis = NULL;
            ULONG ulWritten = 0;

            // We only support marshaling the engine in-proc.
            if (__uuidof(IBurnCore) != riid)
            {
                hr = E_NOINTERFACE;
                ExitOnFailure(hr, "Not marshaling the IBurnCore interface.");
            }

            if (0 == (MSHCTX_INPROC & dwDestContext))
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Not marshaling the IBurnCore interface in-proc.");
            }

            // "pv" may not be set, so we should us "this" otherwise.
            if (pv)
            {
                pThis = reinterpret_cast<IBurnCore*>(pv);
            }
            else
            {
                pThis = static_cast<IBurnCore*>(this);
            }

            // E_INVALIDARG is not a supported return value.
            ExitOnNull(pStm, hr, E_FAIL, "The marshaling stream parameter is NULL.");

            // Marshal the interface pointer in-proc as is.
            hr = pStm->Write(pThis, sizeof(pThis), &ulWritten);
            if (STG_E_MEDIUMFULL == hr)
            {
                ExitOnFailure(hr, "Failed to write the stream because the stream is full.");
            }
            else if (FAILED(hr))
            {
                // Any other STG error cannot be returned as-is based on IMarshal documentation.
                hr = E_FAIL;
                ExitOnFailure(hr, "Failed to write the IBurnCore interface pointer to the marshaling stream.");
            }

        LExit:
            return hr;
        }
        
        virtual STDMETHODIMP UnmarshalInterface( 
            __in IStream *pStm,
            __in REFIID riid,
            __deref_out LPVOID *ppv)
        {
            HRESULT hr = S_OK;
            ULONG ulRead = 0;

            // We only support marshaling the engine in-proc.
            if (__uuidof(IBurnCore) != riid)
            {
                hr = E_NOINTERFACE;
                ExitOnFailure(hr, "Not unmarshaling the IBurnCore interface.");
            }

            // E_INVALIDARG is not a supported return value.
            ExitOnNull(pStm, hr, E_FAIL, "The marshaling stream parameter is NULL.");
            ExitOnNull(ppv, hr, E_FAIL, "The interface output parameter is NULL.");

            // Unmarshal the interface pointer in-proc as is.
            hr = pStm->Read(*ppv, sizeof(LPVOID), &ulRead);
            if (FAILED(hr))
            {
                // Any STG error cannot be returned as-is based on IMarshal documentation.
                hr = E_FAIL;
                ExitOnFailure(hr, "Failed to read the IBurnCore interface pointer from the marshaling stream.");
            }

        LExit:
            return hr;
        }
        
        virtual STDMETHODIMP ReleaseMarshalData( 
            __in IStream *pStm)
        {
            return E_NOTIMPL;
        }
        
        virtual STDMETHODIMP DisconnectObject( 
            __in  DWORD dwReserved)
        {
            return E_NOTIMPL;
        }

        //
        // IBurnCore
        //
        virtual STDMETHODIMP GetPackageCount(
            __out DWORD* pcPackages
            )
        {
            if (pcPackages)
                *pcPackages = m_engine.GetAuthoredItemCount();
            return S_OK;
        }

        // Get the command line parameters not pertinent to the engine
        virtual STDMETHODIMP GetCommandLineParameters(
            __out_ecount_opt(*pcchCommandLine) LPWSTR psczCommandLine,
            __inout DWORD* pcchCommandLine
            )
        {
            HRESULT hr = S_OK;
            IronMan::CCmdLineSwitches switches;
            CSimpleArray<CString> rgstrAdditionalSwitches; // Deliberately empty
            CString strUnrecognizedSwitches;

            if (!pcchCommandLine || !*pcchCommandLine)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            switches.AllSwitchesAreValid(rgstrAdditionalSwitches,strUnrecognizedSwitches);

            // Check buffer space allowing for null terminator
            DWORD dwCharactersNotIncludingNulTerminator = strUnrecognizedSwitches.GetLength();

            if ( *pcchCommandLine <= dwCharactersNotIncludingNulTerminator )
            {
                // Return actual character count which can be used to resize buffer
                *pcchCommandLine = dwCharactersNotIncludingNulTerminator;
                hr = E_MOREDATA ;
                ExitOnFailure(hr, "Insufficent buffer passed in for string.");
            }
            else
            {
                // This function requires (dwCharactersNotIncludingNulTerminator + 1) characters
                hr = StringCchCopyW(psczCommandLine, *pcchCommandLine, strUnrecognizedSwitches.GetBuffer());
                ExitOnFailure(hr, "Failed to copy string.");
            }

            // Return actual character count
            *pcchCommandLine = dwCharactersNotIncludingNulTerminator;

        LExit:
            return hr;
        }

        virtual STDMETHODIMP GetVariableNumeric(
            __in_z LPCWSTR wzVariable,
            __out LONGLONG* pllValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable || !pllValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable
            hr = VariableGetNumeric(&vpEngineState->variables, wzVariable, pllValue);
            ExitOnFailure(hr, "Failed to get numeric variable.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }

        virtual STDMETHODIMP GetVariableString(
            __in_z LPCWSTR wzVariable,
            __out_ecount_opt(*pcchValue) LPWSTR wzValue,
            __inout DWORD* pcchValue
            )
        {
            HRESULT hr = S_OK;
            DWORD cch = 0;
            LPWSTR scz = NULL;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable || !pcchValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable
            hr = VariableGetString(&vpEngineState->variables, wzVariable, &scz);
            ExitOnFailure(hr, "Failed to get string variable.");

            // copy to output buffer
            cch = lstrlenW(scz);
            if (cch < *pcchValue) // note that cch actually has to be less, to account for the null terminator
            {
                if (wzValue)
                {
                    hr = StringCchCopyW(wzValue, *pcchValue, scz);
                    ExitOnFailure(hr, "Failed to copy string.");
                }
            }
            else
            {
                hr = E_MOREDATA;
            }

            // return character count
            *pcchValue = cch;

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            ReleaseStr(scz);

            return hr;
        }

        virtual STDMETHODIMP GetVariableVersion(
            __in_z LPCWSTR wzVariable,
            __out DWORD64* pqwValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable || !pqwValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable
            hr = VariableGetVersion(&vpEngineState->variables, wzVariable, pqwValue);
            ExitOnFailure(hr, "Failed to get version variable.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }


        /*******************************************************************
        OpenStore - Called from UX to open variable store of given bundle.
                    Only one store can be open at any time.
        *******************************************************************/
        virtual HRESULT __stdcall OpenStore(
            __in_z LPCWSTR wzBundleId
            )
        {
            HRESULT hr = S_OK;
        //    LPWSTR sczExecutable = NULL;
        //    LPWSTR sczPipeName = NULL;
        //    LPWSTR sczClientToken = NULL;

        //    if (!wzBundleId || !*wzBundleId)
        //    {
        //        ExitFunction1(hr = E_INVALIDARG);
        //    }

        //    LogStringLine(REPORT_VERBOSE, "OpenStore: '%ls'", wzBundleId);

        //    if (m_hServerProcess)
        //    {
        //        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
        //        ExitOnRootFailure(hr, "A store is already opened.");
        //    }

        //    // Get the path to the bundle exe
        //    // Only works within Families
        //    hr = E_INVALIDARG;  // if BundleId not found
        //    for (DWORD i = 0; i < vpEngineState->registration.cRegdataFamily; ++i)
        //    {
        //        if ( CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, vpEngineState->registration.rgRegdataFamily[i].sczBundleId, -1, wzBundleId, -1) )
        //        {
        //            // Found Match
        //            hr = StrAllocString(&sczExecutable, vpEngineState->registration.rgRegdataFamily[i].sczExecutablePath, 0);
        //            ExitOnFailure(hr, "Unable to allocate memory for path");
        //            break;
        //        }
        //    }
        //    ExitOnFailure(hr, "BundleId not found in Bundles of same Family being replaced.");

        //    // Launch executable with BurnServer switch and connect
        //    hr = StrAllocString(&sczPipeName,L"BurnServerPipe",0);
        //    ExitOnFailure(hr, "Failed to copy pipe name.");

        //    hr = StrAllocString(&sczClientToken, L"{D3D8CA1B-045B-4632-BBE0-1083D6F05984}", 0);
        //    ExitOnFailure(hr, "Failed to copy pipe token.");

        //    hr = ParentProcessConnect(NULL, sczExecutable, L"server", sczPipeName, sczClientToken, &m_hServerProcess, &m_hServerPipe);
        //    ExitOnFailure(hr, "Failed to create server process.");

        //    ::LogStringLine(REPORT_VERBOSE, "Successfully opened variable store.");
 
        //LExit:
        //    if (FAILED(hr))
        //    {
        //        ::LogStringLine(REPORT_ERROR, "Failed to open store '%ls'.", wzBundleId);
        //        CloseStore();
        //    }

        //    ReleaseStr(sczExecutable);
        //    ReleaseStr(sczPipeName);
        //    ReleaseStr(sczClientToken);

            return hr;
        }

        /*******************************************************************
        GetPriorVariableNumeric - Called from UX to get numeric value from 
                                  variable store opend by OpenStore call.
        *******************************************************************/
        virtual HRESULT __stdcall GetPriorVariableNumeric(
            __in_z LPCWSTR wzVariable,
            __out LONGLONG* pllValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;
            BYTE* pbData = NULL;
            SIZE_T cbData = 0;
            BURN_ELEVATION_MESSAGE msg = { };
            SIZE_T iData = 0;

            if (!wzVariable || !*wzVariable || !pllValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            LogStringLine(REPORT_VERBOSE, "GetPriorVariableNumeric: '%ls'", wzVariable);
            ExitOnNull(m_hServerProcess, hr, E_UNEXPECTED, "OpenStore needs to be called successfully before PriorVariableGetNumeric is called.");

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable

            // serialize message data
            hr = BuffWriteString(&pbData, &cbData, wzVariable);
            ExitOnFailure(hr, "Failed to copy variable.");

            // send message
            DWORD dwResult = -1;
            hr = ElevationPostMessage(m_hServerPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLENUMERIC, pbData, cbData);
            ExitOnFailure(hr, "Failed to send BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLENUMERIC message to server process.");

            // pump messages from server process
            while (S_OK == (hr = ElevationGetMessage(m_hServerPipe, &msg)))
            {
                // Process the message.
                switch (msg.dwMessage)
                {
                case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT:
                    iData = 0;
                    hr = BuffReadNumber64((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD64*)pllValue);
                    ExitOnFailure(hr, "Failed to read value.");
                    break;

                case BURN_PIPE_MESSAGE_TYPE_COMPLETE:
                    if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                    {
                        hr = E_INVALIDARG;
                        ExitOnFailure(hr, "Invalid data for complete message.");
                    }
                    ExitFunction1(hr = *(HRESULT*)msg.pvData);
                    break;

                default:
                    hr = E_INVALIDARG;
                    ExitOnRootFailure1(hr, "Unexpected message sent to client process, msg: %u", msg.dwMessage);
                }

                ElevationMessageUninitialize(&msg);
            }

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            ElevationMessageUninitialize(&msg);
            LogStringLine(REPORT_VERBOSE, "GetPriorVariableNumeric: '%ls' Value '%lld'", wzVariable, *pllValue);
            ReleaseBuffer(pbData);
            return hr;
        }

        /*******************************************************************
        GetPriorVariableString - Called from UX to get string value from 
                                  variable store opend by OpenStore call.
        *******************************************************************/
        virtual HRESULT __stdcall GetPriorVariableString(
            __in_z LPCWSTR wzVariable,
            __out_ecount_opt(*pcchValue) LPWSTR wzValue,
            __inout DWORD* pcchValue
            )
        {
            HRESULT hr = S_OK;
            DWORD cch = 0;
            LPWSTR scz = NULL;
            BOOL fEnteredCriticalSection = FALSE;
            BYTE* pbData = NULL;
            SIZE_T cbData = 0;
            BURN_ELEVATION_MESSAGE msg = { };
            SIZE_T iData = 0;


            if (!wzVariable || !*wzVariable || !pcchValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            LogStringLine(REPORT_VERBOSE, "GetPriorVariableString: '%ls'", wzVariable);
            ExitOnNull(m_hServerProcess, hr, E_UNEXPECTED, "OpenStore needs to be called successfully before GetPriorVariableString is called.");

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable
            // serialize message data
            hr = BuffWriteString(&pbData, &cbData, wzVariable);
            ExitOnFailure(hr, "Failed to copy variable.");

            // send message
            DWORD dwResult = -1;
            hr = ElevationPostMessage(m_hServerPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLESTRING, pbData, cbData);
            ExitOnFailure(hr, "Failed to send BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLESTRING message to server process.");

            // pump messages from server process
            while (S_OK == (hr = ElevationGetMessage(m_hServerPipe, &msg)))
            {
                // Process the message.
                switch (msg.dwMessage)
                {
                case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT:
                    iData = 0;
                    hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &scz);
                    ExitOnFailure(hr, "Failed to read string value.");

                    // copy to output buffer
                    cch = lstrlenW(scz);
                    if (cch < *pcchValue) // note that cch actually has to be less, to account for the null terminator
                    {
                        if (wzValue)
                        {
                            hr = StringCchCopyW(wzValue, *pcchValue, scz);
                            ExitOnFailure(hr, "Failed to copy string.");
                        }
                    }
                    else
                    {
                        hr = E_MOREDATA;
                    }

                    // return character count
                    *pcchValue = cch;
                    break;

                case BURN_PIPE_MESSAGE_TYPE_COMPLETE:
                    if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                    {
                        hr = E_INVALIDARG;
                        ExitOnFailure(hr, "Invalid data for complete message.");
                    }
                    ExitFunction1(hr = *(HRESULT*)msg.pvData);
                    break;

                default:
                    hr = E_INVALIDARG;
                    ExitOnRootFailure1(hr, "Unexpected message sent to client process, msg: %u", msg.dwMessage);
                }

                ElevationMessageUninitialize(&msg);
            }

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            ElevationMessageUninitialize(&msg);
            LogStringLine(REPORT_VERBOSE, "GetPriorVariableString: '%ls' Value '%ls'", wzVariable, wzValue);
            ReleaseStr(scz);
            ReleaseBuffer(pbData);
            return hr;
        }

        /*******************************************************************
        GetPriorVariableVersion - Called from UX to get version value from 
                                  variable store opend by OpenStore call.
        *******************************************************************/
        virtual HRESULT __stdcall GetPriorVariableVersion(
            __in_z LPCWSTR wzVariable,
            __out DWORD64* pqwValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;
            BYTE* pbData = NULL;
            SIZE_T cbData = 0;
            BURN_ELEVATION_MESSAGE msg = { };
            SIZE_T iData = 0;

            if (!wzVariable || !*wzVariable || !pqwValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            LogStringLine(REPORT_VERBOSE, "GetPriorVariableVersion: '%ls'", wzVariable);
            ExitOnNull(m_hServerProcess, hr, E_UNEXPECTED, "OpenStore needs to be called successfully before GetPriorVariableVersion is called.");

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // get variable

            // serialize message data
            hr = BuffWriteString(&pbData, &cbData, wzVariable);
            ExitOnFailure(hr, "Failed to copy variable.");

            // send message
            DWORD dwResult = -1;
            hr = ElevationPostMessage(m_hServerPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLEVERSION, pbData, cbData);
            ExitOnFailure(hr, "Failed to send BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLEVERSION message to server process.");

            // pump messages from server process
            while (S_OK == (hr = ElevationGetMessage(m_hServerPipe, &msg)))
            {
                // Process the message.
                switch (msg.dwMessage)
                {
                case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT:
                    iData = 0;
                    hr = BuffReadNumber64((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD64*)pqwValue);
                    ExitOnFailure(hr, "Failed to read value.");
                    break;

                case BURN_PIPE_MESSAGE_TYPE_COMPLETE:
                    if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                    {
                        hr = E_INVALIDARG;
                        ExitOnFailure(hr, "Invalid data for complete message.");
                    }
                    ExitFunction1(hr = *(HRESULT*)msg.pvData);
                    break;

                default:
                    hr = E_INVALIDARG;
                    ExitOnRootFailure1(hr, "Unexpected message sent to client process, msg: %u", msg.dwMessage);
                }

                ElevationMessageUninitialize(&msg);
            }

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            ElevationMessageUninitialize(&msg);
            LogStringLine(REPORT_VERBOSE, "GetPriorVariableVersion: '%ls' Value '%hu.%hu.%hu.%hu'", wzVariable, (WORD)(*pqwValue >> 48), (WORD)(*pqwValue >> 32), (WORD)(*pqwValue >> 16), (WORD)(*pqwValue));
            ReleaseBuffer(pbData);
            return hr;
        }

        virtual void __stdcall CloseStore(
            )
        {
            ::LogStringLine(REPORT_VERBOSE, "CloseStore()");
            // Post terminate message over the pipe
            if (m_hServerPipe)
            {
                ElevationParentProcessTerminate(m_hServerProcess, m_hServerPipe);

                ::CloseHandle(m_hServerProcess);
                m_hServerProcess = NULL;

                ::CloseHandle(m_hServerPipe);
                m_hServerPipe = INVALID_HANDLE_VALUE;
                ::LogStringLine(REPORT_VERBOSE, "CloseStore() suceeded.");
            }
            else
            {
                ::LogStringLine(REPORT_DEBUG, "No open store to close.");
            }
        }


        virtual STDMETHODIMP SetVariableNumeric(
            __in_z LPCWSTR wzVariable,
            __in LONGLONG llValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // set variable
            hr = VariableSetNumeric(&vpEngineState->variables, wzVariable, llValue);
            ExitOnFailure(hr, "Failed to set numeric variable.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }

        virtual STDMETHODIMP SetVariableString(
            __in_z LPCWSTR wzVariable,
            __in_z LPCWSTR wzValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable || !wzValue)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // set variable
            hr = VariableSetString(&vpEngineState->variables, wzVariable, wzValue);
            ExitOnFailure(hr, "Failed to set string variable.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }

        virtual STDMETHODIMP SetVariableVersion(
            __in_z LPCWSTR wzVariable,
            __in DWORD64 qwValue
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzVariable || !*wzVariable)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // set variable
            hr = VariableSetVersion(&vpEngineState->variables, wzVariable, qwValue);
            ExitOnFailure(hr, "Failed to set version variable.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }

        virtual STDMETHODIMP FormatString(
            __in_z LPCWSTR wzIn,
            __out_ecount_opt(*pcchOut) LPWSTR wzOut,
            __inout DWORD* pcchOut
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;
            DWORD cch = 0;
            LPWSTR scz = NULL;

            if (!wzIn || !pcchOut)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // do the actual formatting
            hr = VariableFormatString(&vpEngineState->variables, wzIn, wzOut ? &scz : NULL, &cch);
            ExitOnFailure(hr, "Failed to format string.");

            // copy to output buffer
            if (cch < *pcchOut) // note that cch actually has to be less, to account for the null terminator
            {
                if (wzOut)
                {
                    hr = StringCchCopyW(wzOut, *pcchOut, scz);
                    ExitOnFailure(hr, "Failed to copy string.");
                }
            }
            else
            {
                hr = E_MOREDATA;
            }

            // return character count
            *pcchOut = cch;

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            ReleaseStr(scz);

            return hr;
        }

        virtual STDMETHODIMP EscapeString(
            __in_z LPCWSTR wzIn,
            __out_ecount_opt(*pcchOut) LPWSTR wzOut,
            __inout DWORD* pcchOut
            )
        {
            HRESULT hr = S_OK;
            DWORD cch = 0;
            LPWSTR scz = NULL;

            // escape string
            hr = VariableEscapeString(wzIn, &scz);
            ExitOnFailure(hr, "Failed to escape string.");

            // copy to output buffer
            cch = lstrlenW(scz);
            if (cch < *pcchOut) // note that cch actually has to be less, to account for the null terminator
            {
                if (wzOut)
                {
                    hr = StringCchCopyW(wzOut, *pcchOut, scz);
                    ExitOnFailure(hr, "Failed to copy string.");
                }
            }
            else
            {
                hr = E_MOREDATA;
            }

            // return character count
            *pcchOut = cch;

        LExit:
            ReleaseStr(scz);

            return hr;
        }

        virtual STDMETHODIMP EvaluateCondition(
            __in_z LPCWSTR wzCondition,
            __out BOOL* pf
            )
        {
            HRESULT hr = S_OK;
            BOOL fEnteredCriticalSection = FALSE;

            if (!wzCondition || !pf)
            {
                ExitFunction1(hr = E_INVALIDARG);
            }

            // enter engine activity critical section
            ::EnterCriticalSection(&vpEngineState->csActive);
            fEnteredCriticalSection = TRUE;

            if (vpEngineState->fActive)
            {
                ExitFunction1(hr = E_INVALIDSTATE);
            }

            // evaluate condition
            hr = ConditionEvaluate(&vpEngineState->variables, wzCondition, pf);
            ExitOnFailure(hr, "Failed to evaluate condition.");

        LExit:
            if (fEnteredCriticalSection)
            {
                ::LeaveCriticalSection(&vpEngineState->csActive);
            }

            return hr;
        }

        virtual STDMETHODIMP Log(
            __in BURN_LOG_LEVEL level,
            __in_z LPCWSTR wzMessage
            )
        {
            HRESULT hr = S_OK;
            ILogger::LoggingLevel internalLevel;

            switch (level)
            {
            case BURN_LOG_LEVEL_STANDARD:
                internalLevel = ILogger::Information;
                break;
            case BURN_LOG_LEVEL_VERBOSE:
                internalLevel = ILogger::Verbose;
                break;
            case BURN_LOG_LEVEL_DEBUG:
                internalLevel = ILogger::Debug;
                break;
            case BURN_LOG_LEVEL_ERROR:
                internalLevel = ILogger::Error;
                break;
            default:
                ExitFunction1(hr = E_INVALIDARG);
            }

            this->m_logger.Log(internalLevel, wzMessage);
        LExit:
            return hr;
        }

        //
        // Elevate - Creates elevated process and handles user cancellation with retry option.
        //
        virtual STDMETHODIMP Elevate(
            __in_opt HWND hwndParent
            )
        {
            HRESULT hr = S_OK;
            int nResult = IDOK;
            BOOL fIsAlreadyElevated = FALSE;
            IronMan::CCmdLineSwitches switches;

            // If already elevated then no UAC prompt will occur so even quiet mode can continue, e.g. if being run from Windows Update
            hr = OsIsRunningPrivileged(&fIsAlreadyElevated);
            ExitOnFailure(hr, "Failed to obtain elevation state.");

            // Set appropriate flags for silent, passive and interactive modes
            if (!fIsAlreadyElevated && switches.QuietMode() )
            {
                hr = HRESULT_FROM_WIN32(ERROR_ELEVATION_REQUIRED);
                LOG(m_logger, ILogger::Error, L"Operation requires elevation but mode is quiet. Please re-run in interactive mode.");
                ExitOnRootFailure(hr, "Operation requires elevation but mode is quiet. Please re-run in interactive mode.");
            }

            // check if per-machine process has already been created
            if (vpEngineState->hElevatedProcess || INVALID_HANDLE_VALUE != vpEngineState->hElevatedPipe)
            {
                hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
                ExitOnRootFailure(hr, "Already launched elevated process and/or created elevated pipe.");
            }

            do
            {
                nResult = IDOK;
                hr = ElevationParentProcessConnect(hwndParent, &vpEngineState->hElevatedProcess, &vpEngineState->hElevatedPipe);

                if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
                {
                    nResult = m_burnView.OnError(NULL, ERROR_CANCELLED, NULL, MB_ICONERROR | MB_RETRYCANCEL);
                }

            } while (IDRETRY == nResult && !SUCCEEDED(hr));

            ExitOnFailure(hr, "Failed to connect to elevated process.");

        LExit:
            return hr;
        }

        //
        // Detect - called from UX to determine applicable packages (items)..
        //
        virtual STDMETHODIMP Detect()
        {
            HRESULT hr = S_OK;

            LOG(m_logger, ILogger::Information, L"CBurnController::Detect()");

            if (!::PostThreadMessageW(m_dwThreadId, WM_BURN_DETECT, 0, 0))
            {
                ExitWithLastError(hr, "Failed to post detect message.");
            }

        LExit:
            return hr;
        }

        //
        // Plan
        //
        virtual STDMETHODIMP Plan(
            __in BURN_ACTION action
            )
        {
            HRESULT hr = S_OK;

            LOG(m_logger, ILogger::Information, L"CBurnController::Plan()");

            if (!::PostThreadMessageW(m_dwThreadId, WM_BURN_PLAN, 0, action))
            {
                ExitWithLastError(hr, "Failed to post plan message.");
            }

        LExit:
            return hr;
        }

        //
        // OnApply - called from UX to start applying (cache and execute) packages.
        //
        virtual STDMETHODIMP Apply( __in_opt HWND hwndParent )
        {
            HRESULT hr = S_OK;

            LOG(m_logger, ILogger::Information, L"CBurnController::Apply()");

            if (!::PostThreadMessageW(m_dwThreadId, WM_BURN_APPLY, 0, reinterpret_cast<LPARAM>(hwndParent)))
            {
                ExitWithLastError(hr, "Failed to post apply message.");
            }

        LExit:
            return hr;
        }

        virtual STDMETHODIMP Suspend()
        {
            HRESULT hr = S_OK;

            LOG(m_logger, ILogger::Information, L"CBurnController::Suspend()");

            vpEngineState->fSuspend = TRUE;
            m_controller.Stop();

        LExit:
            return hr;
        }

        virtual STDMETHODIMP Reboot()
        {
            HRESULT hr = S_OK;
            LOG(m_logger, ILogger::Information, L"CBurnController::Reboot()");

            vpEngineState->fForcedReboot = TRUE;
            m_controller.Stop();

        LExit:
            return hr;
        }

        virtual STDMETHODIMP Shutdown(
            __in DWORD dwExitCode
            )
        {
            HRESULT hr = S_OK;

            if (!::PostThreadMessageW(m_dwThreadId, WM_BURN_SHUTDOWN, 0, (LPARAM)dwExitCode))
            {
                ExitWithLastError(hr, "Failed to post shutdown message.");
            }

        LExit:
            return hr;
        }

        //
        // Called by UX to set new source location
        //
        virtual STDMETHODIMP SetSource(__in LPCWSTR wzSourcePath)
        {
            return m_engine.SetSource(wzSourcePath);
        }

        //
        // RunViewUX Launch UI Thread and pump messages for worker thread.
        //
        HRESULT RunViewUX()
        {
            HRESULT hr = S_OK;
            BOOL fRet = FALSE;
            MSG msg = { };

            // Initialize this thread's message queue and create the UI thread.
            IMASSERT(0 == m_dwThreadId);
            m_dwThreadId = ::GetCurrentThreadId();
            ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

            // initialize UX
            hr = m_burnView.Initialize(this, 0, vResumeType);
            ExitOnFailure(hr, "Failed to initialize view.");

            // Message pump.
            while (0 != (fRet = ::GetMessageW(&msg, NULL, WM_BURN_FIRST, WM_BURN_LAST)))
            {
                if (-1 == fRet)
                {
                    hr = E_UNEXPECTED;
                    ExitOnFailure(hr, "Unexpected return value from message pump.");
                }
                else
                {
                    ProcessMessage(&msg);
                }
            }

            // get exit code
            vpEngineState->dwExitCode = (DWORD)msg.wParam;

        LExit:
            m_burnView.Uninitialize();

            m_dwThreadId = 0;
            return hr;
        }


        // Abort the download and install performers.
        virtual void Abort()
        {
            m_controller.Abort();
        }

        //
        // GetBurnCommand Information on action, display mode and restart mode
        //
        static BURN_COMMAND GetBurnCommand(Operation::euiOperation operation)
        {
            BURN_COMMAND command = { };
            IronMan::CCmdLineSwitches switches;

            // Set appropriateflags for silent, passive and interactive modes
            if (switches.QuietMode())
            {
                command.display = BURN_DISPLAY_NONE;
            }
            else if (switches.PassiveMode())
            {
                command.display = BURN_DISPLAY_PASSIVE;
            }
            else
            {
                command.display = BURN_DISPLAY_FULL;
            }

            // Handle /norestart , /promptrestart and /forcerestart
            if (switches.NoRestartMode())
            {
                // If any of the patches required reboot, setup should neither prompt nor cause a reboot
                command.restart = BURN_RESTART_NEVER;
            }
            else if (switches.PromptForRestartMode())
            {
                // If any of the patches required reboot, setup should prompt for reboot after install, and trigger reboot if user agrees
                command.restart = BURN_RESTART_PROMPT;
            }
            else if (switches.ForceRestartMode())
            {
                // Setup should force a restart without asking on completion, irrespective of whether it was required by any packages
                command.restart = BURN_RESTART_ALWAYS;
            }
            else
            {
                // If any of the patches required reboot, setup should restart after presenting a dialog box with a timer warning the user that the computer will restart in x seconds.
                command.restart = BURN_RESTART_AUTOMATIC;
            }

            // Get the action, e.g. BURN_ACTION_UNINSTALL
            command.action = GetBurnAction(operation);

            return command;
        }

    private:

        //
        // GetCorrectError - Returns correct error code when downloader/installer aborts the other
        //
        static HRESULT GetCorrectError(HRESULT hrDownload, HRESULT hrInstall)
        {
            if (SUCCEEDED(hrDownload)) 
            {
                return hrInstall;
            }

            if (SUCCEEDED(hrInstall))
            {
                return hrDownload;
            }

            // At this point, both are errors or aborts

            // However on error, the composite controller aborts the other guy, so...
            if (hrDownload == E_ABORT)
            {
                return hrInstall;
            }

            if (hrInstall == E_ABORT)
            {
                return hrDownload;
            }

            return hrInstall; // both are real errors.  Return the install error
        }

        //
        // RunSimultaneousDownloadAndInstall - Download payload first and then start
        //                                   executing the packages, sequentially.
        HRESULT RunSimultaneousDownloadAndInstall(DWORD itemCount)
        {
            HANDLE hEvents[2];
            hEvents[0] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            hEvents[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

            class ParallelObserver : public IProgressObserver
            {
                HANDLE m_hEvent;
                HRESULT m_hr;
                CBurnView& m_burnView;
                BOOL m_fDownloadObserver;
                unsigned char m_downloadProgress;
                unsigned char m_installProgress;
                unsigned char m_progressDetail;

            public:
                ParallelObserver(HANDLE hEvent, CBurnView& burnView, BOOL fDownloadObserver) 
                    : m_hEvent(hEvent) 
                    , m_hr(E_FAIL)
                    , m_burnView(burnView)
                    , m_fDownloadObserver(fDownloadObserver)
                    , m_downloadProgress (0)
                    , m_installProgress (0)
                    , m_progressDetail (0)
                {}

                HRESULT GetResult() const 
                { 
                    return m_hr; 
                }

            private:
                virtual void OnStateChange(IProgressObserver::State enumVal) {}
                virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) 
                {
                    CString id;
                    CString filename;
                    CString result;
                    int curPos = 0;

                    switch (enumVal)
                    {
                    case IProgressObserver::Downloading:
                        if (changeInfo != L"")
                        {
                            id = changeInfo.Tokenize(L";", curPos);
                            filename = changeInfo.Tokenize(L";", curPos);
                            m_burnView.OnDownloadPayloadBegin(id, filename);
                        }
                        break;
                    case IProgressObserver::DownloadItemComplete:
                        id = changeInfo.Tokenize(L";", curPos);
                        filename = changeInfo.Tokenize(L";", curPos);
                        result = changeInfo.Tokenize(L";", curPos);
                        m_burnView.OnDownloadPayloadComplete(id, filename, _wtoi(result));
                        break;
                    }
                }

                virtual int OnProgress(unsigned char soFar) 
                {
                    int ret = IDOK;

                    if (m_fDownloadObserver)
                    {
                        m_downloadProgress = 100*soFar/255;
                        ret = m_burnView.OnDownloadProgress(m_progressDetail, m_downloadProgress);
                    }
                    else
                    {
                        m_installProgress = 100*soFar/255;
                        ret = m_burnView.OnExecuteProgress(m_progressDetail, m_installProgress);
                    }

                    int progRet = m_burnView.OnProgress(m_progressDetail, (m_downloadProgress + m_installProgress)/2);
                    if (progRet != IDOK)
                    {
                        ret = progRet;
                    }

                    return ret;
                }
                virtual int OnProgressDetail(unsigned char soFar) 
                {
                    m_progressDetail = 100*soFar/255;
                    return IDOK; 
                }

                virtual void Finished(HRESULT hr)
                {
                    if (m_fDownloadObserver)
                    {
                        m_burnView.OnDownloadProgress(m_progressDetail, 100);
                    }
                    m_hr = hr;
                    ::SetEvent(m_hEvent);
                }

                virtual void OnRebootPending()
                {
                }

            } pod(hEvents[0], m_burnView, TRUE) // parallel observer for downloading
                , poi(hEvents[1], m_burnView, FALSE);   // parallel observer for installing

            m_controller.MayBegin(pod);
            m_controller.MayBegin(poi);

            ::WaitForMultipleObjects(2, hEvents, TRUE, INFINITE);
            ::CloseHandle(hEvents[0]);
            ::CloseHandle(hEvents[1]);

            return GetCorrectError(pod.GetResult(), poi.GetResult());
        }

        // RunSequentialDownloadAndInstall
        //
        // Download payload first and then start executing the packages, sequentially.
        HRESULT RunSequentialDownloadAndInstall(DWORD itemCount, bool& bRebootPending)
        {
            HANDLE hEvent = NULL;
            HRESULT hr = E_FAIL;
            int nResult = IDOK;

            hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            DWORD dwError = GetLastError();

            // This class communicates progess to BurnUX using CBurnView
            class SerialObserver : public IProgressObserver
            {
                HANDLE m_hEvent;
                CBurnView& m_burnView;
                HRESULT &m_hr;
                bool &m_bRebootPending;
                bool m_bDownloadFinished;
                unsigned char m_downloadProgress;
                unsigned char m_installProgress;
                unsigned char m_progressDetail;

            public:
                SerialObserver(HANDLE hEvent, CBurnView& burnUXStub,  HRESULT &hr, bool &bRebootPending)
                    : m_hEvent(hEvent)
                    , m_burnView(burnUXStub)
                    , m_hr(hr) 
                    , m_bRebootPending(bRebootPending)
                    , m_bDownloadFinished(false)
                    , m_downloadProgress (0)
                    , m_installProgress (0)
                    , m_progressDetail (0)
                {}
                HRESULT GetResult() const { return m_hr; }
            private:
                virtual void OnStateChange(IProgressObserver::State enumVal) {}
                virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo)
                {
                    CString id;
                    CString filename;
                    CString result;
                    int curPos = 0;

                    switch (enumVal)
                    {
                    case IProgressObserver::Downloading:
                        if (changeInfo != L"")
                        {
                            id = changeInfo.Tokenize(L";", curPos);
                            filename = changeInfo.Tokenize(L";", curPos);
                            m_burnView.OnDownloadPayloadBegin(id, filename);
                        }
                        break;
                    case IProgressObserver::DownloadItemComplete:
                        id = changeInfo.Tokenize(L";", curPos);
                        filename = changeInfo.Tokenize(L";", curPos);
                        result = changeInfo.Tokenize(L";", curPos);
                        m_burnView.OnDownloadPayloadComplete(id, filename, _wtoi(result));
                        break;
                    }
                }

                // OnProgress - Progress ranges from 0 to 255
                virtual int OnProgress(unsigned char soFar) 
                {
                    int ret = IDOK;

                    if (!m_bDownloadFinished)
                    {
                        m_downloadProgress = 100*soFar/255;
                        ret = m_burnView.OnDownloadProgress(m_progressDetail, m_downloadProgress);
                    }
                    else
                    {
                        m_installProgress = 100*soFar/255;
                        ret = m_burnView.OnExecuteProgress(m_progressDetail, m_installProgress);
                    }

                    int progRet = m_burnView.OnProgress(m_progressDetail, (m_downloadProgress + m_installProgress)/2);
                    if (progRet != IDOK)
                    {
                        ret = progRet;
                    }

                    return ret;
                }

                // OnProgressDetail - Progress ranges from 0 to 255, for individual package/payload.
                virtual int OnProgressDetail(unsigned char soFar) 
                {
                    m_progressDetail = 100*soFar/255;
                    return IDOK; 
                }

                // Finished - Gets called when download thread or execute thread completes the operation.
                virtual void Finished(HRESULT hr)
                {
                    if (!m_bDownloadFinished)
                    {
                        m_bDownloadFinished = true;
                        m_burnView.OnDownloadProgress(m_progressDetail, 100);
                    }
                    m_hr = hr;
                    ::SetEvent(m_hEvent);
                }

                virtual void OnRebootPending()
                {
                    m_bRebootPending = true;
                }

            } serialObserver(hEvent, m_burnView, hr, bRebootPending);

            if (vpEngineState->fSuspend || vpEngineState->fForcedReboot)
            {
                LOG( m_logger, ILogger::Warning, L"Stopping before beginning download operation." );
                ExitFunction1(hr = S_OK);
            }

            if (!m_controller.MayBegin(serialObserver))
            {
                ExitOnFailure(E_UNEXPECTED, "CBurnController: Failed to launch Download thread. ");
            }

            ::WaitForSingleObject(hEvent, INFINITE);
            ::ResetEvent(hEvent);

            if (FAILED(hr))
            {
                LOG( m_logger, ILogger::Error, L"CBurnController: Download operation failed." );
                goto LExit;
            }

            if (vpEngineState->fSuspend || vpEngineState->fForcedReboot)
            {
                LOG( m_logger, ILogger::Warning, L"Stopping before beginning install operation." );
                ExitFunction1(hr = S_OK);
            }

            LOG( m_logger, ILogger::Verbose, L"Launching Install operation. Download operation is completed." );
            nResult = m_burnView.OnExecuteBegin(itemCount);
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                ExitOnFailure(hr, "User interface commanded engine to abort");
            }

            if (!m_controller.MayBegin(serialObserver))
            {
                ExitOnFailure(E_UNEXPECTED, "CBurnController: Failed to launch Install thread. ");
            }

            ::WaitForSingleObject(hEvent, INFINITE);
            ::ResetEvent(hEvent);
            m_burnView.OnExecuteComplete(hr);

            if (!MSIUtils::IsSuccess(hr))
            {
                LOG( m_logger, ILogger::Error, L"CBurnController: Install operation failed." );
                goto LExit;
            }


LExit:
            if (hEvent)
                ::CloseHandle(hEvent);
            return hr;
        }

        //
        // ProcessMessage - called to process a controller message.
        //
        void ProcessMessage(
            __in const MSG* pmsg
            )
        {
            switch (pmsg->message)
            {
            case WM_BURN_VIEW_INITIALIZED:
                break;

            case WM_BURN_DETECT:
                OnDetect();
                break;

            case WM_BURN_PLAN:
                OnPlan(static_cast<BURN_ACTION>(pmsg->lParam));
                break;

            case WM_BURN_APPLY:
                OnApply(reinterpret_cast<HWND>(pmsg->lParam));
                break;

            case WM_BURN_SHUTDOWN:
                ::PostQuitMessage((int)pmsg->lParam);
                break;

            default:
                CString logMessage;
                logMessage.Format(L"Unexpected controller message: %u", pmsg->message);
                LOG(m_logger, ILogger::Warning, logMessage);
                break;
            }
        }

        void OnDetect()
        {
            HRESULT hr = S_OK;

            Trace(REPORT_STANDARD, "CBurnController::OnDetect() - enter");

            // UX has to be initialized before UX can call Detect
            if (!m_burnView.IsUxInitialized())
            {
                hr = E_UNEXPECTED;
                LOG(m_logger, ILogger::Error, L"UX has to be initialized successfully before Detect can be called.");
                ExitOnFailure(hr, "Cannot call Detect if Initialize() is not completed successfully");
            }

            int nResult = m_burnView.OnDetectBegin(m_engine.GetAuthoredItemCount());
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted detect begin.");

            hr = m_engine.Detect();
            ExitOnFailure(hr, "Failed on Detect.");

        LExit:
            m_burnView.OnDetectComplete(hr);

            Trace(REPORT_STANDARD, "CBurnController::OnDetect() - exit");
            return;
        }

        void OnPlan(
            __in BURN_ACTION action
            )
        {
            HRESULT hr = S_OK;

            Trace(REPORT_STANDARD, "CBurnController::OnPlan() - enter");

            // Make sure Detect completed successfully
            if (!m_burnView.IsDetectComplete())
            {
                hr = E_UNEXPECTED;
                LOG(m_logger, ILogger::Error, L"Detect has to complete successfully before Plan can be called.");
                ExitOnFailure(hr, "Cannot call Plan if Detect is not completed successfully");
            }

            m_action = action;
            m_burnView.SetCurrentAction(m_action);

            int nResult = m_burnView.OnPlanBegin(m_engine.GetAuthoredItemCount());
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted plan begin.");

            hr = m_engine.Plan(action);
            ExitOnFailure(hr, "Failed on Plan.");

        LExit:
            m_burnView.OnPlanComplete(hr);

            Trace(REPORT_STANDARD, "CBurnController::OnPlan() - exit");
            return;
        }

        //
        // OnApply - called from UX to start applying (cache and execute) packages.
        //
        void OnApply(__in_opt HWND hwndParent)
        {
            HRESULT hr = S_OK;
            HRESULT hrPrimaryAction = S_OK;
            BOOL fUserCancelled = FALSE; // Indicates that user cancelled, using
            BOOL fRebootRequired = FALSE; // Indicates that one of the packages required reboot.
            BOOL fRebootInitiated = FALSE; // Indicates that windows installer initiated reboot.
            bool rebootPending = false; // Is this needed anymore
            BOOL fNeedsElevation = FALSE;
            BOOL fRollback = FALSE;
            BURN_ACTION action = BURN_ACTION_UNKNOWN;
            BYTE* pbBuffer = NULL;
            SIZE_T cbBuffer = 0;
            BOOL fRegistering = FALSE;
            BOOL fUnregistering = FALSE;
            int nResult = IDOK;
            IronMan::CCmdLineSwitches switches;

            // Make sure Detect completed successfully
            if (!m_burnView.IsDetectComplete())
            {
                hr = E_UNEXPECTED;
                LOG(m_logger, ILogger::Error, L"Detect has to complete successfully before Apply can be called.");
                ExitOnFailure(hr, "Cannot call Apply if Detect is not completed successfully");
            }
            // Make sure Plan completed successfully
            if (!m_burnView.IsPlanReady())
            {
                hr = E_UNEXPECTED;
                LOG(m_logger, ILogger::Error, L"Plan has to complete successfully before Apply can be called.");
                ExitOnFailure(hr, "Cannot call Apply if Plan is not completed successfully");
            }

            nResult = m_burnView.OnApplyBegin();
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted apply begin.");

            fNeedsElevation = m_burnView.IsElevationRequired();

            // create per-machine process if needed
            if ((fNeedsElevation || vpEngineState->registration.fPerMachine) && !vpEngineState->hElevatedProcess)
            {
                hr = Elevate(hwndParent);
                if (HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED) == hr)
                {
                    hr = S_OK;
                }
                ExitOnFailure(hr, "Failed to connect to elevated process.");
            }

            // begin or resume session
            nResult = m_burnView.OnRegisterBegin();
            fRegistering = TRUE;
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnFailure(hr, "UX aborted OnRegisterBegin");
            if (BURN_RESUME_TYPE_NONE == vResumeType)
            {
                // begin new session
                hr =  RegistrationSessionBegin(&vpEngineState->registration, &vpEngineState->userExperience, this->m_action, 0, FALSE);
                ExitOnFailure(hr, "Failed to begin registration session.");

                if (vpEngineState->registration.fPerMachine)
                {
                    hr = ElevationSessionBegin(vpEngineState->hElevatedPipe, this->m_action, 0);
                    ExitOnFailure(hr, "Failed to begin registration session in per-machine process.");
                }
            }
            else
            {
                // resume previous session
                hr =  RegistrationSessionResume(&vpEngineState->registration, this->m_action, FALSE);
                ExitOnFailure(hr, "Failed to resume registration session.");

                if (vpEngineState->registration.fPerMachine)
                {
                    hr =  ElevationSessionResume(vpEngineState->hElevatedPipe, this->m_action);
                    ExitOnFailure(hr, "Failed to resume registration session in per-machine process.");
                }
            }

            // save engine state
            hr = CoreSerializeEngineState(vpEngineState, &pbBuffer, &cbBuffer);
            ExitOnFailure(hr, "Failed to serialize engine state.");

            if (vpEngineState->registration.fPerMachine)
            {
                hr = ElevationSaveState(vpEngineState->hElevatedPipe, pbBuffer, cbBuffer);
                ExitOnFailure(hr, "Failed to save engine state in per-machine process.");
            }
            else
            {
                hr = RegistrationSaveState(&vpEngineState->registration, pbBuffer, cbBuffer);
                ExitOnFailure(hr, "Failed to save engine state.");
            }

            fRegistering = FALSE;
            m_burnView.OnRegisterComplete(hr);

            // if we need to elevate, create per-machine process
            if (fNeedsElevation && !vpEngineState->hElevatedProcess)
            {
                hr = ElevationParentProcessConnect(hwndParent, &vpEngineState->hElevatedProcess, &vpEngineState->hElevatedPipe);
                ExitOnFailure(hr, "Failed to connect to elevated process.");
            }

            // Extract attached payloads to working directory
            hr = ModuleUtils::ExtractEngineData<CCmdLineSwitches>(ModuleUtils::eplAttachedPayloads);
            ExitOnFailure(hr, "Failed to extract attached payloads to working directory.");

            // Download and Cache + Install the packages
            if (switches.SerialDownloadSwitchPresent())
            {
                LOG(m_logger, ILogger::Result, L"Using sequential download and install mechanism.");
                hrPrimaryAction = RunSequentialDownloadAndInstall(m_engine.GetInstallItems().GetCount(), rebootPending);
            }
            else
            {
                LOG(m_logger, ILogger::Result, L"Using simultaneous download and install mechanism.");
                hrPrimaryAction = RunSimultaneousDownloadAndInstall(m_engine.GetInstallItems().GetCount());
            }

            if (!MSIUtils::IsSuccess(hrPrimaryAction))
            {
                fRollback = TRUE;
            }
            
            m_burnView.GetViewState(&fUserCancelled, &fRebootRequired, &fRebootInitiated);
            
            // The following variables to be used by Fredrik's suspend/resume/reboot changes
            vpEngineState->fCancelled = fUserCancelled;
            // vpEngineState->fSuspend is TRUE when UX called suspend.
            // vpEngineState->fForcedReboot is TRUE when UX called Reboot

            if (fRebootRequired)
            {
                vpEngineState->fReboot = m_burnView.OnRestartRequired();
            }

            fUnregistering = TRUE;
            m_burnView.OnUnregisterBegin();

            // save engine state
            hr = CoreSerializeEngineState(vpEngineState, &pbBuffer, &cbBuffer);
            ExitOnFailure(hr, "Failed to serialize engine state.");

            if (vpEngineState->registration.fPerMachine)
            {
                hr = ElevationSaveState(vpEngineState->hElevatedPipe, pbBuffer, cbBuffer);
                ExitOnFailure(hr, "Failed to save engine state in per-machine process.");
            }
            else
            {
                hr = RegistrationSaveState(&vpEngineState->registration, pbBuffer, cbBuffer);
                ExitOnFailure(hr, "Failed to save engine state.");
            }

            // suspend or end session
            if (vpEngineState->fSuspend || vpEngineState->fForcedReboot)
            {
                if (vpEngineState->registration.fPerMachine)
                {
                    hr = ElevationSessionSuspend(vpEngineState->hElevatedPipe, this->m_action, vpEngineState->fForcedReboot);
                    ExitOnFailure(hr, "Failed to suspend session in per-machine process.");
                }

                hr = RegistrationSessionSuspend(&vpEngineState->registration, this->m_action, vpEngineState->fForcedReboot, FALSE);
                ExitOnFailure(hr, "Failed to suspend session in per-user process.");
            }
            else
            {
                if (vpEngineState->registration.fPerMachine)
                {
                    hr = ElevationSessionEnd(vpEngineState->hElevatedPipe, this->m_action, fRollback);
                    ExitOnFailure(hr, "Failed to end session in per-machine process.");
                }

                hr = RegistrationSessionEnd(&vpEngineState->registration, this->m_action, fRollback, FALSE);
                ExitOnFailure(hr, "Failed to end session in per-user process.");
            }

           fUnregistering = FALSE;
           m_burnView.OnUnregisterComplete(hr);

LExit:
            if (fRegistering)
            {
                m_burnView.OnRegisterComplete(hr);
            }
            if (fUnregistering)
            {
                m_burnView.OnUnregisterComplete(hr);
            }

            if (FAILED(hrPrimaryAction))
            {
                m_burnView.OnApplyComplete(hrPrimaryAction);
            }
            else
            {
                m_burnView.OnApplyComplete(hr);
            }

            // end per-machine process if running
            if (vpEngineState->hElevatedProcess)
            {
                ElevationParentProcessTerminate(vpEngineState->hElevatedProcess, vpEngineState->hElevatedPipe);

                ::CloseHandle(vpEngineState->hElevatedProcess);
                vpEngineState->hElevatedProcess = NULL;

                ::CloseHandle(vpEngineState->hElevatedPipe);
                vpEngineState->hElevatedPipe = INVALID_HANDLE_VALUE;
            }
        }

        //
        // GetBurnAction
        // 
        static BURN_ACTION GetBurnAction(Operation::euiOperation operation)
        {
            if (operation == Operation::uioUninstalling)
                return BURN_ACTION_UNINSTALL;
            if (operation == Operation::uioRepairing)
                return BURN_ACTION_REPAIR;
            if (operation == Operation::uioMaintenance)
                return BURN_ACTION_MODIFY;
            return BURN_ACTION_INSTALL;
        }
    };
}
