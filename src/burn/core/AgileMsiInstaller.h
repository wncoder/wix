//-------------------------------------------------------------------------------------------------
// <copyright file="AgileMsiInstaller.h" company="Microsoft">
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

#include "MsiInstaller.h"

namespace IronMan
{

class AgileMsiInstaller : public MsiInstallerBaseT<AgileMSI>
{
public:
    AgileMsiInstaller(const AgileMSI& msi,  ILogger& logger, Ux& uxLogger)
        : MsiInstallerBaseT<AgileMSI>(msi, IProgressObserver::Installing, logger, uxLogger)
    {}

private:
    virtual UINT Execute()
    {
        CCmdLineSwitches switches; 

        //The order in which MSIOptions are applied is important here because MSI will get the last set property value.
        CString csMsiOptions = CString(GetMsi().GetMsiOptions() + L" " + switches.GetMsiOptions()).Trim();		

        const CSimpleArray<CString>& mspPackagePaths = GetMsi().GetMspPackages();
        
        _ASSERT(mspPackagePaths.GetSize() > 0);

        CString csMspCommandLine(L" PATCH=\"");
        csMspCommandLine += mspPackagePaths[0];

        for(int i = 1; i < mspPackagePaths.GetSize() ; i++)
        {
            csMspCommandLine += L";" + mspPackagePaths[i];
        }

        csMspCommandLine += L"\" ";
        csMsiOptions += csMspCommandLine;

        CString log(L"Calling MsiInstallProduct("); 	
        log += GetMsi().GetName() + L", " + csMsiOptions; 
        LOG(Logger(), ILogger::Verbose, log);

        return MsiInstallProduct(GetMsi().GetName(), csMsiOptions);        
    }

    virtual CString Operation(void)
    {
        return CString(L"Installation");
    }

private: // test sub-class test hook
    virtual UINT WINAPI MsiInstallProduct(__in LPCWSTR szPackagePath, __in_opt LPCWSTR szCommandLine)
    {
        IMASSERT(szPackagePath);
        IMASSERT(szCommandLine);
        return ::MsiInstallProduct(szPackagePath, szCommandLine);
    }
};

typedef MsiRepairerT<AgileMSI, CCmdLineSwitches> AgileMsiRepairer;
typedef MsiUnInstallerT<AgileMSI> AgileMsiUnInstaller;

//
// CreateAgileMsiPerformer
//
static HRESULT CreateAgileMsiPerformer(
                           __in const ActionTable::Actions action,
                           __in const AgileMSI& agileMsi,
                           __in 
                           __in ILogger& logger,
                           __in Ux& uxLogger,
                           __out IPerformer** ppPerformer
                           )
{
    HRESULT hr = S_OK;
    LOG(logger, ILogger::Verbose, L"Creating new Performer for AgileMSI item");

    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new AgileMsiInstaller(agileMsi, logger, uxLogger);
        break;

    case ActionTable::Uninstall:
        *ppPerformer = new AgileMsiUnInstaller(agileMsi, logger, uxLogger);
        break;

    case ActionTable::Repair:
        *ppPerformer = new AgileMsiRepairer(agileMsi, logger, uxLogger);
        break;

    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();
        break;
 
    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type. Can't create performer.");
        hr = E_INVALIDARG;
    }

    return hr;
}

}
