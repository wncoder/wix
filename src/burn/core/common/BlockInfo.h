//-------------------------------------------------------------------------------------------------
// <copyright file="BlockInfo.h" company="Microsoft">
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
    struct BlockInfo
    {
        CString strText;
        CSimpleArray<CString> arrayOfSubText;

        BlockInfo(const CString& _strText, ILogger& logger)
            : strText(_strText)
        {
        }
        BlockInfo(const CString& _strText, const CSimpleArray<CString>& _arrayOfSubText) 
            : strText(_strText)
            , arrayOfSubText(_arrayOfSubText)
        {
        }
    };

    struct Blocks
    {
        CSimpleArray<BlockInfo> SuccessBlocks;
        CSimpleArray<BlockInfo> StopBlocks;
        CSimpleArray<BlockInfo> WarnBlocks;

        bool HasBlocks(void) const
        {
            return (HasSuccessBlocks() || HasStopBlocks() || HasWarnBlocks());
        }
        bool HasSuccessBlocks(void) const
        {
            return (SuccessBlocks.GetSize() > 0);
        }
        bool HasStopBlocks(void) const
        {
            return (StopBlocks.GetSize() > 0);
        }
        bool HasWarnBlocks(void) const
        {
            return (WarnBlocks.GetSize() > 0);
        }   
    };
}
