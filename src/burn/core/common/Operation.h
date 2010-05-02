//-------------------------------------------------------------------------------------------------
// <copyright file="Operation.h" company="Microsoft">
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

//------------------------------------------------------------------------------
// Enumeration for the operation being performed at the global level, i.e.
// Installing, Uninstalling, Repairing or Creating Layout.
//------------------------------------------------------------------------------
namespace IronMan
{
    class Operation
    {
    public:
        enum euiOperation
        {
            uioUninitalized = 0,
            uioInstalling   = 1,
            uioUninstalling = 2,
            uioRepairing    = 3,
            uioCreateLayout = 4,
            uioMaintenance  = 5,
            uioCreateUnattend = 6,
            uioUninstallingPatch = 7,
            uioNone = 8
        };

    public:
        // Constructor
        Operation(euiOperation uioOperation) 
            :m_uioOperation(uioOperation)
        {
        }

        // Constructor
        // Encapsulate the computing of operations from commandline switches
        Operation()
            :m_uioOperation(ComputeOperationFromSwitches<CCmdLineSwitches>())
        {
        }

        // ----------------------------------------------------------------------------------------
        // GetOperation()
        // Return type of operation, e.g. installation, uninstall, repair
        // ----------------------------------------------------------------------------------------
        euiOperation GetOperation() const
        {
            return m_uioOperation;
        }

        // ----------------------------------------------------------------------------------------
        // GetOperationCanonicalString()
        // Returns a canonical (English only) string for the opeartion, useful for logging purposes
        // ----------------------------------------------------------------------------------------
        static CString  GetOperationCanonicalString(euiOperation uioOperation)
        {
            CString strOperation("Error");

            switch (uioOperation)
            {
            case Operation::uioInstalling :
                strOperation = L"Installing";
                break;
            case Operation::uioUninstalling :
                strOperation = L"Uninstalling";
                break;
            case Operation::uioRepairing :
                strOperation = L"Repairing";
                break;
            case Operation::uioCreateLayout :
                strOperation = L"Creating Layout";
                break;
            case Operation::uioUninstallingPatch :
                strOperation = L"Uninstalling Patch";
                break;
            default:
                // Operation type should be known, e.g. Installing
                break;
            }

            return strOperation;
        }

        // ----------------------------------------------------------------------------------------
        // GetCmdLineSwitchFromOperation()
        // Returns a canonical (English only) string for the opeartion, useful for logging purposes
        // ----------------------------------------------------------------------------------------
        static CString  GetCmdLineSwitchFromOperation(euiOperation uioOperation)
        {
            CString strOperation("Error");

            switch (uioOperation)
            {
            case Operation::uioInstalling :
                strOperation = L"install";
                break;
            case Operation::uioUninstalling :
                strOperation = L"uninstall";
                break;
            case Operation::uioRepairing :
                strOperation = L"repair";
                break;
            case Operation::uioCreateLayout :
                strOperation = L"createlayout";
                break;
            case Operation::uioUninstallingPatch :
                strOperation = L"uninstallpatch";
                break;
            default:
                // Operation type should be known, e.g. Installing
                break;
            }

            return strOperation;
        }

        // ----------------------------------------------------------------------------------------
        // UninstallPatchTransform()
        // If the /UninstallPatch switch is set, this transforms the operation to uioUninstallingPatch
        // If the /UninstallPatch switch is not set, returns the same operation that was passed in
        // ----------------------------------------------------------------------------------------
        template<typename TCmdLineSwitches>
        static Operation::euiOperation UninstallPatchTransform(Operation::euiOperation op)
        {
            // if the /UninstallPatch switch is not set, return unmodified
            TCmdLineSwitches switches;
            CString patchCode(switches.UninstallPatch());
            if (patchCode.IsEmpty())
            {
                return op;
            }
            else
            {
                return Operation::uioUninstallingPatch;
            }
        }

    //Made protected for testing purpose.
    protected:
        // ----------------------------------------------------------------------------------------
        // DetermineOperationFromSwitches()
        // Compute the operation based on the command line switches.
        // ----------------------------------------------------------------------------------------
        template<typename TCmdLineSwitches>
        static Operation::euiOperation ComputeOperationFromSwitches()
        {
            TCmdLineSwitches switches;
            if (!switches.CreateLayout().IsEmpty())
            {
                return Operation::uioCreateLayout;
            }
            else if (switches.UninstallMode())
            {
                return Operation::uioUninstalling;
            }
            else if (switches.MaintenanceMode())
            {
                return Operation::uioRepairing;
            }
            else
            {
                return Operation::uioInstalling;
            }
        }

    private:
        euiOperation m_uioOperation;
    };

}
