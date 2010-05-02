//-------------------------------------------------------------------------------------------------
// <copyright file="VersionUtil.h" company="Microsoft">
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

namespace IronMan
{
    class CVersionUtil
    {
        public:
            //------------------------------------------------------------------------------
            // GetExeFileVersion
            //
            // 
            // This function is written to retrieve the file version of the calling exe. 
            //------------------------------------------------------------------------------
            static CString GetExeFileVersion(HMODULE hModule = NULL)
            {
                CString strExeFileVersion = L"0.0.0.0";
                CString strExeFilePath;
                DWORD dwResult = ::GetModuleFileName(hModule, strExeFilePath.GetBuffer(MAX_PATH + 1), MAX_PATH);
                strExeFilePath._ReleaseBuffer();

                DWORD dwHandle = 0;
                DWORD dwLength = ::GetFileVersionInfoSize(strExeFilePath, &dwHandle);
                if (dwLength != 0)
                {	
                    void* pvVerInfo = new BYTE[dwLength];
                    if (::GetFileVersionInfo(strExeFilePath, dwHandle, dwLength, pvVerInfo))
                    {
                        VS_FIXEDFILEINFO* pFixedFileInfo;
                        UINT cbFixedFileInfo;
                        if (::VerQueryValue(pvVerInfo, L"\\", reinterpret_cast<void**>(&pFixedFileInfo), &cbFixedFileInfo))
                        {
                            strExeFileVersion.Format(	L"%d.%d.%d.%d", 
                                                        pFixedFileInfo->dwFileVersionMS >> 16,
                                                        pFixedFileInfo->dwFileVersionMS & 0xFFFF,
                                                        pFixedFileInfo->dwFileVersionLS >> 16,
                                                        pFixedFileInfo->dwFileVersionLS & 0xFFFF);
                        }									
                    }
                    delete[] pvVerInfo;
                }
                return strExeFileVersion;
            }		
    };
}
