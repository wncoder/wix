//-------------------------------------------------------------------------------------------------
// <copyright file="ExeSelectingPerformer.h" company="Microsoft">
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
//    The class for creating the various exe type.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IPerformer.h"
#include "schema\EngineData.h"
#include "ExeInstaller.h"
#include "CartmanExeInstaller.h"
#include "IronManExeInstaller.h"
#include "MsuInstaller.h"

//------------------------------------------------------------------------------
// Class: ExeSelectingPerformer
//
// This class instantates the type specific EXE installers and EXE uninstallers
// to perform the required operations.
//------------------------------------------------------------------------------

namespace IronMan
{
    template<typename ExePerformer, typename CartmanExePerformer, typename IronManExePerformer, typename MsuExePerformer>
    class ExeSelectingPerformer : public AbortPerformerBase
    {
        IPerformer* m_performer;

    public:
        ExeSelectingPerformer(const ExeBase& exe,  bool bSerialDownload, ILogger& logger, Ux& uxLogger)
        {
            if (exe.GetExeType().IsCartmanExe())
            {
                m_performer = new CartmanExePerformer(exe, logger, uxLogger);
            }
            else if (exe.GetExeType().IsIronManExe())
            {
                m_performer = new IronManExePerformer(exe, bSerialDownload, logger, uxLogger);
            }
            else if (exe.GetExeType().IsMsuPackageExe())
            {
                m_performer = new MsuExePerformer(exe, logger, uxLogger);
            }
            else if (exe.GetExeType().IsLocalExe() || exe.GetExeType().IsUnknownExe())
            {
                m_performer = new ExePerformer(exe, logger, uxLogger);
            }
            else 
            {
                IMASSERT2(0, L"Exe type is not yet implemented");
                throw E_NOTIMPL;
            }
        }

        virtual ~ExeSelectingPerformer()
        {
            delete m_performer; 
        }

        virtual void PerformAction(IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void Abort()
        {
            m_performer->Abort();
        }
    };

    typedef ExeSelectingPerformer<ExeInstaller, CartmanExeInstaller, IronManExeInstaller, MsuInstaller> ExeSelectingInstaller;
    typedef ExeSelectingPerformer<ExeUnInstaller, CartmanExeUnInstaller, IronManExeUnInstaller, MsuUninstaller> ExeSelectingUnInstaller;
    typedef ExeSelectingPerformer<ExeRepairer, CartmanExeRepairer, IronManExeRepairer, MsuRepairer> ExeSelectingRepairer;

//
// CreateExePerformer
//
    static HRESULT CreateExePerformer(
        __in const ActionTable::Actions action,
        __in const Exe& exePackage, 
        __in  
        __in bool bSerialDownload, 
        __in ILogger& logger, 
        __in Ux& uxLogger,
        __out IPerformer** ppPerformer
        )
    {
        HRESULT hr = S_OK;
        LOG(logger, ILogger::Verbose, L"Creating new Performer for Exe item");
        const ExeBase* pRepairExe = NULL;
        const ExeBase* pUninstallExe = NULL;

        switch(action)
        {
        case ActionTable::Install:
            *ppPerformer = new ExeSelectingInstaller(exePackage, bSerialDownload, logger, uxLogger);
            break;

        case ActionTable::Uninstall:
            pUninstallExe = exePackage.GetUninstallOverride();

            // If there is an UninstallOverride Exe available, that needs to be used to uninstall 
            // this 'exePackage' item, use it.

            if (pUninstallExe)
                *ppPerformer = new ExeSelectingUnInstaller(*pUninstallExe, bSerialDownload, logger, uxLogger);
            else
                *ppPerformer = new ExeSelectingUnInstaller(exePackage, bSerialDownload, logger, uxLogger);
            break;

        case ActionTable::Repair:
            pRepairExe = exePackage.GetRepairOverride();

            // If there is an RepairOverride Exe available, that needs to be used to Repair 
            // this 'exePackage' item, use it.

            if (pRepairExe)
                *ppPerformer = new ExeSelectingRepairer(*pRepairExe, bSerialDownload, logger, uxLogger);
            else
                *ppPerformer = new ExeSelectingRepairer(exePackage, bSerialDownload, logger, uxLogger);
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

} // namespace IronMan
