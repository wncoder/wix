//-------------------------------------------------------------------------------------------------
// <copyright file="IRebootDialog.h" company="Microsoft">
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
class IRebootDialog
{
public:
    enum Result
    {
        RebootNow = IDOK,
        RebootLater = 0,
    };
protected:
    virtual ~IRebootDialog() {}
public:
    virtual void DestroySelf() = 0;
    virtual Result Run(void) = 0;
};
}
