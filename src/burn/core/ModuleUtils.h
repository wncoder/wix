//-------------------------------------------------------------------------------------------------
// <copyright file="ModuleUtils.h" company="Microsoft">
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

#include "..\Common\SystemUtil.h"
#include "LogSignatureDecorator.h"
#include "CmdLineParser.h"
#include <intsafe.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace IronMan
{

struct ModuleUtils
{
    // Return the program name including full path
    static CPath GetExecutableFileSpecification(void)
    {
        CString strExecutableName;
        GetModuleFileName(NULL,strExecutableName.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
        return CPath(strExecutableName);
    }

    static CPath GetExecutablePathSpecification()
    {	        
        CPath path = GetExecutableFileSpecification();
        path.RemoveFileSpec();
        return path;
    }

    static CPath GetDllPath()
    {
        CString dllModulePath;
        GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), dllModulePath.GetBuffer(MAX_PATH), MAX_PATH);
        dllModulePath._ReleaseBuffer();
        
        CPath pthFullPath(dllModulePath);
        pthFullPath.RemoveFileSpec();
        return pthFullPath;
    }

    // Return the program name, i.e. the filename part of the executable without the .exe (or whatever) extension
    static CString GetProgramName(void)
    {
        CPath pthExecutable = GetExecutableFileSpecification();
        CString strExecutableNameWithoutExtension = pthExecutable;
        // Get the file name part only
        strExecutableNameWithoutExtension = GetFileNameOnlyWithoutExtension(strExecutableNameWithoutExtension);

        return strExecutableNameWithoutExtension;
    }

    // Return the file name only, i.e. the filename part without the .exe (or whatever) extension
    static CString GetFileNameOnlyWithoutExtension(const CString & fullFilePath)
    {
        CPath pthFile = CPath(fullFilePath);
        CString strFileNameWithoutExtension = pthFile;
        // Get the file name part only
        int iIndexOfFileName = pthFile.FindFileName();
        int iIndexOfExtension = pthFile.FindExtension();
        CString strExtension = strFileNameWithoutExtension.Mid(iIndexOfExtension);
        strFileNameWithoutExtension = strFileNameWithoutExtension.Mid(iIndexOfFileName,(iIndexOfExtension - iIndexOfFileName));

        return strFileNameWithoutExtension;
    }

    // Return the file name, without the path. E.g., C:\myLogs\Log.html -> Log.html
    // This function also expand environment variable.
    static CString GetFileNameFromFullPath(const CString &strFullPath) 
    {
        CString strExpandedLocation;
        CSystemUtil::ExpandEnvironmentVariables(strFullPath, strExpandedLocation);
        CPath filePath = CPath(strExpandedLocation);
        //Return the file name only when
        // a.  It is not a directory
        // b.  The input path does not ends with \\.
        if (!filePath.IsDirectory() && strFullPath.GetLength() > 0 && '\\' != strFullPath[strFullPath.GetLength()-1])
        {
            filePath.StripPath();
            return filePath;
        }
        return L"";
    }

    static CPath AppendFileNameToModulePath(LPCTSTR filename)
    {
        CPath path = ModuleUtils::GetExecutableFileSpecification();
        path.RemoveFileSpec();
        CPath returnPath;
        returnPath.Combine(path, filename);		
        return returnPath;
    }

    static CPath FullPath(const CString& csLocation, ILogger& logger)
    {
        CString expandedLocation;
        CSystemUtil::ExpandEnvironmentVariables(csLocation, expandedLocation);
        CPath file(expandedLocation);

        if (file.IsRelative())
        {            
            file.Combine(ModuleUtils::GetDllPath(), expandedLocation);            
        }
        
        return file;
    }

    // If the parameter file is embedded then set a Parameter file path which overrides anything else
    static void SetParameterFileFolder(const CString strFolder)
    {
        m_pthParameterFileFolder = (CPath)(strFolder);
    }

    // Get the the ParameterInfo.xml path
    static CPath GetParameterFilePath(const CString filename)
    {
        CPath fullPath;

        // If a particular folder has been set such as in the self extraction case, use that
        if (HasEmbeddedContent())
        {
            fullPath = m_pthParameterFileFolder;
        }
        else
        {
            // Fall back 
            fullPath = ModuleUtils::GetDllPath();
        }
        
        fullPath.Append(filename);
        return fullPath;
    }

    // Set the folder where the engine should look for the Ux dll
    static void SetUxFolder(const CString strFolder)
    {
        m_pthUxFolder = (CPath)(strFolder);
    }

    // Get the folder where engine should look for the Ux dll
    static CPath GetUxFolderPath()
    {
        CString strUxFolder(m_pthUxFolder);
        if (strUxFolder.IsEmpty())
        {
            return ModuleUtils::GetDllPath();
        }
        else
        {
            return m_pthUxFolder;
        }
    }

    // Set the folder where the engine should look for the included payloads, i.e. embedded ones
    // that have been extracted, or "loose" files next to the setup executable
    static void SetIncludedPayloadsFolder(const CString strFolder)
    {
        m_pthIncludedPayloadsFolder = (CPath)(strFolder);
    }

    // Get the folder where engine should look for the included payloads, i.e. embedded ones
    // that have been extracted, or "loose" files next to the setup executable
    static CPath GetIncludedPayloadsFolderPath()
    {
        CString strIncludedPayloadsFolder(m_pthIncludedPayloadsFolder);
        if (strIncludedPayloadsFolder.IsEmpty())
        {
            return ModuleUtils::GetDllPath();
        }
        else
        {
            return m_pthIncludedPayloadsFolder;
        }
    }

    static void SetHasEmbeddedContent()
    {
        if (!m_fHasEmbeddedContent)
        {
            LogStringLine(REPORT_STANDARD, "Contains embedded payloads.");
            LogStringLine(REPORT_STANDARD, "Included payloads will be extracted to working path '%ls'", GetIncludedPayloadsFolderPath());
            LogStringLine(REPORT_STANDARD, "UX payloads will be extracted to working path '%ls'", GetUxFolderPath());
            LogStringLine(REPORT_STANDARD, "Parameter file will be extracted to working path '%ls'", GetParameterFilePath(L""));

            m_fHasEmbeddedContent = true;
        }
    }

    // Does the executable have embedded content
    static bool HasEmbeddedContent()
    {
        return m_fHasEmbeddedContent;
    }

    enum PayloadType
    {
        eplAllPayloads       = 0,
        eplUxPayloads        = 1,
        eplAttachedPayloads  = 2
    };

    static HRESULT HasDHTMLLogHeader(__out BOOL *pfHasHeader)
    {
        HRESULT hr = S_OK;
        LPWSTR sczBundleId = NULL;
        LPWSTR sczStubExePath = NULL;
        LPWSTR sczTempPath = NULL;
        LPBYTE pbManifest = NULL;
        DWORD cbManifest = 0;
        DWORD dwPayloadsOffset = 0;
        DWORD dwAttachedContainerOffset = 0;
        DWORD_PTR cchTempPath = MAX_PATH;
        IXMLDOMDocument* pixdManifest = NULL;
        IXMLDOMNode* pixnHeader = NULL;

        hr = StrAlloc(&sczTempPath, cchTempPath);
        ExitOnFailure(hr, "Failed to allocate memory for the temp path.");

        // Figure out path for this executable
        hr = PathForCurrentProcess(&sczStubExePath,NULL);
        ExitOnFailure(hr, "Failed to get path for current process.");

        // Find the manifest if it is attached
        hr = PayloadGetManifest(sczStubExePath, &pbManifest, &cbManifest, &sczBundleId, &dwPayloadsOffset, &dwAttachedContainerOffset);
        ExitOnFailure(hr, "Failed to get embedded manifest");

        hr = XmlLoadDocumentFromBuffer(pbManifest, cbManifest, &pixdManifest);
        ExitOnFailure(hr, "Failed to load manifest as XML document.");

        hr = XmlSelectSingleNode(pixdManifest, L"//Resource[@FileName='DHtmlHeader.html']", &pixnHeader);
        if (FAILED(hr) || S_FALSE == hr)
        {
            *pfHasHeader = FALSE;
        }
        else
        {
            *pfHasHeader = TRUE;
        }

LExit:
        ReleaseStr(sczBundleId);
        ReleaseStr(sczStubExePath);
        ReleaseStr(sczTempPath);
        ReleaseMem(pbManifest);
        ReleaseObject(pixdManifest);
        ReleaseObject(pixnHeader);

        return hr;
    }

    // Only supports embedded payloads and manifest
    template<typename TCmdLineSwitches>
    static HRESULT ExtractEngineData(PayloadType eplType)
    {
        HRESULT hr = S_FALSE; // Assume no embedded payload
        LPWSTR sczStubExePath = NULL;
        LPWSTR sczTempPath = NULL;
        LPWSTR sczWorkingPath = NULL;
        LPWSTR sczUniqueWorkingPath = NULL;
        LPBYTE pbManifest = NULL;
        DWORD cbManifest = 0;
        LPWSTR sczBundleId = NULL;
        IBurnPayload** rgpUXPayloads = NULL;
        IBurnPayload** rgpChainedPayloads = NULL;
        IXMLDOMDocument* pixdManifest = NULL;
        DWORD dwPayloadsOffset = 0;
        DWORD dwAttachedContainerOffset = 0;
        DWORD cUXPayloads = 0;
        DWORD cChainedPayloads = 0;
        DWORD_PTR cchTempPath = MAX_PATH;
        TCmdLineSwitches switches;
        bool fIsElevated = switches.IsRunningElevated();

        hr = StrAlloc(&sczTempPath, cchTempPath);
        ExitOnFailure(hr, "Failed to allocate memory for the temp path.");

        // Figure out path for this executable
        hr = PathForCurrentProcess(&sczStubExePath,NULL);
        ExitOnFailure(hr, "Failed to get path for current process.");

        // Find the manifest if it is attached
        hr = PayloadGetManifest(sczStubExePath, &pbManifest, &cbManifest, &sczBundleId, &dwPayloadsOffset, &dwAttachedContainerOffset);
        ExitOnFailure(hr, "Failed to get embedded manifest");

        if (!m_fHasFoldersSet)
        {
            if (!::GetTempPathW(cchTempPath, sczTempPath))
            {
                ExitWithLastError(hr, "Failed to get temp path.");
            }

            hr = PathConcat(sczTempPath, sczBundleId, &sczWorkingPath);
            ExitOnFailure(hr, "Failed to get bundle ID temp path");

            hr = PathCreateTempDirectory(sczWorkingPath, L"UX%d", 999999, &sczUniqueWorkingPath);
            ExitOnFailure(hr, "Failed to get unique temporary folder for UX");

            ModuleUtils::SetUxFolder(sczUniqueWorkingPath);
            ModuleUtils::SetParameterFileFolder(sczUniqueWorkingPath);
            ModuleUtils::SetIncludedPayloadsFolder(sczWorkingPath);

            m_fHasFoldersSet = true;
        }

        hr = XmlLoadDocumentFromBuffer(pbManifest, cbManifest, &pixdManifest);
        ExitOnFailure(hr, "Failed to load manifest as XML document.");

        hr = ParseManifestDocument(sczBundleId, sczStubExePath, sczUniqueWorkingPath, pixdManifest, dwPayloadsOffset, dwAttachedContainerOffset, &cChainedPayloads, &cUXPayloads, &rgpUXPayloads, &rgpChainedPayloads);
        ExitOnFailure(hr, "Failed to parse manifest document.");

        // If required loop over the attached payloads to get the local names, don't extract
        if ( cChainedPayloads > 0 && m_AttachedPayloadLocalNames.GetSize() == 0 )
        {
            hr = ExtractPayloads(sczBundleId,true,true, cChainedPayloads,rgpChainedPayloads);
            ExitOnFailure(hr, "Failed to store Local Names of Attached payloads.");
        }

        // If required loop over the Ux resources & ParameterInfo and extract them
        if (eplType == eplAllPayloads  ||  eplType == eplUxPayloads)
        {
            hr = ExtractPayloads(sczBundleId,fIsElevated, false, cUXPayloads,rgpUXPayloads);
            ExitOnFailure(hr, "Failed to extract Ux payloads.");
        }

        // If required loop over the attached payloads and extract them
        if (eplType == eplAllPayloads  ||  eplType == eplAttachedPayloads)
        {
            hr = ExtractPayloads(sczBundleId,fIsElevated, false, cChainedPayloads,rgpChainedPayloads);
            ExitOnFailure(hr, "Failed to extract Attached payloads.");
        }

LExit:
        ReleaseStr(sczTempPath);
        ReleaseStr(sczWorkingPath);
        ReleaseStr(sczUniqueWorkingPath);
        ReleaseNullStr(sczStubExePath);
        ReleaseStr(sczBundleId);
        ReleaseMem(pbManifest);
        ReleaseObject(pixdManifest);
        ReleaseObjectArray(rgpUXPayloads, cUXPayloads);
        ReleaseObjectArray(rgpChainedPayloads, cChainedPayloads);

        if ((SUCCEEDED(hr) || E_NOTFOUND == hr) && !ModuleUtils::HasEmbeddedContent())
        {
            // No error but there was nothing to extract
            LogStringLine(REPORT_STANDARD, "No embedded payloads found");
            hr = S_FALSE;
        }

        return hr;
    }

    // Loop over the array of payloads passed in and extract them to the appropriate location
    static HRESULT ExtractPayloads(
        __in_z LPWSTR sczBundleId,         // GUID ID of Bundle
        __in bool fRecordLocationOnly,   // Don't actually extract if true
        __in bool fStoreLocalName,       // only stores local name in m_AttachedPayloadLocalNames, does not extract
        __in DWORD cPayloads,            // Count of payloads
        __deref_in_ecount(cPayloads) IBurnPayload** rgpPayloads  // Array of cPayloads payloads
    )
    {
        HRESULT hr = S_OK;
        IBurnPayload* pPayload = NULL;
        LPWSTR sczSourcePath = NULL;
        LPWSTR sczWorkingPath = NULL;
        LPWSTR sczWorkingDir = NULL;
        LPWSTR sczEmbeddedId = NULL;
        LPWSTR sczExtractedPath = NULL;
        LPWSTR sczLocalName = NULL;
        SOURCE_TYPE sourceType = SOURCE_TYPE_EMBEDDED;
        DWORD64 dwEmbeddedContainerOffset = 0;

        // Loop over the payloads and extract them
        for (DWORD dwNextPayload = 0; dwNextPayload < cPayloads; ++dwNextPayload)
        {
            pPayload = rgpPayloads[dwNextPayload];
            hr = pPayload->GetSource(&sourceType, &sczSourcePath);
            ExitOnFailure(hr, "Failed to get source path for payload.");

            switch (sourceType)
            {
            case SOURCE_TYPE_EMBEDDED:
                hr = pPayload->GetLocalName(&sczLocalName);
                ExitOnFailure(hr, "Failed to get Local Name for Payload");

                // If fStoreLocalName is true, just store the local name, do not do anything else
                if (fStoreLocalName)
                {
                    m_AttachedPayloadLocalNames.Add(sczLocalName);
                }
                else
                {
                    // If running elevated then only extract ParameterInfo.xml again - the unelevated process has already done that
                    if (!fRecordLocationOnly || 0 == lstrcmpW(sczLocalName, L"ParameterInfo.xml"))
                    {
                        hr = pPayload->GetWorkingPath(&sczWorkingPath);
                        ExitOnFailure(hr, "Failed to get working path for payload.");

                        LogStringLine(REPORT_STANDARD, "Extracting embedded Ux from source '%ls' to working path '%ls'", sczSourcePath, sczWorkingPath);

                        hr = CacheEnsureWorkingDirectory(sczWorkingPath, &sczWorkingDir);
                        ExitOnFailure(hr, "Failed create working directory for payload.");

                        hr = pPayload->GetEmbeddedSource(&dwEmbeddedContainerOffset, &sczEmbeddedId);
                        ExitOnFailure1(hr, "Failed to query embedded payload information from: %S.", sczSourcePath);

                        hr = CabExtract(sczSourcePath, sczEmbeddedId, sczWorkingDir, NULL, NULL, dwEmbeddedContainerOffset);
                        ExitOnFailure1(hr, "Failed to extract embedded payload from: %S.", sczSourcePath);

                        // the file is extracted to the specified path with a name of its cabinet token, so rename it
                        hr = PathConcat(sczWorkingDir, sczEmbeddedId, &sczExtractedPath);
                        ExitOnFailure(hr, "Failed to create path to extracted payload.");

                        hr = FileEnsureMove(sczExtractedPath, sczWorkingPath, TRUE, TRUE);
                        ExitOnFailure(hr, "Failed to rename extracted payload to its working path.");
                    }

                    // Log that we have embedded content, and that we're extracting to the previously-known paths
                    SetHasEmbeddedContent();
                }
                break;
            case SOURCE_TYPE_DOWNLOAD:
                // Nothing to do for Packaging='download' type payloads
                break;
            case SOURCE_TYPE_EXTERNAL:
                // Nothing to do for Packaging='external' type payloads
                break;
            default:
                // Nothing to do for SOURCE_TYPE_NONE
                break;
            }

            ReleaseNullStr(sczSourcePath);
            ReleaseNullStr(sczWorkingPath);
            ReleaseNullStr(sczWorkingDir);
            ReleaseNullStr(sczExtractedPath);
            ReleaseNullStr(sczEmbeddedId);
            ReleaseNullStr(sczLocalName);
        }
LExit:

        ReleaseNullStr(sczSourcePath);
        ReleaseNullStr(sczWorkingPath);
        ReleaseNullStr(sczWorkingDir);
        ReleaseNullStr(sczExtractedPath);
        ReleaseNullStr(sczEmbeddedId);
        ReleaseNullStr(sczLocalName);

        return hr;
    }

    static bool IsPayloadEmbedded(const CString& csLocalName)
    {
        return ( -1 != m_AttachedPayloadLocalNames.Find(csLocalName) );
    }

    private:
        static CPath m_pthParameterFileFolder;
        static CPath m_pthUxFolder;
        static CPath m_pthIncludedPayloadsFolder;
        static bool m_fHasFoldersSet;
        static bool m_fHasEmbeddedContent;
        static CSimpleArray<CString> m_AttachedPayloadLocalNames;
};

}
