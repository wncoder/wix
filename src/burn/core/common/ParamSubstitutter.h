//-------------------------------------------------------------------------------------------------
// <copyright file="ParamSubstitutter.h" company="Microsoft">
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


//
//  Class:      ParamSubstituter
//
//  Purpose:    Class to substitute params in a given string.
//
//              This class takes in the prefix & the postfix string to identify the param's in a given string.
//              
//              It also requires a class that implements the method below, to get the value to substitute for a param.
//                  
//                  CString GetParamValue(CString param);
//

struct NullParamValueGetter
{
    static CString GetParamValue(CString param)
    {
        return param;
    }
};


template<typename ParamGetter>
class ParamSubstituter
{
    ParamGetter& m_pg;

    const CString m_prefix;
    const UINT m_prefixLen;
    
    const CString m_postfix;
    const UINT m_postfixLen;

public:
    ParamSubstituter(ParamGetter& pg, LPCWSTR prefix, LPCWSTR postfix)
        : m_pg(pg)
        , m_prefix(prefix)
        , m_prefixLen(m_prefix.GetLength())
        , m_postfix(postfix)
        , m_postfixLen(m_postfix.GetLength())
    {
    }
    int FindParam(const CString& str, int iStart, CString& param)
    {
        //iStart = str.Find(L"$$", iStart);
        iStart = str.Find(m_prefix, iStart);
        if (iStart < 0)
            return -1;
        
        //int iEnd = str.Find(L"$$", iStart+2);
        int iEnd = str.Find(m_postfix, iStart + m_prefixLen);
        if (iEnd < 0)
            return -1;
        
        CopySubString(param, str, iStart, iEnd + m_postfixLen - 1);
        return iStart;
    }

    static void CopySubString(CString& sOut, const CString& sIn, int iStart, int iEnd)
    {
        IMASSERT(iEnd >= iStart);
        int length = iEnd -iStart + 1;
        LPCWSTR szIn = (LPCWSTR)sIn;
        CString temp;
        CString::CopyChars(temp.GetBuffer(length+1), length+1, &szIn[iStart], length);
        temp._ReleaseBuffer(length+1);
        temp.SetAt(length, L'\0');
        sOut += temp;
    }

    CString SubstituteAnyParams(const CString& textIn)
    {
        bool bMadeSubstitutions = false;
        return SubstituteAnyParams(textIn, bMadeSubstitutions);
    }
    CString SubstituteAnyParams(const CString& textIn, bool& bMadeSubstitutions)
    {
        CString textOut;
        bMadeSubstitutions = false;

        int iStart=0;
        
        while(1)
        {
            CString param;
            int iNextStart = FindParam(textIn, iStart, param);
            if (iNextStart < 0)
            {
                //LPCWSTR szIn = (LPCWSTR)(textIn.GetBuffer());
                LPCWSTR szIn = (LPCWSTR)textIn;
                textOut += &szIn[iStart];
                break;
            }

            if (iNextStart > iStart)
                CopySubString(textOut, textIn, iStart, iNextStart-1);

            iStart = iNextStart + param.GetLength();

            CString temp = m_pg.GetParamValue(param);
            textOut +=  temp;
            bMadeSubstitutions = true;
        }
        return textOut;
    }
};
