//-------------------------------------------------------------------------------------------------
// <copyright file="BitsLogger.h" company="Microsoft">
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
//    This objective of this class is to turn BITS tracing ON or OFF
//    BITS uses the ETW tracing to create a log of BITS activities/errors.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "common\ProcessUtils.h"

#define MAX_SESSION_NAME_LENGTH 1024
#define MAX_LOG_FILE_NAME 1024
#define MAX_LOG_FILE_SIZE 5 //MB

namespace IronMan
{
    /* 9AB2A13B-A832-4A65-B4BB-7FB260AC9D2D */
    static const GUID BITS_TRACE_SESSION_ID  = { 0x9AB2A13B, 0xA832, 0x4A65, { 0xB4, 0xBB, 0x7F, 0xB2, 0x60, 0xAC, 0x9D, 0x2D} };
    static const CString BITSLOGGER_SESSION_NAME = L"Ironman Bits Logger Session";
    static const GUID BITS_PROVIDER_ID = { 0x4A8AAA94, 0xCFC4, 0x46A7, { 0x8E, 0x4E, 0x17, 0xBC, 0x45, 0x60, 0x8F, 0x0A} };

class BitsLogger
{
    typedef struct _FULL_EVENT_TRACE_PROPERTIES 
    {
        EVENT_TRACE_PROPERTIES EventProperties;
        TCHAR szSessionName[MAX_SESSION_NAME_LENGTH]; // must come before log file name
        TCHAR szTraceOutputName[MAX_LOG_FILE_NAME];
    } FULL_EVENT_TRACE_PROPERTIES;

    enum BitsLogKeywords
    {
        LogFlagInfo = 0x0000000000000001,
        LogFlagWarning = 0x0000000000000002,
        LogFlagError = 0x0000000000000004,
        LogFlagFunction = 0x0000000000000008,
        LogFlagRefCount = 0x0000000000000010,
        LogFlagSerialize = 0x0000000000000020,
        LogFlagDownload = 0x0000000000000040,
        LogFlagTask = 0x0000000000000080,
        LogFlagLock = 0x0000000000000100,
        LogFlagService = 0x0000000000000200,
        LogFlagDataBytes = 0x0000000000000400,
        LogFlagTransferDetails = 0x0000000000000800,
        LogFlagPeer = 0x0000000000001000
    };

    CPath m_bitsLogFile;
    FULL_EVENT_TRACE_PROPERTIES m_sProperties;
    TRACEHANDLE m_hTrace;
    const BitsLogKeywords m_defaultKeywords;
    ILogger& m_logger;

    bool m_bWasSessionActive;
    bool m_bWasProviderEnabled;

public:
    //Constructor
    BitsLogger(ILogger& logger)
        : m_logger(logger)
        , m_bWasSessionActive(false)
        , m_bWasProviderEnabled(false)
        , m_hTrace(NULL)
        , m_defaultKeywords((BitsLogKeywords) (LogFlagInfo | LogFlagWarning | LogFlagError | LogFlagFunction | LogFlagDownload | LogFlagTask | LogFlagLock | LogFlagService | LogFlagTransferDetails | LogFlagPeer))
    {
            //Create a file with dd prefix so that it can be pick up by the collect tool.
            m_bitsLogFile.Combine(ProcessUtils::GetTempFolderOfDelevatedUser(), L"dd_BITS.log");
            LOG(logger, ILogger::InternalUseOnly, m_bitsLogFile);
    }

    //Destructor
    ~BitsLogger()
    {
    }

    //Here is the step to turn on logging
    // a. Check if BITS trace is active 
    //    1. If activeOtherwise, Enable Bits Tracing
    //    2. Verify again if BITS trace is active
    // b. Check if the provider is active
    //    1. Otherwise, Enable Bits provider 
    //    2. Verify again if the provider is active.
    void Start()
    {
        //Since Start can be called multiple time, we want to start only the same time.
        if(NULL == m_hTrace)
        {
            bool bIsActive = false;
            HRESULT hr = S_OK;
            hr = IsTraceActive(bIsActive);
            if (S_OK == hr)
            {
                if (bIsActive)
                {
                    //Need to remember the original state
                    m_bWasSessionActive = true;
                }
                else
                {
                    hr = EnableBitsTrace(m_bitsLogFile);
                    if(S_OK == hr)
                    {
                        hr = IsTraceActive(bIsActive);
                    }
                }

                if ((S_OK == hr) && (bIsActive))
                {
                    hr = IsProviderActive(bIsActive);
                    if (S_OK == hr) 
                    {
                        if (bIsActive)
                        {
                            m_bWasProviderEnabled = true;
                        }
                        else
                        {
                            hr = EnableBitsProvider();
                            if (S_OK == hr)
                            {
                                LOG(m_logger, ILogger::Information, L"BITS logging turned on");
                                hr = IsProviderActive(bIsActive);
                            }
                        }
                    }
                }
            }

            //Error handling
            //Turn turn off if it was originally on.
            if ((FAILED(hr) || !bIsActive)  && !m_bWasSessionActive)
            {
                LOG(m_logger, ILogger::Warning, L"Failed to turn on Bits Logging");
                DisableBitsTrace();
            }
        }
    }

    //Stop Bits logging
    void Stop()
    {
        bool bActive = false;

        if (!m_bWasProviderEnabled)
        {
            HRESULT hr = IsTraceActive(bActive);
            if (S_OK == hr && bActive)
            {
                hr = IsProviderActive(bActive);
                if(S_OK == hr && bActive)
                {
                    LOG(m_logger, ILogger::Information, L"BITS logging turned off");
                    DisableBitsProvider();
                }
            }
        }

        //Don't even bother to turn off if it was originally on.
        if (!m_bWasSessionActive)
        {
            DisableBitsTrace();
            m_hTrace = NULL;
        }
    }

private:
    //Determine if BITS tracing is active.
    HRESULT IsTraceActive(bool &bActive)
    {
        HRESULT hr = S_OK;
        FULL_EVENT_TRACE_PROPERTIES properties = {0};

        if(NULL == m_hTrace)
        {
            bActive = false;
        }
        else
        {
            properties.EventProperties.Wnode.BufferSize = sizeof(properties);
            properties.EventProperties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
            properties.EventProperties.Wnode.Guid = BITS_TRACE_SESSION_ID;
            properties.EventProperties.LoggerNameOffset = (ULONG)(offsetof(FULL_EVENT_TRACE_PROPERTIES, szSessionName));
            properties.EventProperties.LogFileNameOffset = 0;

            ULONG returnCode = ControlTrace(m_hTrace, NULL, (PEVENT_TRACE_PROPERTIES) &properties, EVENT_TRACE_CONTROL_QUERY);

            if(ERROR_SUCCESS == returnCode)
            {
                bActive = true;
            }
            else if (ERROR_WMI_INSTANCE_NOT_FOUND == returnCode)
            {
                bActive = false;
            }
            else
            {
                hr = E_FAIL;
            }
        }

        return hr;
    }

    //Enable BITS tracing
    HRESULT EnableBitsTrace(const CString& strLogName)
    {
        HRESULT hr = S_OK;

        ZeroMemory(&m_sProperties, sizeof(m_sProperties));
        //BufferSize: Total size of memory allocated, in bytes, for the event tracing session properties.
        m_sProperties.EventProperties.Wnode.BufferSize = sizeof(m_sProperties);
        //Flags: Must contain WNODE_FLAG_TRACED_GUID to indicate that the structure contains event tracing information.
        m_sProperties.EventProperties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
        //Clock resolution to use when logging the time stamp for each event.
        // 1 - Query performance counter (QPC).
        m_sProperties.EventProperties.Wnode.ClientContext = 1;
        //Guid: The GUID that you define for the session.
        m_sProperties.EventProperties.Wnode.Guid = BITS_TRACE_SESSION_ID;
        //LogFileMode: Logging modes for the event tracing session.
        //EVENT_TRACE_USE_PAGED_MEMORY   - Uses paged memory for buffer space which is a relative less expensive then nonpaged memory
        //EVENT_TRACE_FILE_MODE_CIRCULAR - Writes events to a log file. After the file reaches the maximum size,
        //                                 the oldest events are replaced with incoming events.
        m_sProperties.EventProperties.LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR | EVENT_TRACE_USE_PAGED_MEMORY;
        //MaximumFileSize: Maximum size of the file used to log events, in megabytes.
        m_sProperties.EventProperties.MaximumFileSize = MAX_LOG_FILE_SIZE;

        m_sProperties.EventProperties.LoggerNameOffset = (ULONG)(offsetof(FULL_EVENT_TRACE_PROPERTIES, szSessionName));
        m_sProperties.EventProperties.LogFileNameOffset = (ULONG)(offsetof(FULL_EVENT_TRACE_PROPERTIES, szTraceOutputName));
        StringCchCopy(m_sProperties.szTraceOutputName, MAX_LOG_FILE_NAME, m_bitsLogFile);

        ULONG returnCode = StartTrace(&m_hTrace, BITSLOGGER_SESSION_NAME, (PEVENT_TRACE_PROPERTIES)&m_sProperties);
        if(ERROR_SUCCESS != returnCode)
        {
            hr = E_FAIL;
            ZeroMemory(&m_sProperties, sizeof(m_sProperties));
        }
        return hr;
    }

    //Disable BITS tracing
    HRESULT DisableBitsTrace()
    {
        HRESULT hr = S_OK;

        m_sProperties.EventProperties.LoggerNameOffset = (ULONG)(offsetof(FULL_EVENT_TRACE_PROPERTIES, szSessionName));
        m_sProperties.EventProperties.LogFileNameOffset = (ULONG)(offsetof(FULL_EVENT_TRACE_PROPERTIES, szTraceOutputName));
        ULONG returnCode = ControlTrace(m_hTrace, NULL, (PEVENT_TRACE_PROPERTIES)&m_sProperties, EVENT_TRACE_CONTROL_STOP);
        if(ERROR_SUCCESS != returnCode)
        {
            hr = E_FAIL;
        }
        return hr;
    }

    //Enable BITS provider
    HRESULT EnableBitsProvider()
    {
        HRESULT hr = S_OK;

        ULONG returnCode = EnableTrace(TRUE, m_defaultKeywords, 255, &BITS_PROVIDER_ID, m_hTrace);
        if(ERROR_SUCCESS != returnCode)
        {
            hr = E_FAIL;
        }

        return hr;
    }

    //Determine if BITS provider is enabled.
    HRESULT IsProviderActive(bool &bActive)
    {
        HRESULT hr = S_OK;
        TRACE_GUID_PROPERTIES traceInfo = {0};
        PTRACE_GUID_PROPERTIES pTraceInfo = &traceInfo;
        PTRACE_GUID_PROPERTIES *ppProviders = NULL;
        PTRACE_GUID_PROPERTIES pProviderBuffer = NULL;
        ULONG nActualProviderCount = 0;

        bActive = false;

        ULONG ulReturnCode = EnumerateTraceGuids(&pTraceInfo, 1, &nActualProviderCount);

        if(ERROR_MORE_DATA == ulReturnCode && nActualProviderCount > 0)
        {
            ppProviders = static_cast<PTRACE_GUID_PROPERTIES*>(calloc(nActualProviderCount, sizeof(PTRACE_GUID_PROPERTIES)));
            pProviderBuffer = static_cast<PTRACE_GUID_PROPERTIES>(calloc(nActualProviderCount, sizeof(TRACE_GUID_PROPERTIES)));

            if(ppProviders && pProviderBuffer)
            {
                for(ULONG i = 0; i < nActualProviderCount; i++)
                {
                    ppProviders[i] = &(pProviderBuffer[i]);
                }

                ulReturnCode = EnumerateTraceGuids(ppProviders, nActualProviderCount, &nActualProviderCount);
            }
            else
            {
                ulReturnCode = ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            ppProviders = &pTraceInfo;
        }

        if(ERROR_SUCCESS == ulReturnCode)
        {
            for(ULONG i = 0; !bActive && i < nActualProviderCount; i++)
            {
                if(ppProviders[i]->IsEnable)
                {
                    if(ppProviders[i]->Guid == BITS_PROVIDER_ID && ppProviders[i]->LoggerId == m_hTrace)
                    {
                        bActive = true;
                    }
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }

        if(ppProviders != &pTraceInfo)
        {
            free(ppProviders);
            ppProviders = NULL;
        }

        if(pProviderBuffer)
        {
            free(pProviderBuffer);
            pProviderBuffer = NULL;
        }

        return hr;
    }

    //Disable BITS provider.
    HRESULT DisableBitsProvider(void)
    {
        HRESULT hr = S_OK;

        ULONG returnCode = EnableTrace(FALSE, 0, 0, &BITS_PROVIDER_ID, m_hTrace);
        if(ERROR_SUCCESS != returnCode)
        {
            hr = E_FAIL;
        }

        return hr;
    }

//Virtual methods specifically for testing
private:
    virtual ULONG ControlTrace(__in TRACEHANDLE TraceHandle
                        , __in_opt LPCWSTR InstanceName
                        , __inout PEVENT_TRACE_PROPERTIES Properties
                        , __in ULONG ControlCode)
    {
        return ::ControlTrace(TraceHandle, InstanceName, Properties, ControlCode);
    }

    virtual ULONG StartTrace(__out PTRACEHANDLE TraceHandle
                            , __in LPCWSTR InstanceName
                            , __inout PEVENT_TRACE_PROPERTIES Properties)
    {
        return ::StartTrace(TraceHandle, InstanceName, Properties);
    }

    virtual ULONG EnumerateTraceGuids(  __inout_ecount(PropertyArrayCount) PTRACE_GUID_PROPERTIES *GuidPropertiesArray
                                        ,  __in ULONG PropertyArrayCount
                                        , __out PULONG GuidCount)
    {
        return ::EnumerateTraceGuids(GuidPropertiesArray, PropertyArrayCount, GuidCount);
    }

    virtual ULONG EnableTrace(__in ULONG Enable
                                , __in ULONG EnableFlag
                                , __in ULONG EnableLevel
                                , __in LPCGUID ControlGuid
                                , __in TRACEHANDLE TraceHandle)
    {
        return ::EnableTrace(Enable, EnableFlag, EnableLevel, ControlGuid, TraceHandle);
    }
};
}
