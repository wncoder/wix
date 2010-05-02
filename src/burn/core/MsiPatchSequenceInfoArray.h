//-------------------------------------------------------------------------------------------------
// <copyright file="MsiPatchSequenceInfoArray.h" company="Microsoft">
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
// MsiPatchSequenceInfoArray class
//
// Used to create an PMSIPATCHSEQUENCEINFO array to be used during MSI calls
//-------------------------------------------------------------------------------
class MsiPatchSequenceInfoArray
{
    int m_size;
    CSimpleArray<CComBSTR> m_bstrBlobArray;
    const CSimpleArray<CString> m_patchFileList;
    PMSIPATCHSEQUENCEINFO m_pMsiPatchSequenceInfo;

public:
    //------------------------------------------------------------------------------
    // MsiXmlBlob Constructor
    //-------------------------------------------------------------------------------
    MsiPatchSequenceInfoArray(CComPtr<IXMLDOMElement> spMsiXmlBlobElement) 
    {
        m_size = CreateBSTRBlobArray(spMsiXmlBlobElement);
        m_pMsiPatchSequenceInfo = new MSIPATCHSEQUENCEINFO[m_size];

        for (int i=0; i<m_size; ++i)
        {
            m_pMsiPatchSequenceInfo[i].szPatchData = m_bstrBlobArray[i];
            m_pMsiPatchSequenceInfo[i].ePatchDataType = MSIPATCH_DATATYPE_XMLBLOB;
            m_pMsiPatchSequenceInfo[i].dwOrder=-1;
            m_pMsiPatchSequenceInfo[i].uStatus=-1;
        }
    }

    //------------------------------------------------------------------------------
    // Patch File Array Constructor
    //-------------------------------------------------------------------------------
    MsiPatchSequenceInfoArray(const CSimpleArray<CString>& patchFileList)
        : m_patchFileList(patchFileList)
    {
        m_size = m_patchFileList.GetSize();
        m_pMsiPatchSequenceInfo = new MSIPATCHSEQUENCEINFO[m_size];
        for (int i=0; i<m_size; ++i)
        {
            m_pMsiPatchSequenceInfo[i].szPatchData = patchFileList[i];
            m_pMsiPatchSequenceInfo[i].ePatchDataType = MSIPATCH_DATATYPE_PATCHFILE;
            m_pMsiPatchSequenceInfo[i].dwOrder=-1;
            m_pMsiPatchSequenceInfo[i].uStatus=-1;
        }

    }

    //------------------------------------------------------------------------------
    // Destructor
    //-------------------------------------------------------------------------------
    ~MsiPatchSequenceInfoArray()
    {
        delete [] m_pMsiPatchSequenceInfo;
        m_pMsiPatchSequenceInfo = NULL;
        m_size = 0;
    }

    UINT Size(void) const
    {
        return m_size;
    }

    operator PMSIPATCHSEQUENCEINFO()
    {
        return m_pMsiPatchSequenceInfo;
    }

    MSIPATCHSEQUENCEINFO& operator[] (int i) 
    {
        IMASSERT2(i < m_size, L"Trying to acess an array out of boubds");
        if (i >= m_size)
            throw ERROR_INVALID_ACCESS;

        return this->m_pMsiPatchSequenceInfo[i];
    }

    void ClearStatusAndOrder(void)
    {
        for (int i=0; i<m_size; ++i)
        {
            m_pMsiPatchSequenceInfo[i].dwOrder = -1;
            m_pMsiPatchSequenceInfo[i].uStatus = -1;
        }
    }

    //------------------------------------------------------------------------------
    // MsiDeterminePatchSequence might succeed, yet the uStatus of all the elements of the PatchSequenceInfo
    // might not be ERROR_SUCCESS(0).  Example product installed, but patch not applicable due to missing base patch
    // in that case the uStatus = ERROR_PATCH_TARGET_NOT_FOUND(1642)
    // Verify that at least one patch's uStatus = ERROR_SUCCESS
    // Also the uStatus can be ERROR_SUCCESS, but the patch is obsolete or superceded
    // in this case the patch does apply and the dwOrder is set to -1
    //------------------------------------------------------------------------------
    bool HasAtleastOneApplicablePatch(void) const
    {
        // If a patch is applicable, then its uStatus = ERROR_SUCCESS
        for (int i=0; i<m_size; ++i)
        {
            if (  IsPatchApplicable(i) )
            {
                return true;
            }
        }
        return false;
    }

    CSimpleArray<CString> ApplicablePatches()
    {
        CSimpleArray<CString> applicablePatches;
        for (int i=0; i<m_size; ++i)
        {
            if ( IsPatchApplicable(i) )
            {
                applicablePatches.Add(m_pMsiPatchSequenceInfo[i].szPatchData);
            }
        }
        return applicablePatches;
    }

private:

    // -------------------------------------
    // IsPatchApplicable
    // Given an index into m_pMsiPatchSequenceInfo, returns true if the patch is applicable
    // -------------------------------------
    bool IsPatchApplicable( const int iPatchIndex) const
    {
        // If a patch is applicable, then its uStatus = ERROR_SUCCESS
        //      Superseded patches need to be installed.
        //      The idea is that if the superseding patch(es) is later removed, the superseded patch is now 
        //      applicable and applied. This is good for caching security updates on disk for future benefit
        //      and is the default behavior of Windows Installer.
        //
        if ( iPatchIndex >= 0 && iPatchIndex < m_size && ERROR_SUCCESS == m_pMsiPatchSequenceInfo[iPatchIndex].uStatus )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    int CreateBSTRBlobArray(CComPtr<IXMLDOMElement> spMsiXmlBlobElement)
    {
        int count = 0;

        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spMsiXmlBlobElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            m_bstrBlobArray.Add(CComBSTR());
            spChild->get_xml(&m_bstrBlobArray[count]);
            ++count;

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    m_bstrBlobArray.Add(CComBSTR());
                    spSibling->get_xml(&m_bstrBlobArray[count]);
                    ++count;
                }
                spChild = spSibling;
            } while (!!spChild);
        }

        IMASSERT(m_bstrBlobArray.GetSize() == count);
        return count;
    }
};
}
