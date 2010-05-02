//-------------------------------------------------------------------------------------------------
// <copyright file="InInternal.h" company="Microsoft">
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
//  This class is provided to enable internal testing.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace IronMan
{
struct InTestEnvironment
{
    //Replace this with whatever mechanim you have to detect test environment.
    static bool IsTestEnvironment()
    {
        return false;
    }
};
}
