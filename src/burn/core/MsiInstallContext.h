//-------------------------------------------------------------------------------------------------
// <copyright file="MsiInstallContext.h" company="Microsoft">
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
//------------------------------------------------------------------------------
// CMsiInstallContext class
//
// Used to determine if the InstallContext of the installed product
//-------------------------------------------------------------------------------
class CMsiInstallContext
{
    const CString& m_productCode;
public:
    //------------------------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------------------------
    CMsiInstallContext(const CString& productCode)
        : m_productCode(productCode)
    {
    }

    //------------------------------------------------------------------------------
    // Destructor
    //-------------------------------------------------------------------------------
    virtual ~CMsiInstallContext()
    {
    }

    //------------------------------------------------------------------------------
    // GetContext
    // Used to determine the Install Context of a product
    // return error if product not installed or Install Context cannot be determined
    //-------------------------------------------------------------------------------
    UINT GetContext(MSIINSTALLCONTEXT &context)
    {
        UINT result = ERROR_SUCCESS;

        try
        {
            if (ProductIsInstalledPerMachine())
                context = MSIINSTALLCONTEXT_MACHINE;
            else if (ProductIsManaged())
                context = MSIINSTALLCONTEXT_USERMANAGED;
            else
                context = MSIINSTALLCONTEXT_USERUNMANAGED;
        }
        catch (UINT errorCode)
        {
            result = errorCode;
        }

        return result;
    }

private:
    bool ProductIsInstalledPerMachine(void)
    {
        WCHAR wszAssignmentType[10] = {0};
        DWORD cchAssignmentType = _countof(wszAssignmentType);
        UINT uiReturn = this->MsiGetProductInfo(m_productCode,INSTALLPROPERTY_ASSIGNMENTTYPE,wszAssignmentType,&cchAssignmentType);
        if (uiReturn != ERROR_SUCCESS)
            throw uiReturn;

        return (wszAssignmentType[0] == L'1');
    }

    //------------------------------------------------------------------------------
    // Only applications that require elevated privileges for installation and being
    // installed through advertisement are considered managed, which means that an
    // application installed per-machine is always considered managed. 
    // An application that is installed per-user is only considered managed if it is
    // advertised by a local system process that is impersonating the user. 
    //-------------------------------------------------------------------------------
    bool ProductIsManaged(void)
    {
        BOOL fManaged = FALSE;
        UINT uiReturn = this->MsiIsProductElevated(m_productCode, &fManaged);
        if (uiReturn != ERROR_SUCCESS)
            throw uiReturn;

        return fManaged;
    }

private:
    //------------------------------------------------------------------------------
    // Stubs for unit tests
    //-------------------------------------------------------------------------------
    virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    {
        return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf);
    }

    virtual UINT MsiIsProductElevated(__in LPCWSTR szProduct, __out BOOL *pfElevated)  
    {
        return ::MsiIsProductElevated(szProduct, pfElevated);
    }
};
}
