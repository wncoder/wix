//-------------------------------------------------------------------------------------------------
// <copyright file="LiveOperation.h" company="Microsoft">
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

#include "Interfaces\IProvideDataToEngine.h"
#include "CmdLineParser.h"

//This class holds the true MODE the chainer is in. 
namespace IronMan
{   
template <typename CmdLineSwitches>
class LiveOperationT
{
    IProvideDataToEngine* m_provideDataToEngine;
    Operation::euiOperation m_initialOperation;
public:
    LiveOperationT(Operation::euiOperation initialOperation)
        : m_provideDataToEngine(NULL)
        , m_initialOperation(initialOperation)
    {}
    virtual ~LiveOperationT() {}

    void SetEngineDataProvider(IProvideDataToEngine* provider) 
    { 
        m_provideDataToEngine = provider; 
    }

    //The initial state may change because of the MaintenanceIfBlock,
    //thus need a way to reset the initial state.
    void ResetInitialOperation(Operation::euiOperation initialOperation)
    {
        m_initialOperation = initialOperation;
    }
    bool IsInitialOperationInMaintenanceMode() const
    {
        return ((m_initialOperation == Operation::uioRepairing) || (m_initialOperation == Operation::uioUninstalling));
    }
    Operation::euiOperation GetOperationFromUi() const
    {
        CmdLineSwitches switches;

        if (!switches.InteractiveMode() || NULL == m_provideDataToEngine)
        {
            return m_initialOperation;
        }
        else
        {
            //if (IsInitialOperationInMaintenanceMode())
            //{
            //    // Operation::euiOperation operation = m_provideDataToEngine->GetMaintenanceModeOperation();
            //    //This check is put in because in passive mode, we don't show the Maintenance mode page.  
            //    //This resulted in the function returning uioUninitalized.
            //    if (Operation::uioUninitalized == operation)
            //    {
            //        //The initial value depends on the command line passing through
            //        //Also if we are in install operation and in maintenance mode, we will default to Repair.
            //        operation = m_initialOperation;
            //    }
            //    return operation;
            //}
            return m_initialOperation;
        }
    }

    // Returns initial value of live operation
    Operation::euiOperation GetOperation(void) const
    {
        return m_initialOperation;
    }
};

typedef LiveOperationT<CCmdLineSwitches>    LiveOperation;

}
