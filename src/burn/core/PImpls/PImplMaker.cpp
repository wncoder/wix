//-------------------------------------------------------------------------------------------------
// <copyright file="PImplMaker.cpp" company="Microsoft">
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

#include "precomp.h"

#include "EnginePImplsPriv.h"
#include "MsiInstallerPImplsPriv.h"
#include "CompositePerformerPimplsPriv.h"
#include "NotifyControllerPImplsPriv.h"
#include "ExeInstallerPImplsPriv.h"
#include "CompositeDownloaderPImplsPriv.h"
#include "MSPInstallerPImplsPriv.h"
#include "MultipleMSPInstallerPImplsPriv.h"
#include "MultipleMSPUninstallerPImplsPriv.h"
#include "CoordinatorPimplsPriv.h"
#include "AgileMSIInstallerPImplsPriv.h"

extern "C" void * WINAPI MakePImpl(int id)
{
    switch(id)
    {
    case ENGINEDATAPIMPL:
        return EnginePImplsPrivate::CEngineDataPImpl::MakePImpl;
    case MSIINSTALLERPIMPL:
        return MsiInstallerPImplsPrivate::CMsiInstallerPImpl::MakePImpl;
    case COMPOSITEPERFORMERPIMPL:
        return CompositePerformerPImplsPrivate::CCompositePerformerPImpl::MakePImpl;
    case NOTIFYCONTROLLERPIMPL:
        return NotifyControllerPImplsPrivate::CNotifyControllerPImpl::MakePImpl;
    case EXEINSTALLERPIMPL:
        return ExeInstallerPImplsPrivate::CExeInstallerPImpl::MakePImpl;
    case COMPOSITEDOWNLOADERPIMPL:
        return CompositeDownloaderPImplsPrivate::CCompositeDownloaderPImpl::MakePImpl;
    case MSPINSTALLERPIMPL:
        return MSPInstallerPImplsPrivate::CMSPInstallerPImpl::MakePImpl;
    case MULTIPLEMSPINSTALLERPIMPL:
        return MultipleMSPInstallerPImplsPrivate::CMultipleMSPInstallerPImpl::MakePImpl;
    case MULTIPLEMSPUNINSTALLERPIMPL:
        return MultipleMSPUninstallerPImplsPrivate::CMultipleMSPUninstallerPImpl::MakePImpl;
    case COORDINATORPIMPL:
        return CoordinatorPImplsPrivate::CCoordinatorPImpl::MakePImpl;
    case AGILEMSIINSTALLERPIMPL:
        return AgileMsiInstallerPImplsPrivate::CAgileMsiInstallerPImpl::MakePImpl;
    default:
        return NULL;
    }
}
