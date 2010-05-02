//-------------------------------------------------------------------------------------------------
// <copyright file="CustomErrors.h" company="Microsoft">
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
//------------------------------------------------------------------------------
// CustomErrors
//
// Custom errors that IronMan can return
//-------------------------------------------------------------------------------
class CustomErrors
{
public:
    //------------------------------------------------------------------------------
    // enum for custom errors that IronMan can return
    //------------------------------------------------------------------------------
    enum Errors
    {
        // This is returned by IronMan if a StopBlocker was hit 
        // or a system requirement (disk space, process block or service block) was hit
        // during quiet or passive mode.
        StopBlockerHitOrSystemRequirementNotMet = 5100,
        // This is returned by Ironman if an internal Error or User Error was hit.
        InternalErrorOrUserError = 5101
    };

    //------------------------------------------------------------------------------
    // GetErrorString
    //
    // Gets text version of the custom error.  If not a custom error returns a empty string
    //-------------------------------------------------------------------------------
    static CString GetErrorString( DWORD dwError)
    {
        CString errorString;
        switch (dwError)
        {
        case CustomErrors::StopBlockerHitOrSystemRequirementNotMet :
            {
                errorString = L"A StopBlock was hit or a System Requirement was not met.";
                break;
            }
        case CustomErrors::InternalErrorOrUserError :
            {
                errorString = L"An internal or user error was encountered.";
                break;
            }
        }
        return errorString;
    }

};
}
