//-------------------------------------------------------------------------------------------------
// <copyright file="DataToOperand.h" company="Microsoft">
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
//    This data class contains the information that may be needed by an operand.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "LiveOperation.h"
#include "Interfaces\IProvideDataToOperand.h"
#include "common\IronManAssert.h"

namespace IronMan
{
    // The DataToOperand object will be passed to the respective operand at evaluation time.
    // The objective is enable the chainer to pass in environment information for the operands that needed it.
    class DataToOperand : public IProvideDataToOperand
    {
        LiveOperation m_liveOperation;

    public:
        //Constructor
        DataToOperand(Operation::euiOperation initialOperation)
            : m_liveOperation(initialOperation)
        {
        }

        //Hold a reference to the IProvideDataToEngine object so that 
        //we have the latest operation status.
        void SetEngineDataProvider(IProvideDataToEngine* provider) 
        {
            m_liveOperation.SetEngineDataProvider(provider); 
        }

        virtual const CString GetChainerMode() const
        {
            CString csChainerMode = Operation::GetOperationCanonicalString(m_liveOperation.GetOperationFromUi());
            return csChainerMode;
        }
    };
}
