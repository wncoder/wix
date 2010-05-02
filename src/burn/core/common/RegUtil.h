//-------------------------------------------------------------------------------------------------
// <copyright file="RegUtil.h" company="Microsoft">
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
    class RegUtil
    {
    public:
        static bool GetHKLMRegDWORD(CString subKey, CString valueName, DWORD& keyValue )
        {            
            bool result = false;
            DWORD bufferSize = sizeof(DWORD);		    
            HKEY hkey;
            if (ERROR_SUCCESS == RegOpenKeyEx (	HKEY_LOCAL_MACHINE, 
                                                subKey, 
                                                0, 
                                                KEY_QUERY_VALUE, 
                                                &hkey))
            {
                if (ERROR_SUCCESS == RegQueryValueEx(	hkey,
                                                        valueName,
                                                        NULL,
                                                        NULL,
                                                        (LPBYTE) &keyValue,
                                                        &bufferSize))
                {
                    result = true;                    
                }
            }
            RegCloseKey(hkey);
            return result;
        }        
    };
}
