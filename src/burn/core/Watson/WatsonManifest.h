//-------------------------------------------------------------------------------------------------
// <copyright file="WatsonManifest.h" company="Microsoft">
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

#include "WatsonDefinition.h"
#include "..\VersionUtil.h"
#include "..\common\SystemUtil.h"
#include "..\common\Operation.h"

//WER Specific
#include "WerWatson.h"

namespace IronMan
{

//Templatize to enable unit testing.
    template <typename OSHELPER = OSHelper, typename WER = WerWatson> 
    class WatsonManifest
    {
    public:
        WatsonManifest()
        {}

        //--------------------------------------------------------------------------------------
        // SendReport
        //
        // Determine the appropriate Watson reporting mechanism, either Watson Legacy or WER 
        // Below are the execution steps:
        // a. Populate the Bucketting parameters
        // b. Depending on the OS major version
        //    i.  OS major version < 6, report Watson using WatsonClientManifest
        //    ii. OS major version >= 6, report Watson using WerWatson
        //--------------------------------------------------------------------------------------
        void SendReport(const CString& strApplicationName 
                        , const CString& csPackageVersion
                        , const CString& csPackageName
                        , LCID lcid
                        , const CString& strFaultItem
                        , const CString& csPhase
                        , const CString& csAction
                        , const CString& csUiModeString
                        , HRESULT hrReturnCode
                        , const PathCollection& pathCollection
                        , bool bDisplayWatsonUI
                        , Operation::euiOperation eOperation
                        , bool bQueue
                        , const CString& internalMsiError
                        , const CString& csWatsonBoldText
                        , const CString& csCurrentStep = L" ")
        {
            //Step 1: Set the bucketting parameters
            CString arstrParams[WER_MAX_PARAM_COUNT];

            //P1 is the PackageName
            arstrParams[0] = csPackageName;

            //P2 is the Package Version
            arstrParams[1] = csPackageVersion;

            //P3 is the Ironman Version
            arstrParams[2] = CVersionUtil::GetExeFileVersion();

            //P4 is the operation type (Install | Uninstall | Abort)
            CString csOperation;
            csOperation.Format(L"%d", eOperation);
            arstrParams[3] = csOperation;

            //P5 is the item at fault
            arstrParams[4] = strFaultItem;

            //P6 is Current Flag
            CString csCurrentFlag;
            csCurrentFlag = csAction + L"_";
            csCurrentFlag += csPhase + L"_";
            csCurrentFlag += csUiModeString + L"_";
            csCurrentFlag += L"Error";
            arstrParams[5] = csCurrentFlag;

            //P7 is the return code from the install
            arstrParams[6] = GetReturnCode(hrReturnCode);

            //P8 is the Result Detail
            arstrParams[7] = internalMsiError;

            //P9 is environment variable. 
            arstrParams[8] = csCurrentStep;

            HRESULT hr = S_OK;
            OSHELPER os;
            if (os.IsVistaAndAbove())
            {
                WER wer;
                hr = wer.SendReport( strApplicationName
                                    , bQueue
                                    , pathCollection.ConvertToWatsonString()
                                    , arstrParams
                                    , NULL
                                    , csWatsonBoldText);

                if (S_OK == hr)
                {
                    return;
                }
            }
        }

    private:
        //------------------------------------------------------------------------------
        // GetReturnCode
        //
        // This function format the Win32 code and non Win32 code differently.  
        // For Win32 code, it returns the 4 digit code
        // For non-win32 code, it returns 800xxxxx
        // 
        // This function is written so that a user does not need to decode HRESULT for
        // the MSI return code 
        //------------------------------------------------------------------------------
        static CString GetReturnCode(HRESULT hr)
        {
            CString strHr;
            CString strHrWin32;
            strHrWin32.Format(L"%x", HRESULT_CODE(hr));
            (hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,0) ?  strHr.Format(L"%i", wcstoul(strHrWin32,0,16)) : strHr.Format(L"0x%x", hr); 
            return strHr;
        }
    };

}
