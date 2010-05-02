//-------------------------------------------------------------------------------------------------
// <copyright file="MSIUtils.h" company="Microsoft">
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

#include "interfaces\IProgressObserver.h"
#include "MsiTableUtils.h"
#include "ModuleUtils.h"

namespace IronMan
{
    class MSIUtils
    {
    public:

        //------------------------------------------------------------------------------
        // IsSuccess
        //
        //  The error codes ERROR_SUCCESS, ERROR_SUCCESS_REBOOT_INITIATED, and 
        //  ERROR_SUCCESS_REBOOT_REQUIRED are indicative of success. If 
        //  ERROR_SUCCESS_REBOOT_REQUIRED is returned, the installation completed successfully 
        //  but a reboot is required to complete the installation operation.
        //------------------------------------------------------------------------------
        static bool IsSuccess(HRESULT hr)
        {
            return ((S_OK == hr) ||
                (HRESULT(ERROR_SUCCESS_REBOOT_REQUIRED) == hr) ||  //There are cases where 3010 is not converted to HRESULT
                (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr) ||
                (HRESULT_FROM_WIN32(ERROR_SUCCESS_RESTART_REQUIRED) == hr) ||
                (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr) ||
                (NOTHING_APPLIES == hr));
        }

        //------------------------------------------------------------------------------
        // GetProductNameFromProductGuid
        //
        // This function is used to retrieve the Productname from ProductGuid.
        //------------------------------------------------------------------------------
        static CString GetProductNameFromProductGuid(const CString& csProductGuid)
        {
            WCHAR wszTargetPackageName[MAX_PATH] = { 0 };
            DWORD cchTargetPackageInstallPath = MAX_PATH;
            UINT err = MsiGetProductInfo(csProductGuid, INSTALLPROPERTY_PRODUCTNAME, wszTargetPackageName, &cchTargetPackageInstallPath);
            return (ERROR_SUCCESS == err) ? CString(wszTargetPackageName) : L"";
        }

        //------------------------------------------------------------------------------
        // IsProductInstalled
        //
        // This function is determine if a product has been applied.
        //------------------------------------------------------------------------------
        static bool IsProductInstalled(const CString& csProductGuid)
        {
            return GetProductNameFromProductGuid(csProductGuid) != L"";
        }

        // Set logging level and log name, which is based on the main log name with the specified suffix added at the end
        static CString SetMsiLoggingParameters(const CString & strSuffix, ILogger& logger)
        {
            CString strMsiLogFile;
            CPath pthLogFile = logger.GetFilePath();
            // Verify that the path to the log file is not empty
            // If it is a NullLogger, then we don't want to call MsiEnableLog with a bad path
            // since that would cause the log not to be written.
            if ( !CString(pthLogFile).IsEmpty() )
            {
                // Strip the file name out of the path
                pthLogFile.RemoveFileSpec();
                // Add the file name back in along with the supplied suffix
                strMsiLogFile = CString(pthLogFile) + CString(L"\\") + ModuleUtils::GetFileNameOnlyWithoutExtension(logger.GetFilePath()) + strSuffix + L".txt";

                // Set the MSI log file spec and level
                UINT uiRet = MsiEnableLog(
                    // The logging levels here are the same as Brooklyn
                    INSTALLLOGMODE_FATALEXIT |
                    INSTALLLOGMODE_ERROR |
                    INSTALLLOGMODE_WARNING |
                    INSTALLLOGMODE_USER |
                    INSTALLLOGMODE_INFO |
                    INSTALLLOGMODE_RESOLVESOURCE |
                    INSTALLLOGMODE_OUTOFDISKSPACE |
                    INSTALLLOGMODE_ACTIONSTART |
                    INSTALLLOGMODE_ACTIONDATA |
                    INSTALLLOGMODE_COMMONDATA |
                    INSTALLLOGMODE_PROPERTYDUMP |
                    INSTALLLOGMODE_VERBOSE,
                    strMsiLogFile,
                    INSTALLLOGATTRIBUTES_APPEND);
                if (uiRet != ERROR_SUCCESS)
                {
                    CString strError(L"Error calling MsiEnableLog with log file set to ");
                    strError += strMsiLogFile;
                    LOG(logger, ILogger::Error, strError);
                }
                else
                {
                    CString strMessage(L"Successfully called MsiEnableLog with log file set to ");
                    strMessage += strMsiLogFile;
                    LOG(logger, ILogger::Verbose, strMessage);

                    // Add the Msi log file to the list of files for Watson to send
                    LOG(logger, ILogger::InternalUseOnly, strMsiLogFile);
                }
            }
            return strMsiLogFile;
        }


        // Gets All the Features, ';' delemited of the given product.
        // If bAdvertisedFeaturesOnly is true, the list is filtered to only features that are
        // advertised.
        static CString GetFeatures(const CString& product, bool bAdvertisedFeaturesOnly = false)
        {
            CString features;
            for (DWORD j=0; true; ++j)
            {
                WCHAR featureName[MAX_FEATURE_CHARS+1] = {0};
                if (ERROR_SUCCESS != MsiEnumFeatures(product, j, featureName, NULL))
                    break;

                if (bAdvertisedFeaturesOnly == false || ::MsiQueryFeatureState(product, featureName) == INSTALLSTATE_ADVERTISED)
                {
                    if (features.IsEmpty())
                        features = featureName;
                    else
                        features += CString(L",") + featureName;
                }
            }
            return features;
        }
        
        // Returns ';' delimited advertised features for given product.
        static CString GetAdvertisedFeatures(const CString& product)
        {
            return GetFeatures(product, true);
        }

        // Return true if version string contains digits and dots only
        static bool IsAllNumeric(const CString& cs)
        {
            for(int i=0; i<cs.GetLength(); ++i)
            {
                if ((cs[i] >= L'0') && (cs[i] <= L'9'))
                    continue;
                if (cs[i] == L'.')
                    continue;
                return false;
            }
            return true;
        }

        // Returns number of "."s in a version string.
        // E.g: 3 for 1.1.1.1001
        static int DotCount(const CString& cs)
        {
            int dots = 0;
            for(int i=0; i<cs.GetLength(); ++i)
            {
                if (cs[i] == L'.')
                    ++dots;
            }
            return dots;
        }

        // Takes two vesrions strings and makes them same length by padding appropriate 0.
        // Eg: (1.2,     1.11) -> (1.02,    1.11)
        //     (1.1.2    1.2)  -> (1.1.2,   1.2.0)
        static void NormalizeVersions(const CString& lhs, const CString& rhs, CString& strLHS, CString& strRHS, ILogger& logger)
        {
            if (IsAllNumeric(lhs) && IsAllNumeric(rhs))
            {
                CString tlhs(lhs);
                CString tvalue(rhs);

                // special leading and trailing . handling
                if (tlhs[0]   == L'.')
                    tlhs = L"0" + tlhs;
                if (tvalue[0] == L'.')
                    tvalue = L"0" + tvalue;
                if (!tlhs.IsEmpty()  && tlhs  [  tlhs.GetLength()-1] == L'.')
                    tlhs += L"0";
                if (!tvalue.IsEmpty() && tvalue[tvalue.GetLength()-1] == L'.')
                    tvalue += L"0";

                // equalize the number of dots, by appending
                while (DotCount(tlhs) < DotCount(tvalue))
                    tlhs += L".0";
                while (DotCount(tvalue) < DotCount(tlhs))
                    tvalue += L".0";

                int leftStart=0, rightStart=0;

                for(;;)
                {
                    CString leftPart  = tlhs.Tokenize(L".", leftStart);
                    CString rightPart = tvalue.Tokenize(L".", rightStart);

                    if ((leftStart == -1) || (rightStart == -1))
                    {
                        if ((leftStart != -1) || (rightStart != -1))
                        {	// I've tried and I've tried to get the number of dots to match,
                            // but just in case (for instance, two dots in a row),
                            // handle anything unrecognizable lexically
                            strLHS  = L"." + lhs;
                            strRHS = L"." + rhs;
                            LOG(logger, ILogger::Verbose, L"unrecognizable numeric - not canonicalizing");
                        }
                        else
                        {
                            LOG(logger, ILogger::Verbose, L"all numeric characters - canonicalizing");
                        }
                        break;
                    }

                    // pad segment with leading 0s
                    while(leftPart.GetLength() < rightPart.GetLength())
                        leftPart = L"0" + leftPart;
                    while(rightPart.GetLength() < leftPart.GetLength())
                        rightPart = L"0" + rightPart;

                    strLHS  += L"." + leftPart;
                    strRHS += L"." + rightPart;
                }
                strLHS  = strLHS.Mid(1); // strip off leading .s
                strRHS = strRHS.Mid(1);
            }
            else
            {
                strLHS = lhs;
                strRHS = rhs;
            }
        }

        // Zero if the strings are identical, < 0 if LHS is less than RHS, or > 0 if RHS is greater than LHS.
        static int CompareVersionStrings(const CString lhs, const CString rhs, ILogger& logger = NullLogger::GetNullLogger())
        {
            CString lhsOut;
            CString rhsOut;
            MSIUtils::NormalizeVersions(lhs, rhs, lhsOut, rhsOut, logger);
            return lhsOut.Compare(rhsOut);
        }

        static bool GetMSIProductVersion(const CString& productCode, CString& value, ILogger& logger)
        {
            return GetMSIProductVersionT(productCode, logger, value, MsiGetProductInfo);
        }

        template <typename F1>
        static bool GetMSIProductVersionT(const CString& productCode, ILogger& logger, CString& value, F1 MsiGetProductInfo)
        {
            value.Empty();

            DWORD dwCount = 0;
            UINT err = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, NULL, &dwCount);
            if (err == ERROR_SUCCESS)
            {
                dwCount++;
                err = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, value.GetBuffer(dwCount), &dwCount);
                // assert count was big enough
                value._ReleaseBuffer();
            }
            if (err != ERROR_SUCCESS)
            {
                LOG(logger, ILogger::Information, L"MsiGetProductInfo with product code " + productCode + L" found no matches");
                return false;
            }
            else
            {
                LOG(logger, ILogger::Information, L"MsiGetProductInfo with product code " + productCode + L", returned: " + value);
                return true;
            }
        }

        static bool IsMinVersionSatisfied(const CString& installedVersion, const CString& minVersion, bool includeVersion, ILogger& logger)
        {
            if (minVersion.IsEmpty())
                return true;
            CString lhsMinVersion;
            CString rhsInstalledVersion;

            NormalizeVersions(minVersion, installedVersion, lhsMinVersion, rhsInstalledVersion, logger);
            if (lhsMinVersion < rhsInstalledVersion)
                return true;
            if (includeVersion)
                return lhsMinVersion == rhsInstalledVersion;
            return false;
        }

        static bool IsMaxVersionSatisfied(const CString& installedVersion, const CString& maxVersion, bool includeVersion, ILogger& logger)
        {
            if (maxVersion.IsEmpty())
                return true;
            CString lhsMaxVersion;
            CString rhsInstalledVersion;

            NormalizeVersions(maxVersion, installedVersion, lhsMaxVersion, rhsInstalledVersion, logger);
            if (rhsInstalledVersion < lhsMaxVersion)
                return true;
            if (includeVersion)
                return lhsMaxVersion == rhsInstalledVersion;
            return false;
        }

        // Returns array of product codes of all products that have given upgrade code specified.
        static int GetRelatedProducts(
               const CString& upgradeCode // in
             , const CSimpleArray<CString>& productCodesToSkip //in
             , const CString& minVersion //in
             , bool  minVersionInclusive //in 
             , const CString& maxVersion //in 
             , bool  maxVersionInclusive //in 
             , ILogger& logger //in
             , CSimpleArray<CString>& productCodes) //out 
        {
            int index = 0;
            if (!upgradeCode.IsEmpty())
            {
                CString productCode;
                while (::MsiEnumRelatedProducts(upgradeCode, 0, index, productCode.GetBuffer(MAX_PATH)) == ERROR_SUCCESS)
                {
                    productCode._ReleaseBuffer();
                    if (productCodesToSkip.Find(productCode) == -1 && productCodes.Find(productCode) == -1)
                    {
                        bool addProduct = true;
                        CString installedVersion;
                        // Check for versions applicability if either or both of them are authored.
                        if (!(minVersion.IsEmpty() && maxVersion.IsEmpty())) 
                        {
                            bool versionFound = GetMSIProductVersion(productCode, installedVersion, logger);
                            bool minVersionSatisfied = false;
                            bool maxVersionSatisfied = false;
                            if (versionFound)
                            {
                                minVersionSatisfied = IsMinVersionSatisfied(installedVersion, minVersion, minVersionInclusive, logger);
                                maxVersionSatisfied = IsMaxVersionSatisfied(installedVersion, maxVersion, maxVersionInclusive, logger);
                                addProduct = minVersionSatisfied && maxVersionSatisfied;
                            }
                            else
                            {
                                addProduct = false;
                            }
                        }
                        if (addProduct)
                        {
                            productCodes.Add(productCode);
                        }
                        else
                        {
                            LOG(logger, ILogger::Verbose, productCode + L" " + installedVersion + L" skipped after applying Relation criteria" );
                        }
                    }
                    index++;
                }
            }
            return index;
        }

        // ----------------------------------------------------------------------------------------
        // GetMspDisplayName()
        // returns the DisplayName of the MSP from the MsiPatchMetadata table
        // if it cannot get the DisplayName from the Patch, it returns the Patch Code
        // ----------------------------------------------------------------------------------------
        static CString GetMspDisplayName(const CString& strPatchCode, ILogger& logger)
        {
            return GetMspDisplayNameT( strPatchCode, logger, MSIUtils::GetMspLocalPackage);
        }

        // ----------------------------------------------------------------------------------------
        // GetMspDisplayNameT()  DO NOT CALL DIRECTLY, FOR UNIT TESTING ONLY
        // returns the DisplayName of the MSP from the MsiPatchMetadata table
        // if it cannot get the DisplayName from the Patch, it returns the Patch Code
        // ----------------------------------------------------------------------------------------
        template <typename F1>
        static CString GetMspDisplayNameT(const CString& strPatchCode, ILogger& logger, F1 GetMspLocalPackage)
        {
            CString strDisplayName;
            CString strLocalCachedPackage;
            UINT err = GetMspLocalPackage(strPatchCode, strLocalCachedPackage, logger);
            if ( ERROR_SUCCESS != err)
            {
                // Return the Patch Code if unable to get the MSP DisplayName
                strDisplayName = strPatchCode;
            }
            else
            {
                strDisplayName = GetMspDisplayNameFromLocalPackage(strLocalCachedPackage, logger);
                //Translate to PatchCode if there is a failure.
                if (strDisplayName == strLocalCachedPackage)
                {
                    strDisplayName = strPatchCode;
                }
            }
            return strDisplayName;
        }

        // ----------------------------------------------------------------------------------------
        // GetMspDisplayNameFromLocalPackage()
        // returns the DisplayName of the MSP from the MsiPatchMetadata table
        // if it cannot get the DisplayName from the Patch, it returns the Patch Code
        // ----------------------------------------------------------------------------------------
        static CString GetMspDisplayNameFromLocalPackage(const CString& strLocalCachedPackage, ILogger& logger)
        {
            CString strDisplayName;

            MsiTableUtils msiTableUtils(MsiTableUtils::CreateMsiTableUtils(strLocalCachedPackage, MsiTableUtils::MspFile, logger));
            UINT err = msiTableUtils.ExecuteScalar(L"Value", L"MsiPatchMetadata", L"`Property` = 'DisplayName'", strDisplayName);
            if ( ERROR_SUCCESS != err)
            {
                // Return the Patch Code if unable to get the MSP DisplayName
                strDisplayName = strLocalCachedPackage;
            }
            return strDisplayName;
        }

        // ----------------------------------------------------------------------------------------
        // GetMspLocalPackage()
        // returns the local cached package
        // ----------------------------------------------------------------------------------------
        static UINT GetMspLocalPackage(const CString& patchCode
                                    , CString& localCachedPackage
                                    , ILogger& logger)
        {
            return GetMspLocalPackageT( patchCode, localCachedPackage, logger, ::MsiGetPatchInfo);
        }

        // ----------------------------------------------------------------------------------------
        // GetMspLocalPackageT()  DO NOT CALL DIRECTLY, FOR UNIT TESTING ONLY
        // returns the local cached package
        // ----------------------------------------------------------------------------------------
        template <typename F1>
        static UINT GetMspLocalPackageT(  const CString& patchCode
                                        , CString& localCachedPackage
                                        , ILogger& logger
                                        , F1 MsiGetPatchInfo)
        {
            UINT uiRet = ERROR_SUCCESS;
            DWORD cchBufferSize = 0;
            localCachedPackage.Truncate(0);
            uiRet = MsiGetPatchInfo(  patchCode
                                    , INSTALLPROPERTY_LOCALPACKAGE
                                    , localCachedPackage.GetBuffer(cchBufferSize)
                                    , &cchBufferSize);
            localCachedPackage._ReleaseBuffer();
            if (ERROR_MORE_DATA == uiRet)
            {
                cchBufferSize++; // add one for the null terminator
                uiRet = MsiGetPatchInfo(  patchCode
                                        , INSTALLPROPERTY_LOCALPACKAGE
                                        , localCachedPackage.GetBuffer(cchBufferSize)
                                        , &cchBufferSize);
                localCachedPackage._ReleaseBuffer();
            }
                return uiRet;
        }

        // Helper to log results.
        static void ProcessReturnValue(UINT err, const CString& itemType, ILogger& logger, const CString& action, const CString& strProductName, const CString& strMsiLogFile, bool& needsReboot)
        {
            CString strLogPrefix;
            CString strResult;
            strLogPrefix.Format(L"Return value - 0x%X", err);

            switch(err)
            {
            case ERROR_UNKNOWN_PRODUCT: // not an error if the product isn't installed
                LOG(logger, ILogger::Verbose, strLogPrefix + L": ERROR_UNKNOWN_PRODUCT (not actually an error - patch does not apply to this product)");
                break;
            case ERROR_SUCCESS:
                //Logging Result
                strResult.Format(L"%s (%s) succeeded on product (%s).  Msi Log: <a href=\"%s\">%s</a>",itemType,  action, strProductName, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile)); 
                LOG(logger, ILogger::Result, strResult);
                LOG(logger, ILogger::Verbose, strLogPrefix + L":  no error");
                break;
            case ERROR_SUCCESS_REBOOT_REQUIRED:
                //Logging Result
                strResult.Format(L"%s (%s) succeeded on product (%s) and requires reboot.  Msi Log: <a href=\"%s\">%s</a>",itemType,  action, strProductName, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));  
                LOG(logger, ILogger::Result, strResult);
                LOG(logger, ILogger::Verbose, strLogPrefix + L":  ERROR_SUCCESS_REBOOT_REQUIRED");
                needsReboot = true;
                break;
            case ERROR_SUCCESS_REBOOT_INITIATED:  
                //Logging Result
                strResult.Format(L"%s (%s) succeeded on product (%s) and a reboot has been initiated!!!!.  Msi Log: <a href=\"%s\">%s</a>",itemType,  action, strProductName, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));  
                LOG(logger, ILogger::Result, strResult);   
                LOG(logger, ILogger::Verbose, strLogPrefix + L":  ERROR_SUCCESS_REBOOT_INITIATED");
                break;
            case ERROR_SUCCESS_RESTART_REQUIRED:
                //Logging Result
                strResult.Format(L"%s (%s) succeeded on product (%s) and requires the service to be restarted.  Msi Log: <a href=\"%s\">%s</a>",itemType,  action, strProductName, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));  
                LOG(logger, ILogger::Result, strResult);
                LOG(logger, ILogger::Verbose, strLogPrefix + L":  ERROR_SUCCESS_RESTART_REQUIRED");
                break;
            default:
                //Logging Result
                strResult.Format(L"%s (%s) failed on product (%s).  Msi Log: <a href=\"%s\">%s</a>",itemType,  action, strProductName, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));   
                LOG(logger, ILogger::Result, strResult);
                LOG(logger, ILogger::Error, strLogPrefix);
                break;
            }
        }

        // ----------------------------------------------------------------------------------------
        // GetWindowsInstallerVersion()
        // Returns the version of the Windows Installer.
        // ----------------------------------------------------------------------------------------
        static CString GetWindowsInstallerVersion()
        {
            CString strSystem32Directory;
            CString strWindowsInstallerVersion;
            if (SUCCEEDED(::SHGetFolderPath(NULL
                                            , CSIDL_SYSTEM, NULL
                                            , SHGFP_TYPE_CURRENT
                                            , strSystem32Directory.GetBuffer(MAX_PATH))))
            {
                strSystem32Directory._ReleaseBuffer();
                CPath pthMsiPath;
                pthMsiPath.Combine(strSystem32Directory, L"MSI.dll");

                DWORD dwBufferSize = MAX_PATH;
                //Not checking the return value as this is an best effort for user experience data logging.
                ::MsiGetFileVersion(  CString(pthMsiPath)
                                    , strWindowsInstallerVersion.GetBuffer(dwBufferSize)
                                    , &dwBufferSize
                                    , 0
                                    , 0);
                strWindowsInstallerVersion._ReleaseBuffer();
            }
            return strWindowsInstallerVersion;
        }
    };
}
