//-------------------------------------------------------------------------------------------------
// <copyright file="MsgWaitForObject.h" company="Microsoft">
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

//
// Waits for the given object to be signaled, while allowing other input events to be processed
//

class DoNothing
{
public:
    void operator() (void)
    {
    }
};

template<typename T>
class MsgWaitForObjectT
{
public:
    static void Wait(const HANDLE objectHandle, T& t = DoNothing())
    {
        DWORD dwRet = WAIT_TIMEOUT;

        do
        {
            dwRet = ::MsgWaitForMultipleObjects( 
                        1,              // number of object handles 
                        &objectHandle,  // array of object handles
                        FALSE,          // wait for all objects to be signaled
                        100,            // time-out interval, in milliseconds
                        QS_ALLINPUT );  // Wake mask

            t();

            if (dwRet == WAIT_OBJECT_0 + 1)
            {
                //gotta pump because msgwaitformultipleobjects says so
                MSG msg;
                while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

        } while( dwRet != WAIT_OBJECT_0 && dwRet != WAIT_FAILED );
    }
};

typedef MsgWaitForObjectT<DoNothing> MsgWaitForObject;

}
