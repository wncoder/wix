//-------------------------------------------------------------------------------------------------
// <copyright file="Operands.h" company="Microsoft">
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

//#include "Interfaces\IDataProviders.h"

#include "Common\XmlUtils.h"
#include "Common\SystemUtil.h"
#include "ModuleUtils.h"
#include "Common\IronManAssert.h"
#include "Common\CoInitializer.h"
#include "interfaces\IExceptions.h"
#include "LogSignatureDecorator.h"
#include "MsiXmlBlobBase.h"
#include "CmdLineParser.h"
#include "common\CoInitializer.h"

namespace IronMan
{

class Operand
{
    bool m_bSupportsLessThanGreaterThan;
public:
    Operand(bool bSupportsLessThanGreaterThan)
        : m_bSupportsLessThanGreaterThan(bSupportsLessThanGreaterThan)
    {}

    virtual ~Operand() {}
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString&) = 0;
    virtual Operand* Clone() = 0;
    virtual CString OperandName() = 0;

    //
    // Returns true if the Operand supports the operation: GreaterThan, LessThan, GreaterThanOrEqual, LessThanOrEqual
    //
    bool SupportsLessThanGreaterThan()
    {
        return m_bSupportsLessThanGreaterThan;
    }
};

class MsiProductVersion : public Operand
{
    ILogger& m_logger;
    CString m_productCode;
    CString m_productVersion;

    MsiProductVersion(const MsiProductVersion& rhs)
        : m_logger(rhs.m_logger)
        , m_productCode(rhs.m_productCode)
        , m_productVersion(rhs.m_productVersion)
        , Operand(true)   // Supports LessThan GreaterThan
    {}
public:
    MsiProductVersion(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        GetAuthoredProductInfo(spElement);
        
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }

    virtual ~MsiProductVersion() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"MsiProductVersion";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        CString version;

        DWORD dwCount = 0;
        UINT err = MsiGetProductInfo(m_productCode, INSTALLPROPERTY_VERSIONSTRING, NULL, &dwCount);
        if (err == ERROR_SUCCESS)
        {
            dwCount++;
            err = MsiGetProductInfo(m_productCode, INSTALLPROPERTY_VERSIONSTRING, version.GetBuffer(dwCount), &dwCount);
            version._ReleaseBuffer();
        }

        if (err != ERROR_SUCCESS)
        {
            LOG(m_logger, ILogger::Information, L"MsiGetProductInfo with product code " + m_productCode + L" found no matches");
            return false;
        }

        value = version;

        if (CString::StringLength(m_productVersion) == 0)
        {
            LOG(m_logger, ILogger::Information, L"ProductVersion for product with product code " + m_productCode + L" not authored.");
            return true;
        }

        LOG(m_logger, ILogger::Information, L"Product version of MSI package is" + version);
        LOG(m_logger, ILogger::Information, L"Product version of installed product is" + m_productVersion);

        int iComp = CompareVersionStrings(m_productVersion, version);

        if (iComp < 0)
        {
            LOG(m_logger, ILogger::Information, L"A higher version of the product with product code " + m_productCode + L" is already installed.");
            return true;
        }
        else if  (iComp == 0)
        {
            LOG(m_logger, ILogger::Information, L"Product with product code " + m_productCode + L" and version " + m_productVersion + L" is already installed.");
            return true;
        }
        else
        {
            LOG(m_logger, ILogger::Information, L"This is a minor upgrade of the product with product code " + m_productCode);
            return false;
        }
    }

    int CompareVersionStrings(const CString lhs, const CString rhs)
    {
        CString lhsOut;
        CString rhsOut;
        MSIUtils::NormalizeVersions(lhs, rhs, lhsOut, rhsOut, m_logger);
        return lhsOut.Compare(rhsOut);
    }

    void GetAuthoredProductInfo(const CComPtr<IXMLDOMElement>& spElementIn)
    {
        CComPtr<IXMLDOMElement> spElement = spElementIn;
        m_productCode = ElementUtils::GetAttributeByName(spElement, L"ProductCode", m_logger);

        if (L"Self" == m_productCode)
        {
            CComPtr<IXMLDOMElement> spElementLocal = spElement;

            //Loop until we either find MSI/AgileMSI element
            do
            {
                CComPtr<IXMLDOMNode> spNode;
                HRESULT hr = spElementLocal->get_parentNode(&spNode);
                if (FAILED(hr) || NULL == spNode)
                {
                    CInvalidXmlException ixe(L"schema validation failure: get_parentNode failed");
                    LOG(m_logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }

                CString csElementName = ElementUtils::GetName(spNode);
                if (L"MSI" == csElementName || L"AgileMSI" == csElementName)                   
                {
                    spElement = CComQIPtr<IXMLDOMElement>(spNode);
                    m_productCode = ElementUtils::GetAttributeByName(spElement, L"ProductCode", m_logger); 
                    break;
                }                
                
                spElementLocal = CComQIPtr<IXMLDOMElement>(spNode);

            }while(true);
        }

        if (spElement != NULL)
        {
            m_productVersion = ElementUtils::GetOptionalAttributeByName(spElement, L"ProductVersion", m_logger);
        }
    }

    virtual MsiProductVersion* Clone() { return new MsiProductVersion(*this); }
private: // "TestSubClass"
    virtual UINT MsiGetProductInfo(__in LPCWSTR szProduct, __in LPCWSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPWSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf) { return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf); }
};

class MsiGetCachedPatchPath : public Operand
{
    ILogger& m_logger;
    CString m_patchCode;

    MsiGetCachedPatchPath(const MsiGetCachedPatchPath& rhs)
        : m_logger(rhs.m_logger)
        , m_patchCode(rhs.m_patchCode)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    MsiGetCachedPatchPath(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_patchCode(GetPatchCode(spElement, logger))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~MsiGetCachedPatchPath() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"MsiGetCachedPatchPath";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();

        DWORD dwCount = MAX_PATH;
        UINT err = MsiGetPatchInfo(m_patchCode, INSTALLPROPERTY_LOCALPACKAGE, value.GetBuffer(MAX_PATH), &dwCount);
        value._ReleaseBuffer();
        if (err != ERROR_SUCCESS)
        {
            LOG(m_logger, ILogger::Information, L"MsiGetCachedPatchPath with patch code " + m_patchCode + L" failed");
            return false;
        }
        else
        {
            LOG(m_logger, ILogger::Information, L"MsiGetCachedPatchPath with patch code " + m_patchCode + L", returned: " + value);
            return true;
        }
    }

    static CString GetPatchCode(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CString csPatchCode = ElementUtils::GetAttributeByName(spElement, L"PatchCode", logger);
        if (L"Self" != csPatchCode)
            return csPatchCode;

        // Loop until we find MSP element, or we're out of (grand-)parents
        CComPtr<IXMLDOMElement> spElementLocal = spElement;
        for(;;)
        {
            CComPtr<IXMLDOMNode> spNode;
            HRESULT hr = spElementLocal->get_parentNode(&spNode);
            if (FAILED(hr) || NULL == spNode)
            {
                break;
            }
            spElementLocal = CComQIPtr<IXMLDOMElement>(spNode);
            if (spElementLocal == NULL) // this will fail on the #document node
            {
                break;
            }
            if (L"MSP" == ElementUtils::GetName(spNode))
            {
                return ElementUtils::GetAttributeByName(spElementLocal, L"PatchCode", logger); 
            }                
        }
        CInvalidXmlException ixe(L"schema validation failure searching for PatchCode: get_parentNode failed");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    virtual MsiGetCachedPatchPath* Clone() { return new MsiGetCachedPatchPath(*this); }
private: // "TestSubClass"
    virtual UINT MsiGetPatchInfo(__in LPCWSTR szPatch, __in LPCWSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPWSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    { return ::MsiGetPatchInfo(szPatch, szAttribute, lpValueBuf, pcchValueBuf); }
};

class FileVersion : public Operand
{
    ILogger& m_logger;
    CString m_location;
    FileVersion(const FileVersion& rhs)
        : m_logger(rhs.m_logger)
        , m_location(rhs.m_location)
        , Operand(true)   // Supports LessThan GreaterThan
    {}
public:
    FileVersion(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_location(ModuleUtils::FullPath(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true), logger))
        , m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }

    FileVersion(const CString& location, ILogger& logger)
    : m_location(location)
    , m_logger(logger)
    , Operand(true)   // Supports LessThan GreaterThan
    {}

    virtual ~FileVersion() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"FileVersion";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& csFileVer)
    {
        csFileVer.Empty();

        DWORD dwSize;
        DWORD dwHandle;
        if (0 != (dwSize = GetFileVersionInfoSize(m_location, &dwHandle)))
        {
            CString cs;
            LPVOID lpv = reinterpret_cast<LPVOID>(cs.GetBuffer((dwSize+1)/2));
            if (0 != GetFileVersionInfo(m_location, dwHandle, dwSize, lpv))
            {
                VOID* lpvBuffer = NULL;
                UINT uiLen;
                VerQueryValue(lpv, TEXT("\\"), &lpvBuffer, &uiLen);
                cs._ReleaseBuffer();

                if (lpvBuffer)
                {
                    const VS_FIXEDFILEINFO * pFFI = reinterpret_cast<const VS_FIXEDFILEINFO*>(lpvBuffer);
                    csFileVer.Format(L"%i.%i.%i.%i"
                        , HIWORD(pFFI->dwFileVersionMS)
                        , LOWORD(pFFI->dwFileVersionMS)
                        , HIWORD(pFFI->dwFileVersionLS)
                        , LOWORD(pFFI->dwFileVersionLS));
                    LOG(m_logger, ILogger::Information, L"FileVersion for " + m_location + L" is " + csFileVer);
                    return true;
                }
            }
        }
        LOG(m_logger, ILogger::Information, L"No FileVersion found for " + m_location);
        return false;
    }
    FileVersion* Clone() { return new FileVersion(*this); }
private: // "TestSubClass"
    virtual DWORD GetFileVersionInfoSize(__in LPCWSTR lptstrFilename, __out_opt LPDWORD lpdwHandle) { return ::GetFileVersionInfoSize(lptstrFilename, lpdwHandle); }
    virtual BOOL GetFileVersionInfo(__in LPCWSTR lptstrFilename, __reserved DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData)
    {   // goop added to fix prefast warning - 22116
        return ((NULL == lpData) || (0 > dwLen)) ? FALSE : ::GetFileVersionInfo(lptstrFilename, dwHandle, dwLen, lpData);
    }
    virtual BOOL VerQueryValue(__in LPCVOID pBlock, __in LPCWSTR lpSubBlock, __deref_out_xcount("buffer can be PWSTR or DWORD*") LPVOID * lplpBuffer, __out PUINT puLen) { return ::VerQueryValue(pBlock, lpSubBlock, lplpBuffer, puLen); }
};

class RebootPending : public Operand
{
    ILogger& m_logger;

    RebootPending(const RebootPending& rhs)
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    RebootPending(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~RebootPending() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"RebootPending";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {
        cs.Empty();

        BOOL bRebootPending = IsRebootRequired();

        if (bRebootPending)
        {
            LOG(m_logger, ILogger::Information, L"RebootPending:  A reboot is pending");
        }
        else
        {
            LOG(m_logger, ILogger::Information, L"RebootPending:  No reboot pending");
        }
        return bRebootPending;
    }
    RebootPending* Clone() { return new RebootPending(*this); }

private: // test sub-class
    virtual bool IsRebootRequired()
    {
        CoInitializer ci; // turn COM on
        CComPtr<ISystemInformation> spSystemInformation;
        HRESULT hr = CoCreateInstance(__uuidof(SystemInformation)
                                    , NULL, CLSCTX_INPROC_SERVER
                                    , __uuidof(ISystemInformation)
                                    , reinterpret_cast<LPVOID*>(&spSystemInformation));
        if ( FAILED(hr) )
        {
            CString msg;
            msg.Format(L"CoCreateInstance failed for ISystemInformation with an error of 0x%x, RebootPending will default to FALSE", hr);
            LOG(m_logger, ILogger::Information, msg);
            IMASSERT2(0, L"CoCreateInstance failed for ISystemInformation");
            // if CoCreateInstance fails, do not return true for RebootPending
        }
        else
        {
            VARIANT_BOOL bRetVal;
            hr = spSystemInformation->get_RebootRequired(&bRetVal);
            if ( FAILED(hr) )
            {
                CString msg;
                msg.Format(L"ISystemInformation::get_RebootRequired failed with an error of 0x%x, RebootPending will default to FALSE", hr);
                LOG(m_logger, ILogger::Information, msg);
                IMASSERT2(0, L"ISystemInformation::get_RebootRequired failed");
                // if get_RebootRequired fails, do not return true for RebootPending
            }
            else
            {
                if (VARIANT_FALSE == bRetVal)
                    return false;
                else
                    return true;
            }
        }
        return false;
    }

};

class IsAdministrator : public Operand
{
    ILogger& m_logger;

    IsAdministrator(const IsAdministrator& rhs)
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    IsAdministrator(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~IsAdministrator() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"IsAdministrator";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {
        cs.Empty();

        BOOL bIsMember = FALSE;

        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID pAdministratorsGroup = NULL;
        AllocateAndInitializeSid ( &NtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pAdministratorsGroup); 
        if (pAdministratorsGroup)
        {
            if (!CheckTokenMembership(NULL, pAdministratorsGroup, &bIsMember)) 
                 bIsMember = FALSE;
            FreeSid(pAdministratorsGroup); 
        }

        if (bIsMember == FALSE)
        {
            LOG(m_logger, ILogger::Information, L"IsAdministrator:  NOT a member of the Administrators group");
            return false;
        }
        LOG(m_logger, ILogger::Information, L"IsAdministrator:  IS a member of the Administrators group");
        return true;
    }
    IsAdministrator* Clone() { return new IsAdministrator(*this); }

private: // test sub-class
    virtual BOOL AllocateAndInitializeSid(__in PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority, __in BYTE nSubAuthorityCount, __in DWORD nSubAuthority0, __in DWORD nSubAuthority1, __in DWORD nSubAuthority2, __in DWORD nSubAuthority3, __in DWORD nSubAuthority4, __in DWORD nSubAuthority5, __in DWORD nSubAuthority6, __in DWORD nSubAuthority7, __deref_out PSID *pSid)
    { return ::AllocateAndInitializeSid (pIdentifierAuthority, nSubAuthorityCount, nSubAuthority0, nSubAuthority1, nSubAuthority2, nSubAuthority3, nSubAuthority4, nSubAuthority5, nSubAuthority6, nSubAuthority7, pSid); }
    virtual BOOL CheckTokenMembership(__in_opt HANDLE TokenHandle, __in PSID SidToCheck, __out PBOOL IsMember)
    { return ::CheckTokenMembership(TokenHandle, SidToCheck, IsMember); }
    virtual PVOID FreeSid(__in PSID pSid)
    { return ::FreeSid(pSid); }
};

//------------------------------------------------------------
//Determine if the command line argument is provided
//------------------------------------------------------------
class CommandLineSwitch : public Operand
{
    ILogger& m_logger;
    CString m_csCommandLineArgument;

    //Copy constructor
    CommandLineSwitch(const CommandLineSwitch& rhs)
        : m_csCommandLineArgument(rhs.m_csCommandLineArgument)
        , m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    //Constructor
    CommandLineSwitch(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_csCommandLineArgument(ElementUtils::GetAttributeByName(spElement, L"Name", logger, true))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }

    //Descrtuctor
    virtual ~CommandLineSwitch() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"CommandLineSwitch";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {       
        CCmdLineSwitches switches;
        return switches.SwitchIsPresent(m_csCommandLineArgument);
    }

    CommandLineSwitch* Clone() 
    { 
        return new CommandLineSwitch(*this); 
    }
};

class Lcid : public Operand
{
    ILogger& m_logger;      

    //Copy constructor
    Lcid(const Lcid& rhs)        
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    //Constructor
    Lcid(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
        {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }

    //Descrtuctor
    virtual ~Lcid() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"LCID";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {       
        cs = "1033";
        LOG(m_logger, ILogger::Information, L"Current Lcid value is " + cs);
        return true;
    }

    Lcid* Clone() 
    { 
        return new Lcid(*this); 
    }
};

class ChainerMode : public Operand
{
    ILogger& m_logger;      

    //Copy constructor
    ChainerMode(const ChainerMode& rhs)        
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    //Constructor
    ChainerMode(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }

    //Descrtuctor
    virtual ~ChainerMode() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"Operation";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {       
        cs = ipdtoDataToOperand.GetChainerMode();        
        LOG(m_logger, ILogger::Information, L"Current Operation value is " + cs);
        return true;
    }

    ChainerMode* Clone() 
    { 
        return new ChainerMode(*this); 
    }
};

struct RegKeyStringUtils
{
    static HKEY GetHiveFromString(const CString& cs, const CString& elementName, ILogger& logger)
    {
        CString exception = L"schema validation error:  " + elementName + L"'s Location attribute doesn't match any (supported) hive";

        CString csHive(cs);
        int firstWhack = cs.Find(L'\\');
        if (firstWhack != -1)
            csHive = cs.Left(firstWhack);

        // we now support hives of the format HKLM32 or HKLM64, as well.
        if (csHive.Find(L"32") == csHive.GetLength()-2)
            csHive = csHive.Left (csHive.GetLength()-2);
        if (csHive.Find(L"64") == csHive.GetLength()-2)
            csHive = csHive.Left (csHive.GetLength()-2);

        if ((0 == csHive.CompareNoCase(L"HKLM")) || (0 == csHive.CompareNoCase(L"HKEY_LOCAL_MACHINE")))
            return HKEY_LOCAL_MACHINE;
        else if ((0 == csHive.CompareNoCase(L"HKCR")) || (0 == csHive.CompareNoCase(L"HKEY_CLASSES_ROOT")))
            return HKEY_CLASSES_ROOT;
        else if ((0 == csHive.CompareNoCase(L"HKCU")) || (0 == csHive.CompareNoCase(L"HKEY_CURRENT_USER")))
            return HKEY_CURRENT_USER;
        else if ((0 == csHive.CompareNoCase(L"HKU")) || (0 == csHive.CompareNoCase(L"HKEY_USERS")))
            return HKEY_USERS;

        CInvalidXmlException ixe(exception);
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    static CString GetPathFromString(const CString& cs, ILogger& logger)
    {
        int firstWhack = cs.Find(L'\\');
        if (firstWhack != -1)
            return cs.Mid(firstWhack+1);
        return L"";
    }

    static bool ExtractValueAndPath(CString& path, CString& value)
    {
        // path contains value, so extract it
        int lastWhack = path.ReverseFind(L'\\');
        if (lastWhack != -1)
        {
            value = path.Mid(lastWhack+1);
            //Special case for handling (default) value.
            if (L"(Default)" == value)
                value = L"";
            path  = path.Left(lastWhack);
            return true;
        }
        return false;
    }
};

class HiveSelectionUtils
{
    static CString GetHive(const CString& cs)
    {
        int firstWhack = cs.Find(L'\\');
        if (firstWhack != -1)
            return cs.Left(firstWhack);
        return L"";
    }
    static bool IsExplicitly32BitHive(const CString& cs)
    {
        CString csHive = GetHive(cs);
        return csHive.Find(L"32") == csHive.GetLength()-2;
    }
    static bool IsExplicitly64BitHive(const CString& cs)
    {
        CString csHive = GetHive(cs);
        return csHive.Find(L"64") == csHive.GetLength()-2;
    }

public:
    template<typename CRegKey> static LONG Open(CRegKey& rk, const CString& location)
    {
        HKEY hkey = RegKeyStringUtils::GetHiveFromString(location, L"RegKey or RegKeyValue", NullLogger::GetNullLogger());
        CString subKey = RegKeyStringUtils::GetPathFromString(location, NullLogger::GetNullLogger());

        // three cases:  explicit 32 bit hive, explicit 64 bit hive, or neither (which means check 32 first, then 64)
        if (HiveSelectionUtils::IsExplicitly64BitHive(location))
            return rk.Open(hkey, subKey, KEY_QUERY_VALUE | KEY_WOW64_64KEY);
        if (HiveSelectionUtils::IsExplicitly32BitHive(location))
            return rk.Open(hkey, subKey, KEY_QUERY_VALUE);

        // else check 32 bit first, then 64 bit.
        LONG lres = rk.Open(hkey, subKey, KEY_QUERY_VALUE);
        if (lres != ERROR_SUCCESS)
            lres = rk.Open(hkey, subKey, KEY_QUERY_VALUE | KEY_WOW64_64KEY);
        return lres;
    }
};

template<typename CRegKey>
class RegKeyValueT : public Operand
{
    ILogger& m_logger;
    CString m_location;
    RegKeyValueT(const RegKeyValueT& rhs)
        : m_logger(rhs.m_logger)
        , m_location(rhs.m_location)
        , Operand(true)   // Supports LessThan GreaterThan
    {}
public:
    RegKeyValueT(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_location(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true))
        , m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
        RegKeyStringUtils::GetHiveFromString(m_location, OperandName(), logger);
    }

    RegKeyValueT(const CString& location, ILogger& logger)
        : m_location(location)
        , m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        RegKeyStringUtils::GetHiveFromString(m_location, OperandName(), logger);
    }

    virtual ~RegKeyValueT() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"RegKeyValue";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();

        CString valueName;
        CString path(m_location);
        if (true == RegKeyStringUtils::ExtractValueAndPath(path, valueName))
        {
            CRegKey rk;
            LONG lres = HiveSelectionUtils::Open<CRegKey>(rk, path);
            if (lres == ERROR_SUCCESS)
            {
                DWORD dwType = 0;
                ULONG ulBytes = 0;
                lres = rk.QueryValue(valueName, &dwType, NULL, &ulBytes);

                if (lres == ERROR_SUCCESS)
                {
                    switch(dwType)
                    {
                    default:
                    case REG_NONE:              // No defined value type. 
                    case REG_QWORD:             // A 64-bit number. 
                    case REG_DWORD_BIG_ENDIAN:  // A 32-bit number in big-endian format. 
                    case REG_BINARY:            // Binary data in any form. 
                    {
                        CStringA buf;
                        lres = rk.QueryValue(valueName, &dwType, buf.GetBuffer(ulBytes), &ulBytes);
                        buf._ReleaseBuffer(ulBytes);
                        value = ConvertToNibbles(buf, ulBytes);
                    }
                        break;
                    case REG_DWORD: // A 32-bit number. 
                        {
                            ulBytes = 4;
                            DWORD number = 0;
                            lres = rk.QueryValue(valueName, &dwType, &number, &ulBytes);
                            value.Format(L"%i", number);
                        }
                        break;
                    case REG_SZ:
                    case REG_EXPAND_SZ:
                    case REG_MULTI_SZ:  // A sequence of null-terminated strings, terminated by an empty string (\0). 
                        lres = rk.QueryValue(valueName, &dwType, value.GetBuffer(ulBytes), &ulBytes); // overkill on the buffer size, but that's ok
                        value._ReleaseBuffer();
                        break;
                    }

                    LOG(m_logger, ILogger::Information, L"RegKeyValue: " + m_location + L" contains '" + value + L"'");
                    return true;
                }
            }
        }

        LOG(m_logger, ILogger::Information, L"RegKeyValue: " + m_location + L" does NOT exist.");
        return false;
    }
    RegKeyValueT* Clone() { return new RegKeyValueT(*this); }

private:
    static CString ConvertToNibbles(const CStringA& input, int chars)
    {
        CString cs;
        for(int i=0; i<chars; ++i)
        {
            char c = input[i];
            cs += ConvertToNibble((c>> 4)&0xF);
            cs += ConvertToNibble((c>> 0)&0xF);
        }
        return cs;
    }
    static CString ConvertToNibble(WCHAR c)
    {
        switch(c)
        {
        case 0x0:   return L"0";
        case 0x1:   return L"1";
        case 0x2:   return L"2";
        case 0x3:   return L"3";
        case 0x4:   return L"4";
        case 0x5:   return L"5";
        case 0x6:   return L"6";
        case 0x7:   return L"7";
        case 0x8:   return L"8";
        case 0x9:   return L"9";
        case 0xa:   return L"A";
        case 0xb:   return L"B";
        case 0xc:   return L"C";
        case 0xd:   return L"D";
        case 0xe:   return L"E";
        case 0xf:   return L"F";
        default:
            IMASSERT(0 && L"trying to convert a number > 0xF to a nibble!!!");
            return L"";
        }
    }
};

typedef RegKeyValueT<ATL::CRegKey> RegKeyValue;

class RegKeyFileVersion : public Operand
{
    ILogger& m_logger;
    CString m_location;

    RegKeyFileVersion(const RegKeyFileVersion& rhs)
        : m_logger(rhs.m_logger)
        , m_location(rhs.m_location)
        , Operand(true)   // Supports LessThan GreaterThan
    {}
public:
    RegKeyFileVersion(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_location(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true))
        , m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
        RegKeyStringUtils::GetHiveFromString(m_location, OperandName(), logger);
    }

    virtual ~RegKeyFileVersion() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"RegKeyFileVersion";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();

        RegKeyValue filePathRegKey(m_location, m_logger);
        CString filePath;
        Operand* opr = &filePathRegKey;
        if (opr ->Evaluate(ipdtoDataToOperand, filePath))
        {
            // Expand Env variables.
            CString expandedFilePath;
            CSystemUtil::ExpandEnvironmentVariables(filePath, expandedFilePath);
            FileVersion fileVersion(expandedFilePath, m_logger);
            opr = &fileVersion;
            if (opr->Evaluate(ipdtoDataToOperand, value))
            {
                return true;
            }
            else
            {
                LOG(m_logger, ILogger::Information, L"RegKeyFileVersion: failed to get FileVersion");
            }
        }
        else
        {
            LOG(m_logger, ILogger::Information, L"c: failed to get RegKey value");
        }
        
        return false;
    }
        RegKeyFileVersion* Clone() { return new RegKeyFileVersion(*this); }

};

class TargetArchitecture : public Operand
{
    ILogger& m_logger;
    TargetArchitecture(const TargetArchitecture& rhs)
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    TargetArchitecture(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~TargetArchitecture() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"TargetArchitecture";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        SYSTEM_INFO si = {0};
        MyGetSystemInfo(&si);

        // NOTE: these string values match what Wix and Decatur use.  Don't change them!

        switch(si.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
            value = L"x64";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            value = L"ia64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            value = L"x86";
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            value.Empty();
            LOG(m_logger, ILogger::Information, L"TargetArchitecture is unknown");
            return false;
        }
        LOG(m_logger, ILogger::Information, L"TargetArchitecture is " + value);
        return true;
    }
    TargetArchitecture* Clone() { return new TargetArchitecture(*this); }

private:
    void MyGetSystemInfo(SYSTEM_INFO* si)
    {
        // on WOW64, we must use GetNativeSystemInfo
        void (WINAPI * pfnGetNativeSystemInfo)(LPSYSTEM_INFO) = reinterpret_cast<void (WINAPI *)(LPSYSTEM_INFO)>(
            GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo"));
        if (pfnGetNativeSystemInfo)
            pfnGetNativeSystemInfo(si);
        else
            GetSystemInfo(si);
    }

private: // test-hooks
    virtual VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) { ::GetSystemInfo(lpSystemInfo); }
    virtual FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) { return ::GetProcAddress(hModule, lpProcName); }
};

//This operand determine if we are in OS Compatability Mode.
class IsInOSCompatibilityMode : public Operand
{
    ILogger& m_logger;
    IsInOSCompatibilityMode(const IsInOSCompatibilityMode& rhs)
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    IsInOSCompatibilityMode(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~IsInOSCompatibilityMode() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"IsInOSCompatibilityMode";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& strValue)
    {
        strValue.Empty();
        bool bIsInCompatibilityMode = IsOSCompatibilityMode();

        //Either the environment variable is not specified or the length is 0, we are not in Compatability mode.
        if (!bIsInCompatibilityMode)
        {
            LOG(m_logger, ILogger::Information, L"Not In OS Compatability Mode");
        }
        else
        {
            LOG(m_logger, ILogger::Information, L"In OS Compatability Mode");
        }
        return bIsInCompatibilityMode;
    }
    IsInOSCompatibilityMode* Clone() { return new IsInOSCompatibilityMode(*this); }

private: // test-hooks
    virtual bool IsOSCompatibilityMode()
    {
        return CSystemUtil::IsInOSCompatibilityMode();
    }
};

class TargetOS : public Operand
{
    ILogger& m_logger;
    TargetOS(const TargetOS& rhs)
        : m_logger(rhs.m_logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {}
public:
    TargetOS(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(true)   // Supports LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~TargetOS() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"TargetOS";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();

        OSVERSIONINFOEX osie = {0};
        osie.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (FALSE == GetVersionEx(&osie))
        {
            LOG(m_logger, ILogger::Information, L"TargetOS is unknown");
            return false;
        }

        value.Format(L"%i.%i.%i", osie.dwMajorVersion, osie.dwMinorVersion, osie.wServicePackMajor);

        LOG(m_logger, ILogger::Information, L"TargetOS is " + value);
        return true;
    }
    TargetOS* Clone() { return new TargetOS(*this); }

private: // test-hooks
    virtual BOOL GetVersionEx(OSVERSIONINFOEX* osie) { return ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(osie)); }
};

class TargetOSType : public Operand
{
    ILogger& m_logger;
    TargetOSType(const TargetOSType& rhs)
        : m_logger(rhs.m_logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    TargetOSType(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~TargetOSType() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"TargetOSType";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();

        OSVERSIONINFOEX osie = {0};
        osie.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (FALSE == GetVersionEx(&osie))
        {
            LOG(m_logger, ILogger::Information, L"TargetOSType is unknown");
            return false;
        }

        if (osie.wProductType == VER_NT_WORKSTATION)
            value = L"Client";
        else
            value = L"Server";

        LOG(m_logger, ILogger::Information, L"TargetOSType is " + value);
        return true;
    }
    TargetOSType* Clone() { return new TargetOSType(*this); }

private: // test-hooks
    virtual BOOL GetVersionEx(OSVERSIONINFOEX* osie) { return ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(osie)); }
};


// File/Path, RegKey, etc.  Idea:  return non-empty v. empty
class Path : public Operand
{
    ILogger& m_logger;
    CString m_location;
    Path(const Path& rhs)
        : m_logger(rhs.m_logger)
        , m_location(rhs.m_location)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    Path(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_location(ModuleUtils::FullPath(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true), logger))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~Path() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"Path";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {
        cs.Empty();

        if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(m_location))
        {
            LOG(m_logger, ILogger::Information, L"Path: " + m_location + L" does NOT exist.");
            return false;
        }
        LOG(m_logger, ILogger::Information, L"Path: " + m_location + L" exists.");
        return true;
    }
    Path* Clone() { return new Path(*this); }
};

template<typename CRegKey>
class RegKeyT : public Operand
{
    ILogger& m_logger;
    CString m_location;
    RegKeyT(const RegKeyT& rhs)
        : m_logger(rhs.m_logger)
        , m_location(rhs.m_location)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    RegKeyT(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_location(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {       
        ElementUtils::VerifyName(spElement, OperandName(), logger);
        RegKeyStringUtils::GetHiveFromString(m_location, OperandName(), logger);
    }
    virtual ~RegKeyT() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"RegKey";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {
        cs.Empty();

        // first, try the location as though it were a key only
        CRegKey rk;
        LONG lres = HiveSelectionUtils::Open<CRegKey>(rk, m_location);

        if (lres != ERROR_SUCCESS)
        {   // maybe it was a key + value.  Let's try that.
            CString valueName;
            CString path = m_location;
            if (true == RegKeyStringUtils::ExtractValueAndPath(path, valueName))
            {
                lres = HiveSelectionUtils::Open<CRegKey>(rk, path);
                if (lres == ERROR_SUCCESS)
                {
                    DWORD dwType = 0;
                    ULONG ulBytes = 0;
                    lres = rk.QueryValue(valueName, &dwType, NULL, &ulBytes);
                }
            }
        }

        if (lres != ERROR_SUCCESS)
        {
            LOG(m_logger, ILogger::Information, L"RegKey: " + m_location + L" does NOT exist.");
            return false;
        }
        LOG(m_logger, ILogger::Information, L"RegKey: " + m_location + L" exists.");
        return true;
    }
    RegKeyT* Clone() { return new RegKeyT(*this); }
};
typedef RegKeyT<ATL::CRegKey> RegKey;

class MsiXmlBlob : public Operand, private MsiXmlBlobBase
{
    ILogger& m_logger;
    CComBSTR m_bstrBlob;

    MsiXmlBlob(const MsiXmlBlob& rhs)
        : m_logger(rhs.m_logger)
        , m_bstrBlob(rhs.m_bstrBlob)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    MsiXmlBlob(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_bstrBlob(GetBlob(spElement, logger))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~MsiXmlBlob() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"MsiXmlBlob";
    }

private: // Operand
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& cs)
    {
        cs.Empty();

        bool applicable = EnumerateApplicableProducts(m_bstrBlob);
        if (applicable)
            LOG(m_logger, ILogger::Information, L"MsiXmlBlob: this patch is applicable");
        else
            LOG(m_logger, ILogger::Information, L"MsiXmlBlob: this patch is not applicable");
        return applicable;

    }
    MsiXmlBlob* Clone() { return new MsiXmlBlob(*this); }

private:
    virtual bool OnApplicableProduct(const CComBSTR& productCode)
    {
        return false; // found one; don't keep enumerating
    }

private:
    static CComBSTR GetBlob(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CComBSTR bstrXML;
        spElement->get_xml(&bstrXML);
        return bstrXML;
    }
private: // test hook (for speed purposes, mostly)
    virtual UINT MsiDeterminePatchSequence(__in LPCWSTR szProductCode, __in_opt LPCWSTR szUserSid, __in MSIINSTALLCONTEXT dwContext, __in DWORD cPatchInfo, __inout_ecount(cPatchInfo) PMSIPATCHSEQUENCEINFO pPatchInfo)
    {
        return ::MsiDeterminePatchSequence(szProductCode, szUserSid, dwContext, cPatchInfo, pPatchInfo);
    }
};

class HasAdvertisedFeatures : public Operand
{
    ILogger& m_logger;
    CString m_productCode;

    HasAdvertisedFeatures(const HasAdvertisedFeatures& rhs)
        : m_logger(rhs.m_logger)
        , m_productCode(rhs.m_productCode)
        , Operand(false)   // does not support LessThan GreaterThan
    {}
public:
    HasAdvertisedFeatures(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_productCode(ElementUtils::GetAttributeByName(spElement, L"ProductCode", logger, true))
        , m_logger(logger)
        , Operand(false)   // does not support LessThan GreaterThan
    {
        ElementUtils::VerifyName(spElement, OperandName(), logger);
    }
    virtual ~HasAdvertisedFeatures() {}

    //
    // OperandName returns the name of the Operand, used in VerifyName and for error messages
    //
    virtual CString OperandName()
    {
        return L"HasAdvertisedFeatures";
    }

private: // Operand
    // Returns true if this product has any features that are advertised.
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand, CString& value)
    {
        value.Empty();
        CString features;
        LOG(m_logger, ILogger::Verbose, L"Starting Feature Enumeration for product: " + m_productCode);
        for(DWORD j=0; true; ++j)
        {
            WCHAR featureName[MAX_FEATURE_CHARS+1] = {0};
            // MsiEnumFeatures returns ERROR_UNKNOWN_PRODUCT when the product is not installed.
            if (ERROR_SUCCESS != MsiEnumFeatures(m_productCode, j, featureName, NULL))
                break;
            if(INSTALLSTATE_ADVERTISED == MsiQueryFeatureState(m_productCode, featureName)) 
            {
                if (features.IsEmpty())
                    features = featureName;
                else
                    features += CString(L",") + featureName;
            }
        }
        LOG(m_logger, ILogger::Verbose, L"Completed Feature Enumeration for product: " + m_productCode);
        if (features.GetLength())
        {
            LOG(m_logger, ILogger::Warning, L"Advertised Features found: " + features);
            return true;
        }
        return false;
    }

    virtual HasAdvertisedFeatures* Clone() { return new HasAdvertisedFeatures(*this); }
private: // "TestSubClass"
    virtual UINT MsiEnumFeatures(__in LPCWSTR szProduct, __in DWORD iFeatureIndex, __out_ecount(MAX_FEATURE_CHARS+1) LPWSTR lpFeatureBuf, __out_ecount_opt(MAX_FEATURE_CHARS+1) LPWSTR lpParentBuf)
    { return ::MsiEnumFeatures(szProduct, iFeatureIndex, lpFeatureBuf, lpParentBuf); }
    virtual INSTALLSTATE MsiQueryFeatureState(__in LPCWSTR  szProduct, __in LPCWSTR  szFeature)
    { return ::MsiQueryFeatureState(szProduct, szFeature); }
};

struct OperandUtils
{
    static Operand* GetOperandFromChild(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            CString name = ElementUtils::GetName(spChild);
            if (L"MsiProductVersion" == name)       return new MsiProductVersion(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"MsiGetCachedPatchPath" == name)   return new MsiGetCachedPatchPath(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"FileVersion" == name)             return new FileVersion(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"IsAdministrator" == name)         return new IsAdministrator(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"Path" == name)                    return new Path(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"RegKey" == name)                  return new RegKey(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"RegKeyValue" == name)             return new RegKeyValue(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"MsiXmlBlob" == name)              return new MsiXmlBlob(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"TargetArchitecture" == name)      return new TargetArchitecture(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"TargetOS" == name)                return new TargetOS(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"TargetOSType" == name)            return new TargetOSType(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"HasAdvertisedFeatures" == name)   return new HasAdvertisedFeatures(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"CommandLineSwitch" == name)       return new CommandLineSwitch(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"LCID" == name)                    return new Lcid(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"Operation" == name)               return new ChainerMode(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"RebootPending" == name)           return new RebootPending(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"RegKeyFileVersion" == name)       return new RegKeyFileVersion(CComQIPtr<IXMLDOMElement>(spChild), logger);
            if (L"IsInOSCompatibilityMode" == name) return new IsInOSCompatibilityMode(CComQIPtr<IXMLDOMElement>(spChild), logger);

            CInvalidXmlException ixe(L"schema validation error:  unknown operand element: " + name);
            throw ixe;
        }
        CInvalidXmlException ixe(L"schema validation error:  missing child element of " + ElementUtils::GetName(spElement));
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }
};

}
