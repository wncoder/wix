//-------------------------------------------------------------------------------------------------
// <copyright file="OperationData.h" company="Microsoft">
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

#include "interfaces\IOperationData.h"
#include "ux\uxenum.h"

namespace IronMan
{
    //Holds the data that is being passed around.
    class OperationData : public IOperationData
    {
        const IProvideDataToOperand& m_dataToOperand;
        const IPackageData& m_packageData;

    public:
        //Constructor
        OperationData(const IProvideDataToOperand& dataToOperand
                        , const IPackageData& packageData)
            : m_dataToOperand(dataToOperand)
            , m_packageData(packageData)
        {}

        //Destructor
        virtual ~OperationData()
        {}

        //Return the data captured for Operands
        virtual const IProvideDataToOperand& GetDataToOperandData() const
        {
            return m_dataToOperand;
        }

        //Return the package data that is being captured.
        virtual const IPackageData& GetPackageData() const
        {
            return m_packageData;
        }
    };
}
