//-------------------------------------------------------------------------------------------------
// <copyright file="StringUtil.h" company="Microsoft">
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
    class StringUtil
    {
        public:
            //Have to do this translation as required by user experience data.
            static const CString FromHresult(HRESULT hr)
            {
                CString strHr;
                if (((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,0))
                    || ( 0 == (hr & 0xFFFF0000)))
                {
                    strHr.Format(L"%08x", HRESULT_CODE(hr));
                }
                else
                {
                    strHr.Format(L"%08x", hr);
                }
                return strHr;
            }

            //Translate LCID to string
            static const CString FromLcid(LCID lcid)
            {
                CString str;
                str.Format(L"%d", lcid);
                return str;
            }

            // Get the Url as a CString for convenience
            static const CString FromUrl(const CUrl srcPath)
            {
                CString strSourcePath;
                DWORD len = srcPath.GetUrlLength() + 1;

                if (!srcPath.CreateUrl(strSourcePath.GetBuffer(len), &len))
                {
                    return L"";
                }
                else
                {
                    strSourcePath._ReleaseBuffer();
                    return strSourcePath;
                }
            }

            //Convert DWORD to string
            static const CString FromDword(const DWORD dw)
            {
                CString strResult;
                strResult.Format(L"%u", dw);
                return strResult;
            }

            //Convert from string array to a deliminitor list.
            static CString FromArray(const CSimpleArray<CString>& list, const CString& strDelimiter)
            {
                CString strList;
                for ( int iCount = 0; iCount < list.GetSize(); ++iCount)
                {
                    if ( !strList.IsEmpty() )
                    {
                        strList.Append(strDelimiter);
                    }
                    strList.Append(list[iCount]);
                }
                return strList;
            }

             //Convert from string array to a deliminitor list.
            static CString FromBool(bool bValue)
            {
                return bValue ? L"True" : L"False";
            }
    };
}
