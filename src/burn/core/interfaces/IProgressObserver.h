//-------------------------------------------------------------------------------------------------
// <copyright file="IProgressObserver.h" company="Microsoft">
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

#include "CmdLineParser.h"

namespace IronMan
{
#define NOTHING_APPLIES 0x77777777 //some special number to represent NOTHING is applicable.

class IProgressObserver
{
public:
    virtual int OnProgress(unsigned char) = 0; // 0 - 255:  255 == done
    virtual int OnProgressDetail(unsigned char) = 0;

    enum State
    {
        NotStarted,
        Downloading,
        Installing,
        Rollback,
        Uninstalling,
        Repairing,
        UserCancelled,
        EnableCancel,
        DisableCancel,
        Copying,
        VerifyingFile,
        Updating,
        UninstallingPatch,
        WaitingForAnotherInstallToComplete,
        DownloadItemComplete
    };
    virtual void OnStateChange(State enumVal)  = 0;
    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) = 0;
    virtual void Finished(HRESULT) = 0;
    virtual void OnRebootPending() = 0;

    // ----------------------------------------------------------------------------------------
    // UninstallPatchTransform()
    // If the /UninstallPatch switch is set, this transforms the state to UninstallingPatch
    // If the /UninstallPatch switch is not set, returns the same state that was passed in
    // ----------------------------------------------------------------------------------------
    static IProgressObserver::State UninstallPatchTransform(IProgressObserver::State state)
    {
        // if the /UninstallPatch switch is not set, return unmodified
        CCmdLineSwitches switches;
        CString patchCode(switches.UninstallPatch());
        if (patchCode.IsEmpty())
        {
            return state;
        }
        else
        {
            return IProgressObserver::UninstallingPatch;
        }
    }
};

class NullProgressObserver : private IProgressObserver
{
    virtual int OnProgress(unsigned char) { return IDOK; }
    virtual int OnProgressDetail(unsigned char) { return IDOK; }

    virtual void Finished(HRESULT) {}
    virtual void OnStateChange(State enumVal)  {}
    virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo) {}
    virtual void OnRebootPending() {};

public:
    static IProgressObserver& GetNullProgressObserver()
    {
        static NullProgressObserver npo;
        return npo;
    }
};

}
