//-------------------------------------------------------------------------------------------------
// <copyright file="FileMetrics.h" company="Microsoft">
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
//      This class encapsulate all the functionality to collect usage data.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IMetrics.h"
#include "..\common\logutils.h"

namespace IronMan
{
    class FileMetrics : public IMetrics
    {
        CAtlFile m_file;
        bool m_bSendFile;
        DWORD m_iCurrentStreamId;
        CSimpleArray<CString> m_arrColumns;

    public:
        FileMetrics()
            : m_bSendFile(false)
            , m_iCurrentStreamId(0)
        {}

        ~FileMetrics()
        {
            m_file.Close();
            //Delete the file since it has opt not to send
            if (false == m_bSendFile)
            {
                ::DeleteFile(MetricFilePath());
            }
        }

        //Append to an existing file.  This is intented to be used only when the process crash.
        virtual HRESULT ContinueSession()
        {
            m_file.m_h = Open(CPath(MetricFilePath()));
            return S_OK;
        }

        virtual HRESULT StartSession()
        {
            CSystemUtil::ExpandEnvironmentVariables(CString(LogUtils::GenerateLogFileName(L"Metrics")+ L".xml"), MetricFilePath());
            m_file.m_h = Open(CPath(MetricFilePath()));
            return S_OK;
        }

        //The metrics file will be send.
        virtual HRESULT EndAndSend(const CPath& pthDataFileFullPath, const CPath& pthLoaderFileFullPath)
        {
            //Write closing tag, then close the file
            m_bSendFile = true;  //Keep the file since we are sending it.
            Write(L"\r\n</Data>");
            m_file.Close();  //Close the file
            
            //Call the appropriate exe to upload metrics data 
            STARTUPINFO startupInfo;
            ::ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
            startupInfo.cb = sizeof( STARTUPINFO );
            ProcessUtils::CProcessInformation processInformation;

            const CPath& exeFullName = pthLoaderFileFullPath;

            CPath workingDir(exeFullName);
            workingDir.RemoveFileSpec();

            CPath exeName(exeFullName);
            exeName.StripPath();
            exeName.QuoteSpaces();

            CString commandLine;
            commandLine.Format(L"%s %s", exeName, MetricFilePath());

            bool bCreateProcessSucceded = CreateProcess(exeFullName,
                                                          commandLine.GetBuffer(),
                                                          NULL, // lpProcessAttributes
                                                          NULL, // lpThreadAttributes
                                                          TRUE, // bInheritHandles
                                                          CREATE_NO_WINDOW, // dwCreationFlags
                                                          NULL, // lpEnvironment
                                                          workingDir,
                                                          &startupInfo,
                                                          &processInformation);

            if (false == bCreateProcessSucceded)
            {   
                DWORD err = GetLastError();
                CComBSTR errMsg;
                CSystemUtil::GetErrorString(err, errMsg);

                return HRESULT_FROM_WIN32(err);
            }

            // This is a blocking call and gets signalled at regular interval (100 ms).
            MsgWaitForObject::Wait(processInformation.hProcess); 

            DWORD exitCode = 0;
            GetExitCodeProcess(processInformation.hProcess, &exitCode);
            
            CString resultString;
            if (0 != exitCode)
            {
                return (HRESULT_FROM_WIN32(exitCode));
            }
            return exitCode;
        }

        //Write the stream data with string item type.
        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberOfColumn, const CString& csStringToWrite)
        {
            CString cs;
            cs.Format(L"\r\n\t<String Value=\"%s\" />", csStringToWrite);
            return WriteStream(iDatapointId, iNumberOfColumn, cs);
        }

        //Write the stream data with DWORD item type.
        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberOfColumn, const DWORD& dwValue)
        {
            CString cs;
            cs.Format(L"\r\n\t<Dword Value=\"%s\" />", StringUtil::FromDword(dwValue));
            return WriteStream(iDatapointId, iNumberOfColumn, cs);
        }

        virtual HRESULT Write(__in DWORD iDatapointId, __in_opt DWORD dwValue)
        {
            CString cs;
            cs.Format(L"\r\n<Dword Id=\"%i\" Value=\"%s\" />", iDatapointId, StringUtil::FromDword(dwValue));
            return Write(cs);
        }

        virtual HRESULT Write(__in DWORD iDatapointId, __in_opt HRESULT hrValue)
        {
            CString cs;
            cs.Format(L"\r\n<Hresult Id=\"%i\" Value=\"%s\" />", iDatapointId, StringUtil::FromHresult(hrValue));
            return Write(cs);
        }

        virtual HRESULT Write(__in DWORD iDatapointId, __in_opt bool bValue)
        {
            CString cs;
            cs.Format(L"\r\n<Bool Id=\"%i\" Value=\"%s\" />", iDatapointId, StringUtil::FromBool(bValue));
            return Write(cs);
        }

        virtual HRESULT Write(__in DWORD iDatapointId, __in_opt const CString& csValue)
        {
            CString cs;
            cs.Format(L"\r\n<String Id=\"%i\" Value=\"%s\" />", iDatapointId, csValue);
            return Write(cs);
        }

    private:
        static CString& MetricFilePath()
        {
            static CString& csMetricFilePath = CString(L"");
            return csMetricFilePath;
        }

        HRESULT WriteStream(unsigned int iDatapointId, int iNumberOfColumns, CString& cs)
        {
            if ((0 != m_iCurrentStreamId) && (m_iCurrentStreamId != iDatapointId))
            {
                //clear the array
                m_arrColumns.RemoveAll();
                m_iCurrentStreamId = 0;
            }
            //If we have all the columns, write out it all out.
            else if ((m_iCurrentStreamId == iDatapointId) && (m_arrColumns.GetSize() == iNumberOfColumns-1))
            {
                m_arrColumns.Add(cs);

                CString csWrite;
                csWrite.Format(L"\r\n<Stream Id=\"%i\" NoOfColumns=\"%i\" >", iDatapointId, iNumberOfColumns);
                Write(csWrite);

                //write out all the data point
                for(int i=0; i < m_arrColumns.GetSize(); ++i)
                {
                    Write(m_arrColumns[i]);
                }
                Write(L"\r\n</Stream>");

                m_arrColumns.RemoveAll();
                m_iCurrentStreamId = 0;
            }
            else
            {
                m_arrColumns.Add(cs);
                m_iCurrentStreamId = iDatapointId;
            }
            return S_OK;
        }

        HRESULT Write(CString cs)
        {
            //If we don't have the file handle, don't bother writing.
            if (NULL == m_file.m_h)
            {
                return S_OK;
            }
            return m_file.Write(cs,  sizeof(TCHAR) * cs.GetLength());
        }

        HANDLE Open(CPath pthFileNameInput)
        {
            if (CString(pthFileNameInput).IsEmpty())
            {
                return NULL;
            }

            CPath pthFileName;
            if (pthFileNameInput.IsRelative())
            {
                pthFileName.Combine(ModuleUtils::GetDllPath(), pthFileNameInput);
            }
            else
            {
                pthFileName = pthFileNameInput;
            }

            // Create the log file
            CAtlFile file;

            bool bIsFileExist = pthFileName.FileExists();
            LPSECURITY_ATTRIBUTES lpsa = NULL;
            HRESULT hr = file.Create(pthFileName,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    lpsa);
            if (FAILED(hr))
            {
                return NULL;
            }

            file.Seek(0, FILE_END);

            if (!bIsFileExist)
            {
                // UTF-16, little-endian BOM (byte order mark)
                unsigned char bom[2] = { 0xFF, 0xFE };
                file.Write(bom, 2);
            }

            CString cs = L"<Data>";
            file.Write(cs,  sizeof(TCHAR) * cs.GetLength());
            return file.Detach();
        }
    };
}