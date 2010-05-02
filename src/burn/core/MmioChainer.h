//-------------------------------------------------------------------------------------------------
// <copyright file="MmioChainer.h" company="Microsoft">
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
//    Set of classes that help Memory Mapped IO (MmIO) pipe communication.
//
//    Look at MmioChainer class to implement your own chainer.
//    Look at MmioChainee class to see how IronMan behaves as a chainee when 
//    an external chainer is chaining it.
//    MmioChainerBase class manages the communication and synchronization data 
//    datastructures. It also implements common Getters (for chainer) and 
//    Setters(for chainee).
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IProgressObserver.h"
#include "LogSignatureDecorator.h"
#include "interfaces\ILogger.h"
#include "common\StringUtil.h"

#define MESSAGE_BUFFER_SIZE 8 * 1024

namespace IronMan
{
 
// MmIO DataStructure
struct MmioDataStructure
{
    bool m_downloadFinished;		// download done yet?
    bool m_installFinished;		    // install done yet?
    bool m_downloadAbort;			// set downloader to abort
    bool m_installAbort;			// set installer to abort
    HRESULT m_hrDownloadFinished;	// resultant HRESULT for download
    HRESULT m_hrInstallFinished;	// resultant HRESULT for install
    HRESULT m_hrInternalError;
    WCHAR m_szCurrentItemStep[MAX_PATH];
    unsigned char m_downloadSoFar;	// download progress 0 - 255 (0 to 100% done) 
    unsigned char m_installSoFar;	// install progress 0 - 255 (0 to 100% done)
    WCHAR m_szEventName[MAX_PATH];  // event that chainer 'creates' and chainee 'opens'to sync communications
    
};

//  MmioChainerBase class manages the communication and synchronization data 
//  datastructures. It also implements common Getters (for chainer) and 
//  Setters(for chainee).
class MmioChainerBase
{

private:
    HANDLE m_section;
    MmioDataStructure* m_pData;

protected:
     
    ILogger& m_logger;
    HANDLE m_event;

    virtual ~MmioChainerBase()
    {
        if (m_pData)
            ::UnmapViewOfFile(m_pData);
    }
    MmioChainerBase(HANDLE section, HANDLE hevent, ILogger& logger)
        : m_section(section)
        , m_event(hevent) 
        , m_pData(MapView(section, logger))
        , m_logger(logger)
    {}


public:
    HANDLE GetEventHandle() const 
    { 
        return m_event;
    }

    HANDLE GetMmioHandle()  const { return m_section; }
    MmioDataStructure* GetData() { return m_pData; }

    //This is called by the chainer.    
    void Init(LPCWSTR eventName)
    {
        //Don't do anything if it is invalid.
        if (NULL == m_pData)
        {
            return;
        }

        // Common items for download and install
        wcscpy_s(m_pData->m_szEventName, MAX_PATH, eventName);

        // Download specific data
        m_pData->m_downloadFinished = false;
        m_pData->m_downloadSoFar = 0;
        m_pData->m_hrDownloadFinished = E_PENDING;
        m_pData->m_downloadAbort = false;

        // Install specific data
        m_pData->m_installFinished = false;
        m_pData->m_installSoFar = 0;
        m_pData->m_hrInstallFinished = E_PENDING;
        m_pData->m_installAbort = false;


        m_pData->m_hrInternalError = S_OK;
    }

    // This is called by the chainer to force the chained setup to be cancelled
    void Abort()
    {
      //Don't do anything if it is invalid.
      if (NULL == m_pData)
      {
          return;
      }
            
        // Chainer told us to cancel
       m_pData->m_downloadAbort= true;
       m_pData->m_installAbort = true;
    }

    // Called when chainer wants to know if chained setup has finished both download and install
    bool IsDone() const 
    { 
        if (NULL == m_pData)
        {
          return true;     
        }

        return m_pData->m_downloadFinished && m_pData->m_installFinished; 
    }
    
    // Called by the chainer to get the overall progress, i.e. the combination of the download and install
    unsigned char GetProgress() const 
    { 
        if (NULL == m_pData)
        {
          return 255;
        }
      
        return (m_pData->m_downloadSoFar + m_pData->m_installSoFar)/2;
    }

    // Return Download Progress
    unsigned char GetDownloadProgress() const
    {
        if (NULL == m_pData)
        {
          return 255;
        }
      
        return m_pData->m_downloadSoFar;
    }

    // Return Install Progress
    unsigned char GetInstallProgress() const
    {
        if (NULL == m_pData)
        {
          return 255;
        }
      
        return m_pData->m_installSoFar;
    }

    // Get the combined setup result, the install taking prioroty over download if both failed
    HRESULT GetResult() const
    { 
        if (NULL == m_pData)
        {
          return S_FALSE;
        }
      
        if (m_pData->m_hrInstallFinished != S_OK)
            return m_pData->m_hrInstallFinished;
        return m_pData->m_hrDownloadFinished;
    }

    // Get the Download result
    HRESULT GetDownloadResult() const
    { 
        if (NULL == m_pData)
        {
          return S_FALSE;
        }
      
        return m_pData->m_hrDownloadFinished;
    }

    // Get the install result
    HRESULT GetInstallResult() const
    { 
        if (NULL == m_pData)
        {
          return S_FALSE;
        }
      
         return m_pData->m_hrInstallFinished;
    }

    const HRESULT GetInternalResult() const
    {
        if (NULL == m_pData)
        {
          return S_FALSE;
        }
      
        return m_pData->m_hrInternalError;
    }

    //------------------------------------------------------------------------------
    // GetCurrentItemStep
    //
    // Get the current item step from chainee.
    //------------------------------------------------------------------------------
    const CString GetCurrentItemStep() const
    {
        if (NULL == m_pData)
        {
          return L"";
        }
      
        return CString(m_pData->m_szCurrentItemStep);
    }

    // The chainee calls this to see if the chainer has asked that we cancel
    bool IsAborted() const 
    { 
        if (NULL == m_pData)
        {
          return false;
        }
      
        return m_pData->m_installAbort || m_pData->m_downloadAbort; 
    }

    // The chainee calls this to indicate when the download and install phases are complete
    void Finished(HRESULT hr, HRESULT hrInternalResult, CString strCurrentItemStep, bool bDownloader)
    {
        if (NULL == m_pData)
        {
            //Set the event so that it will not hangs!
            ::SetEvent(m_event);
            return;      
        }

        if (bDownloader)
        {
            m_pData->m_hrDownloadFinished = hr;
            m_pData->m_downloadFinished = true;
        }
        else
        {
            m_pData->m_hrInstallFinished = hr;
            m_pData->m_installFinished = true;
        }
        m_pData->m_hrInternalError = hrInternalResult;
        wcscpy_s(m_pData->m_szCurrentItemStep, MAX_PATH, strCurrentItemStep);
        ::SetEvent(m_event);
    }

    // Called by chainee to indicate phase
    virtual void OnStateChange(IProgressObserver::State enumVal)
    {
    }

    // This is called by the chainee to indicate which item is being operated on
    virtual void OnStateChangeDetail (const IProgressObserver::State enumVal, const CString changeInfo) 
    {
    }

    void SoFar(unsigned char soFar, bool bDownloader)
    {
        if (NULL == m_pData)
        {
            //Set the event so that it will not hangs!
            ::SetEvent(m_event);
            return;
        }

        if (bDownloader)
            m_pData->m_downloadSoFar = soFar;
        else
            m_pData->m_installSoFar = soFar;
        ::SetEvent(m_event);
    }

    // Called when there is a reboot pending
    virtual void OnRebootPending()
    {
    }

protected:
    // Protected utility function to map the file into memory
    static MmioDataStructure* MapView(HANDLE section, ILogger& logger)
    {
        if (NULL == section)
        {
            //This is added to fix an Appverifier crash.
            LOG(logger, ILogger::Verbose, L"The handle to the section is Null");
            return reinterpret_cast<MmioDataStructure*>(NULL);
        }

        return reinterpret_cast<MmioDataStructure*>(::MapViewOfFile(section,
                                                                    FILE_MAP_WRITE,
                                                                    0, 0, // offsets
                                                                    sizeof(MmioDataStructure)));
    }
};

// This is the class that consumer (chainer) should derive from
class MmioChainer : protected MmioChainerBase
{
public:
    // Constructor
    MmioChainer (LPCWSTR sectionName, LPCWSTR eventName, ILogger& logger)
        : MmioChainerBase(CreateSection(sectionName), CreateEvent(eventName), logger)
    {
        Init(eventName);
    }

    // Destructor
    virtual ~MmioChainer ()
    {
        ::CloseHandle(GetEventHandle());
        ::CloseHandle(GetMmioHandle());
    }

public: // the public methods:  Abort and Run
    using MmioChainerBase::Abort;
    using MmioChainerBase::GetInstallResult;
    using MmioChainerBase::GetInstallProgress;
    using MmioChainerBase::GetDownloadResult;
    using MmioChainerBase::GetDownloadProgress;
    using MmioChainerBase::GetCurrentItemStep;

    virtual void OnStateChange(IProgressObserver::State enumVal)
    {
    }

    virtual void OnStateChangeDetail (const IProgressObserver::State enumVal, const CString changeInfo) 
    {
    }

    HRESULT GetInternalErrorCode()
    {
        return GetInternalResult();
    }

    // Called by the chainer to start the chained setup - this blocks untils the setup is complete
    void Run(HANDLE process, IProgressObserver& observer)
    {
        int nResult = IDOK;
        HRESULT hr = S_OK;
        HANDLE handles[2] = { process, GetEventHandle() };

        while(!IsDone())
        {
            DWORD ret = ::WaitForMultipleObjects(2, handles, FALSE, 100); // INFINITE ??
            switch(ret)
            {
            case WAIT_OBJECT_0:
            { // process handle closed.  Maybe it blew up, maybe it's just really fast.  Let's find out.
                if (IsDone() == false) // huh, not a good sign
                {
                    HRESULT hr = GetResult();
                    if (hr == E_PENDING) // untouched
                        observer.Finished(E_FAIL);
                    else
                        observer.Finished(hr);
                   
                    return;
                }
                break;
            }
            case WAIT_OBJECT_0 + 1:
                nResult = observer.OnProgress(GetProgress());
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                    Abort();
                }
                break;
            default:
                break;
            }		
        }
        observer.Finished(GetResult());
    }

private:
    static HANDLE CreateSection(LPCWSTR sectionName)
    {
        return ::CreateFileMapping (INVALID_HANDLE_VALUE,
                                    NULL, // security attributes
                                    PAGE_READWRITE,
                                    0, // high-order DWORD of maximum size
                                    sizeof(MmioDataStructure), // low-order DWORD of maximum size
                                    sectionName);
    }
    static HANDLE CreateEvent(LPCWSTR eventName)
    {
        return ::CreateEvent(NULL, FALSE, FALSE, eventName);
    }
};

// This is used by the chainee
class MmioChainee : protected MmioChainerBase
{
public:
    MmioChainee(LPCWSTR sectionName, ILogger& logger)
        : MmioChainerBase(OpenSection(sectionName), OpenEvent(GetEventName(sectionName, logger)), logger)
    {}
    virtual ~MmioChainee() {}

private:
    static HANDLE OpenSection(LPCWSTR sectionName)
    {
        return ::OpenFileMapping(FILE_MAP_WRITE, // read/write access
                                 FALSE,          // do not inherit the name
                                 sectionName);
    }
    static HANDLE OpenEvent(LPCWSTR eventName)
    {        
        return ::OpenEvent (EVENT_MODIFY_STATE | SYNCHRONIZE,
                            FALSE,
                            eventName);
    }
    static CString GetEventName(LPCWSTR sectionName, ILogger& logger)
    {
        CString cs = L"";

        HANDLE handle = OpenSection(sectionName);
        if (NULL == handle)
        {
            DWORD dw;
            dw = GetLastError();
            LOG(logger, ILogger::Verbose, L"OpenFileMapping fails with last error: " + StringUtil::FromDword(dw));
        }
        else
        {
            const MmioDataStructure* pData = MapView(handle, logger);
            if (pData)
            {
                cs = pData->m_szEventName;
                ::UnmapViewOfFile(pData);
            }
            ::CloseHandle(handle);
        }

        return cs;
    }
};
}
