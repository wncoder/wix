//-------------------------------------------------------------------------------------------------
// <copyright file="SystemUtil.h" company="Microsoft">
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
//    This class is intented to be a a collection utility function related to system functionality
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "CustomErrors.h"

typedef void (WINAPI *pftGetNativeSystemInfo)(LPSYSTEM_INFO);

namespace IronMan
{
    //------------------------------------------------------------------------------
    //
    // This class is being refactored from the CSystemUtil class to help in Unit 
    // Testing.
    //
    //------------------------------------------------------------------------------
    class OSHelper
    {
    public:
        enum OSEnum 
        {
            osNone          = 0,
            osUnknown       = 1,
            osFuture        = 2,
            osWindows31     = 3,
            osWindowsNT_35  = 4,
            osWindowsMe     = 5,
            osWindows95     = 6,
            osWindows95R2   = 7,
            osWindows98     = 8,
            osWindows98SE   = 9,
            osWindowsNT     = 10,
            osWindows2000   = 11,
            osWindowsXP     = 12,
            osWindowsXP64   = 13,
            osWindows2003   = 14,
            osWindows2003R2 = 15,
            osWindowsVista  = 16,
            osWindows2008   = 17,
            osWindows7      = 18,
            osWindows2008R2 = 19,
        };

    private:
        SYSTEM_INFO m_si;

    public: 
        OSHelper(pftGetNativeSystemInfo pGetNativeSystemInfo =&::GetSystemInfo)
        {
            ZeroMemory(&m_si, sizeof(SYSTEM_INFO));
            //Don't need to check return value because the function does not return a value.
            pGetNativeSystemInfo(&m_si);
        }

        virtual ~OSHelper(){}

        const DWORD GetOS()
        {
            return static_cast<DWORD>(ComputeOSVersion());
        }

        const DWORD GetSP()
        {
            OSVERSIONINFOEX osvi;
            if (false == PopulateOSVersionInfo(osvi)) return 0;
            return osvi.wServicePackMajor;
        }

        bool IsVistaAndAbove()
        {
            OSVERSIONINFOEX osvi;
            if (PopulateOSVersionInfo(osvi))
            {
                return 6 <= osvi.dwMajorVersion;
            }
            return false;
        }

        const CString GetOSFullBuild()
        {
            OSVERSIONINFOEX osvi;
            if (false == PopulateOSVersionInfo(osvi)) return L"Error";
            CString csFullBuildNumber;
            csFullBuildNumber.Format(L"%d.%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
            return csFullBuildNumber;
        }    

        const CString GetOsAbbr()
        {   
            OSEnum os = ComputeOSVersion();

            switch(os)
            {
            case osUnknown :        return L"Unknown";
            case osFuture :         return L"Future OS";
            case osWindows31:       return L"Win3x";
            case osWindowsNT_35:    return L"WinNT35";  
            case osWindowsMe:       return L"WinME";
            case osWindows95:       return L"Win95";
            case osWindows95R2:     return L"Win95R2";
            case osWindows98:       return L"Win98";
            case osWindows98SE:     return L"Win98SE";
            case osWindowsNT:       return L"WinNT";
            case osWindows2000:     return L"Win2K";
            case osWindowsXP:       return L"WinXP";
            case osWindowsXP64:     return L"WinXP64";
            case osWindows2003:     return L"Win2K3";
            case osWindows2003R2:   return L"Win2K3R2";
            case osWindowsVista:    return L"Vista";
            case osWindows2008:     return L"Win2K8";
            case osWindows2008R2:   return L"Win2K8R2";
            case osWindows7:        return L"Windows 7";
            default: return L"None";
            }
        }

    public:
        //Returns the populated OSVERSIONINFOEX structure.
        const bool PopulateOSVersionInfo(OSVERSIONINFOEX& osvi)
        {
            ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

            // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
            // If that fails, try using the OSVERSIONINFO structure.
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            if ( !GetVersionEx (&osvi))
            {
                return false;
            }
            return true;
        }

    private:
        OSEnum ComputeOSVersion()
        {
            OSEnum os = osUnknown;  
            OSVERSIONINFOEX osvi;
            if (!PopulateOSVersionInfo(osvi)) return osUnknown;

            switch (osvi.dwPlatformId)
            {
                // Test for the Windows NT product family.
            case VER_PLATFORM_WIN32_NT:
                // Test for the specific product.
                if (6 == osvi.dwMajorVersion)
                {
                    if (0 == osvi.dwMinorVersion)
                    {
                        if(VER_NT_WORKSTATION == osvi.wProductType)
                        {
                            os = osWindowsVista;
                        }
                        else
                        {
                            os = osWindows2008;
                        }
                    }
                    else if (1 == osvi.dwMinorVersion)
                    {
                        if (VER_NT_WORKSTATION == osvi.wProductType)
                        {
                            os = osWindows7;
                        }
                        else
                        {
                            os = osWindows2008R2; // Windows Server 2008 R2
                        }
                    }
                    else
                    {
                        os = osFuture;
                    }                               
                } 
                else if (5 == osvi.dwMajorVersion) 
                {
                    switch (osvi.dwMinorVersion)
                    {
                    case 2: 
                        if (GetSystemMetrics(SM_SERVERR2) )
                        {
                            os = osWindows2003R2;
                        }
                        else if (VER_NT_WORKSTATION == osvi.wProductType && 
                            PROCESSOR_ARCHITECTURE_AMD64 == m_si.wProcessorArchitecture)
                        {
                            os = osWindowsXP64;
                        }
                        else 
                        {
                            os = osWindows2003;
                        }
                        break;
                    case 1:
                        os = osWindowsXP;
                        break;
                    case 0:
                        os = osWindows2000;
                        break;
                    }
                }
                else if (4 >= osvi.dwMajorVersion)
                {
                    os = osWindowsNT;
                }
                else
                {
                    os = osFuture;
                }
                break;

                //Note:  Not really need by no harm adding it.  
                // Test for the Windows Me/98/95.  
            case VER_PLATFORM_WIN32_WINDOWS:
                if (4 == osvi.dwMajorVersion)
                {
                    switch (osvi.dwMinorVersion)
                    {
                    case 0:
                        os = osWindows95;
                        if ('C' == osvi.szCSDVersion[1] || 'B' == osvi.szCSDVersion[1])
                        {
                            os = osWindows95R2;
                        }
                        break;
                    case 10:
                        os = osWindows98;
                        if ('A' == osvi.szCSDVersion[1] || 'B' == osvi.szCSDVersion[1])
                        {
                            os = osWindows98SE;
                        }
                        break;
                    case 90:
                        os = osWindowsMe;
                        break;
                    default:
                        os = osUnknown;
                        break;
                    }
                }
                else
                {
                    os = osUnknown;
                }
                break;

            case VER_PLATFORM_WIN32s:
                os = osWindows31;
                break;
            default:
                os = osUnknown;
            }
            return os;
        }
    private: 
        //Sub-class for overwritting in Unit Test
        virtual BOOL GetVersionEx(OSVERSIONINFOEX* osie) { return ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(osie)); }
    };

    class ProductInfo
    {
        DWORD dwProductType;
    public:
        ProductInfo(const OSVERSIONINFOEX& osvi) 
            : dwProductType(PRODUCT_UNDEFINED)
        {
            typedef BOOL (WINAPI *PGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
            PGetProductInfo pGetProductInfo = (PGetProductInfo) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
            
            if (pGetProductInfo)
                pGetProductInfo(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.wServicePackMinor, &dwProductType);
        }

        LPCWSTR GetEdition()
        {
            switch( dwProductType )
            {
            case PRODUCT_BUSINESS:                      return L"Business Edition";
            case PRODUCT_BUSINESS_N:                    return L"Business (N) Edition";
            case PRODUCT_CLUSTER_SERVER:                return L"Cluster Server Edition";
            case PRODUCT_DATACENTER_SERVER:             return L"Datacenter Edition";
            case PRODUCT_DATACENTER_SERVER_CORE:        return L"Datacenter Edition (core installation)";
            case PRODUCT_ENTERPRISE:                    return L"Enterprise Edition";
            case PRODUCT_ENTERPRISE_SERVER:             return L"Enterprise Edition";
            case PRODUCT_ENTERPRISE_SERVER_CORE:        return L"Enterprise Edition (core installation)";
            case PRODUCT_ENTERPRISE_SERVER_IA64:        return L"Enterprise Edition for Itanium-based Systems";
            case PRODUCT_HOME_BASIC:                    return L"Home Basic Edition";
            case PRODUCT_HOME_BASIC_N:                  return L"Home Basic (N) Edition";
            case PRODUCT_HOME_PREMIUM:                  return L"Home Premium Edition";
            case PRODUCT_HOME_SERVER:                   return L"Home Premium Server Edition";
            case PRODUCT_SERVER_FOR_SMALLBUSINESS:      return L"Server for Small Business Edition";
            case PRODUCT_SMALLBUSINESS_SERVER:          return L"Small Business Server";
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:  return L"Small Business Server Premium Edition";
            case PRODUCT_STANDARD_SERVER:               return L"Standard Edition";
            case PRODUCT_STANDARD_SERVER_CORE:          return L"Standard Edition (core installation)";
            case PRODUCT_STARTER:                       return L"Starter Edition";
            case PRODUCT_STORAGE_ENTERPRISE_SERVER:     return L"Storage Server Enterprise Edition";
            case PRODUCT_STORAGE_EXPRESS_SERVER:        return L"Storage Server Express Edition";
            case PRODUCT_STORAGE_STANDARD_SERVER:       return L"Storage Server Standard Edition";
            case PRODUCT_STORAGE_WORKGROUP_SERVER:      return L"Storage Server Workgroup Edition";
            case PRODUCT_ULTIMATE:                      return L"Ultimate Edition"; 
            case PRODUCT_WEB_SERVER:                    return L"Web Server Edition"; 
            case PRODUCT_UNLICENSED:                    return L"Unlicensed Edition"; 
            }

            CString str;
            str.Format(L"Unkonw Edition (ProductType=%d)", dwProductType);
            return str;
        }

    };


    class CSystemUtil
    {
    public:

        //------------------------------------------------------------------------------
        // GetErrorString
        //
        // Gets textual version of error and also prints it to the debug output window
        //-------------------------------------------------------------------------------
        static void GetErrorString( DWORD dwError, CComBSTR &bstrError )
        {
            // If it is a IronMan custom error, return IronMan's error string(not localized)
            CString customErrorString(CustomErrors::GetErrorString(dwError));
            if ( !customErrorString.IsEmpty() )
            {
                //      Display the string in a message box
                OutputDebugString(customErrorString);
                bstrError = customErrorString;
            }
            else
            {
                LPWSTR lpszMessageBuffer = NULL;
                DWORD dwMessageLength = 0;

                // Security OACR note: format string should never be user data else it can be exploited
                // OACR_REVIEWED_CALL( <alias>,
                __pragma ( prefast(suppress: 25028, "reviewed and correct") )
                    dwMessageLength = ::FormatMessageW( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    reinterpret_cast<LPWSTR>(&lpszMessageBuffer),
                    0,
                    NULL );

                if ( dwMessageLength )
                {
                    //      Display the string in a message box
                    OutputDebugString(lpszMessageBuffer);
                    bstrError = lpszMessageBuffer;
                    //      Free the buffer.
                    LocalFree( lpszMessageBuffer );
                }
            }
        }

        //------------------------------------------------------------------------------
        // ExpandEnvironmentVariables
        //
        // 
        // Expand environment varoables, e.g. %TEMP%\LogFile.log
        // becomes C:\Users\someone\AppData\Local\Temp\LogFile.log
        //------------------------------------------------------------------------------
        static BOOL ExpandEnvironmentVariables(const CString& strString, CString& strExpandedString)
        {
            BOOL fResult = true;
            LPWSTR pszBuffer = strExpandedString.GetBuffer(MAX_PATH + 1);
            DWORD numChars = ::ExpandEnvironmentStrings(strString, pszBuffer, MAX_PATH + 1);
            if (numChars > (MAX_PATH + 1))
            {
                strExpandedString._ReleaseBuffer(0);
                pszBuffer = strExpandedString.GetBuffer(numChars);
                numChars = ::ExpandEnvironmentStrings(strString, pszBuffer, numChars);
            }
            if ((0 == numChars) || (strString == strExpandedString))
            {
                fResult = false;
            }
            strExpandedString._ReleaseBuffer(numChars == 0? numChars: numChars-1);

            return fResult;
        }

        //------------------------------------------------------------------------------
        // GetOSAbbr
        //
        // 
        // Retrieve the OS abbr - For Example: XP, W2K3, W2K
        //------------------------------------------------------------------------------
        static CString GetOSAbbr()
        {
            HMODULE hmodule = GetModuleHandle(TEXT("kernel32.dll"));
            if (NULL == hmodule)
            {
                return L"";
            }
            pftGetNativeSystemInfo pGNSI = reinterpret_cast<pftGetNativeSystemInfo>(GetProcAddress(hmodule, "GetNativeSystemInfo"));
            if (NULL == pGNSI)
            {
                pGNSI = &::GetSystemInfo;
            }

            OSHelper helper(pGNSI);
            return helper.GetOsAbbr();
        }

        static CString GetOSDescription()
        {
            OSVERSIONINFOEX osvi;
            OSHelper osh;
            if (false == osh.PopulateOSVersionInfo(osvi))
                return L"Unknown OS";

            SYSTEM_INFO si={0};
            GetSystemInformation(si);

            CString str;
            str.Format(L"%s - %s %s %s", GetOSAbbr(), GetCPUArchitecture(si), GetEdition(osvi), osvi.szCSDVersion);
            return str;
        }

        //------------------------------------------------------------------------------
        // CreateDirectoryIfNeeded
        //
        // 
        // Create the directory if needed
        //------------------------------------------------------------------------------
        static HRESULT CreateDirectoryIfNeeded(CPath filepath)
        {
            CPath directory = filepath;
            BOOL fCheck = directory.RemoveFileSpec();
            if (!fCheck)
            {
                //LOG(m_logger, ILogger::Error, "CPath::RemoveFileSpec failed!"); 
                return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            }   

            if (!::PathFileExists(directory))
            {
                int iCheck = ::SHCreateDirectoryEx(NULL, directory.m_strPath, NULL);
                if (ERROR_SUCCESS != iCheck)
                {
                    //LOG(m_logger, ILogger::Error, "CPath::RemoveFileSpec failed!"); 
                    return HRESULT_FROM_WIN32(iCheck);
                }
            }
            return S_OK;
        }

        //Returns the populated SYSTEM_INFO structure 
        //It is public as it is a common code.
        static void GetSystemInformation(SYSTEM_INFO& si)
        {
            typedef void (WINAPI *pftGetNativeSystemInfo)(LPSYSTEM_INFO);
            pftGetNativeSystemInfo pfGetNativeSystemInfo = NULL;

            HMODULE hmodule = GetModuleHandle(TEXT("kernel32.dll"));
            if (NULL == hmodule)
            {                
                return;
            }

            // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
            pfGetNativeSystemInfo =  reinterpret_cast<pftGetNativeSystemInfo>(GetProcAddress(hmodule,"GetNativeSystemInfo"));
            if(NULL != pfGetNativeSystemInfo)
            {
                pfGetNativeSystemInfo(&si);
            }
            else
            {
                GetSystemInfo(&si);
            }
        }

        //------------------------------------------------------------------------------
        // GetCPUArchitecture
        //
        //
        // Retrieve the CPU Architecture - For Example: x86, I64, A64
        //------------------------------------------------------------------------------
        static CString GetCPUArchitecture()
        {
            SYSTEM_INFO si = {0};
            GetSystemInformation(si);
            return GetCPUArchitecture(si);
        }

        //------------------------------------------------------------------------------
        // GetOSCompatibilityMode
        //
        //
        // Retrieve the OS CompatibilityMode
        // Note: Should not use __COMPAT_LAYER environment as it may be populated with 
        //       other values beside OS lie.
        //------------------------------------------------------------------------------
        static bool IsInOSCompatibilityMode()
        {
            bool bRealOS = false;
            OSVERSIONINFOEX osVersion = {0};
            osVersion.dwOSVersionInfoSize = sizeof(osVersion);

            if ( GetVersionEx( reinterpret_cast<OSVERSIONINFO*>(&osVersion) ) )
            {
                DWORDLONG dwlConditionMask = 0;
                int iOperation = VER_EQUAL;
                VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, iOperation );
                VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, iOperation );
                VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, iOperation );
                VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, iOperation );

                bRealOS = ::VerifyVersionInfo(&osVersion,
                                                   VER_MAJORVERSION | VER_MINORVERSION |
                                                   VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
                                                   dwlConditionMask);
            }
            return !bRealOS;
        }

    private:
        static CString GetCPUArchitecture(const SYSTEM_INFO& si)
        {
            CString strCPUArchitecture(L"Unknown");

            switch (si.wProcessorArchitecture)
            {
            case PROCESSOR_ARCHITECTURE_AMD64:   // x64 (AMD or Intel) 
                strCPUArchitecture = L"x64";
                break;
            case PROCESSOR_ARCHITECTURE_IA64:    // Intel Itanium Processor Family (IPF) 
                strCPUArchitecture = L"IA64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:   // x86 
                strCPUArchitecture = L"x86";
                break;
            }

            return strCPUArchitecture;
        }   

        static CString GetEdition(const OSVERSIONINFOEX& osvi)
        {
            if (osvi.dwMajorVersion >= 6)
            {
                ProductInfo pi(osvi);
                return pi.GetEdition();
            }

            if (osvi.wProductType != VER_NT_WORKSTATION)
            {
                if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
                    return L"Compute Cluster Edition";
                else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    return L"Datacenter Edition";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    return L"Enterprise Edition";
                else if (osvi.wSuiteMask & VER_SUITE_BLADE)
                    return L"Web Edition";
                else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
                    return L"Storage Edition";
                //else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
                //    return L"Home Server Edition";
                else 
                    return L"Standard Edition";
            }
            else
            {
                if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
                    return L"Home Edition";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    return L"Enterprise Edition";
                else 
                    return L"Professional";
            }
        }
    };
}
