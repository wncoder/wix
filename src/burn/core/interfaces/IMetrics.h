//-------------------------------------------------------------------------------------------------
// <copyright file="IMetrics.h" company="Microsoft">
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
//      IMetrics - The inferface all Metrics class are to implement.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace IronMan
{
    class IMetrics
    {
    public:
        //Continue from a previous session
        virtual HRESULT ContinueSession() = 0;

        //Start a new session
        virtual HRESULT StartSession() =0;

        //CLose the session and send the report
        virtual HRESULT EndAndSend(const CPath& pthDataFileFullPath, const CPath& pthLoaderFileFullPath)=0;

        //Write out a string column of the stream.
        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberofColumn, const CString& csStringToWrite)=0;

        //Write out a DWORD column of the stream.
        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberofColumn, const DWORD& dw)=0;

        //Write out a DWORD datapoint.
        virtual HRESULT Write(__in DWORD dwId, __in_opt DWORD dwVal)=0;

        //Write out a HRESULT datapoint.
        virtual HRESULT Write(__in DWORD dwId, __in_opt HRESULT hrVal)=0;

        //Write out a bool datapoint.
        virtual HRESULT Write(__in DWORD dwId, __in_opt bool bVal)=0;

        //Write out a String datapoint.
        virtual HRESULT Write(__in DWORD dwId, __in_opt const CString& csVal)=0;
    };
}