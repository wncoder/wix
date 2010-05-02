//-------------------------------------------------------------------------------------------------
// <copyright file="BurnPayload.cpp" company="Microsoft">
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
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

class CBurnPayload : public CUnknownImpl<IBurnPayload>
{
public: // IBurnPayload
    STDMETHODIMP_(DWORD) Index()
    {
        return m_dwIndex;
    }

    STDMETHODIMP Clear()
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP IsCached(
        __out BOOL* pfCached
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczCompletedPath = NULL;

        hr = GetCompletedPath(&sczCompletedPath);
        ExitOnFailure(hr, "Failed to get cached path.");

        *pfCached = FileExistsEx(sczCompletedPath, NULL);

    LExit:
        ReleaseStr(sczCompletedPath);

        return hr;
    }

    STDMETHODIMP GetWorkingHandle(
        __out HANDLE* phWorking
        )
    {
        HRESULT hr = S_OK;

        *phWorking = INVALID_HANDLE_VALUE;

        return hr;
    }

    STDMETHODIMP GetWorkingPath(
        __out LPWSTR* psczPath
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczWorkingPath = NULL;
        LPWSTR sczTempPath = NULL;
        DWORD_PTR cchTempPath = MAX_PATH;
        WCHAR wzBundleGuid[39] = {0};

        // If we were given a special temp payload path, then use it
        if (m_sczSpecialPayloadPath && *m_sczSpecialPayloadPath)
        {
            hr = StrAllocString(&sczTempPath, m_sczSpecialPayloadPath, 0);
            ExitOnFailure(hr, "Failed to allocate unique temporary folder for UX");
        }
        else
        {
            // template is <temp>\<bundleId>\<localName>
            hr = StrAlloc(&sczTempPath, cchTempPath);
            ExitOnFailure(hr, "Failed to allocate memory for the temp path.");

            if (!::GetTempPathW(cchTempPath, sczTempPath))
            {
                ExitWithLastError(hr, "Failed to get temp path.");
            }

            hr = StrAllocConcat(&sczTempPath, m_sczBundleId, 0);
            ExitOnFailure(hr, "Failed to concat bundle ID to temp path");
        }

        hr = PathConcat(sczTempPath, m_sczLocalName, &sczWorkingPath);
        ExitOnFailure(hr, "Failed to concatenate local name for working path.");

        *psczPath = sczWorkingPath;
        sczWorkingPath = NULL;

    LExit:
        ReleaseStr(sczTempPath);
        ReleaseStr(sczWorkingPath);

        return hr;
    }

    STDMETHODIMP GetResumePath(
        __out LPWSTR* psczPath
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczResumePath = NULL;
        LPWSTR sczWorkingPath = NULL;

        hr = GetWorkingPath(&sczWorkingPath);
        ExitOnFailure(hr, "Failed to get payload's working path.");

        hr = StrAllocFormatted(&sczResumePath, L"%ls.r", sczWorkingPath);
        ExitOnFailure(hr, "Failed to allocate memory for resume path.");

        *psczPath = sczResumePath;
        sczResumePath = NULL;

    LExit:
        ReleaseStr(sczWorkingPath);
        ReleaseStr(sczResumePath);

        return hr;
    }

    STDMETHODIMP GetCompletedPath(
        __out LPWSTR* psczCompletedPath
        )
    {
        HRESULT hr = S_OK;

        if (m_fTemporary)
        {
            hr = GetWorkingPath(psczCompletedPath);
        }
        else
        {
            hr = StrAllocString(psczCompletedPath, m_sczCompletedPath, 0);
        }

        return hr;
    }

    STDMETHODIMP GetSource(
        __out_opt SOURCE_TYPE* pSourceType,
        __out_opt LPWSTR* psczPath
        )
    {
        HRESULT hr = S_OK;

        if (pSourceType)
        {
            *pSourceType = m_sourceType;
        }

        if (psczPath)
        {
            hr = StrAllocString(psczPath, m_sczSourcePath, 0);
            ExitOnFailure(hr, "Failed to copy source path.");
        }

    LExit:
        return hr;
    }

    STDMETHODIMP GetLocalName(
        __out_opt LPWSTR* psczLocalName
        )
    {
        HRESULT hr = StrAllocString(psczLocalName, m_sczLocalName, 0);
        ExitOnFailure(hr, "Failed to copy Local Name.");

    LExit:
        return hr;
    }

    STDMETHODIMP GetEmbeddedSource(
        __out DWORD64* pdwContainerOffset,
        __out_z LPWSTR* psczEmbeddedId
        )
    {
        HRESULT hr = StrAllocString(psczEmbeddedId, m_sczEmbeddedId, 0);
        ExitOnFailure(hr, "Failed to copy embedded id.");

        *pdwContainerOffset = m_dwEmbeddedContainerOffset;

    LExit:
        return hr;
    }

    STDMETHODIMP SetCompletedRoot(
        __in LPCWSTR wzCompletedRoot
        )
    {
        HRESULT hr = S_OK;

        ReleaseNullStr(m_sczCompletedPath);
        hr = PathConcat(wzCompletedRoot, m_sczLocalName, &m_sczCompletedPath);
        ExitOnFailure(hr, "Failed to allocate completed path.");

    LExit:
        return hr;
    }

    STDMETHODIMP_(void) SetIndex(
        __in DWORD dwIndex
        )
    {
        m_dwIndex = dwIndex;
    }

    STDMETHODIMP_(DWORD64) Size()
    {
        return m_dw64Size;
    }

public:
    //
    // Constructor - intitialize member variables.
    //
    CBurnPayload(
        __in SOURCE_TYPE sourceType,
        __in_z LPCWSTR wzBundleId,
        __in_z LPCWSTR wzSourcePath,
        __in DWORD64 dwContainerOffset,
        __in_z_opt LPCWSTR wzEmbeddedId,
        __in_z_opt LPCWSTR wzLocalName,
        __in_z_opt LPCWSTR wzSpecialPayloadPath,
        __in BOOL fTemporary,
        __out HRESULT* pHR
        )
    {
        HRESULT hr = S_OK;

        m_sczBundleId = NULL;
        m_sczSourcePath = NULL;
        m_sczEmbeddedId = NULL;
        m_sczLocalName = NULL;
        m_sczCompletedPath = NULL;
        m_sczSpecialPayloadPath = NULL;

        if (wzSpecialPayloadPath && *wzSpecialPayloadPath)
        {
            hr = StrAllocString(&m_sczSpecialPayloadPath, wzSpecialPayloadPath, 0);
            ExitOnFailure(hr, "Failed to allocate payload path string.");
        }

        m_dwIndex = 0;
        m_sourceType = sourceType;
        m_fTemporary = fTemporary;
        m_dw64Size = 0; // TODO: populate with actual size
        m_dwEmbeddedContainerOffset = dwContainerOffset;

        hr = StrAllocString(&m_sczBundleId, wzBundleId, 0);
        ExitOnFailure(hr, "Failed to copy string for bundle id.");

        hr = StrAllocString(&m_sczSourcePath, wzSourcePath, 0);
        ExitOnFailure(hr, "Failed to allocate source path.");

        if (wzEmbeddedId)
        {
            hr = StrAllocString(&m_sczEmbeddedId, wzEmbeddedId, 0);
            ExitOnFailure(hr, "Failed to allocate embedded id.");
        }

        if (wzLocalName)
        {
            hr = StrAllocString(&m_sczLocalName, wzLocalName, 0);
            ExitOnFailure(hr, "Failed to allocate local name.");
        }

    LExit:
        *pHR = hr;
    }


    //
    // Destructor - release member variables.
    //
    ~CBurnPayload()
    {
        ReleaseStr(m_sczBundleId);
        ReleaseStr(m_sczSourcePath);
        ReleaseStr(m_sczEmbeddedId);
        ReleaseStr(m_sczLocalName);
        ReleaseStr(m_sczCompletedPath);
        ReleaseStr(m_sczSpecialPayloadPath);
    }

protected:
    DWORD m_dwIndex;
    LPWSTR m_sczBundleId;
    BOOL m_fTemporary;
    SOURCE_TYPE m_sourceType;
    DWORD64 m_dw64Size;
    LPWSTR m_sczSourcePath;
    LPWSTR m_sczEmbeddedId;
    LPWSTR m_sczLocalName;
    LPWSTR m_sczCompletedPath;
    LPWSTR m_sczSpecialPayloadPath;
    DWORD64 m_dwEmbeddedContainerOffset;
};

HRESULT BurnCreatePayload(
    __in SOURCE_TYPE sourceType,
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzSourcePath,
    __in_z LPCWSTR wzLocalName,
    __in_z_opt LPCWSTR wzSpecialPayloadPath,
    __in DWORD64 cbSize,
    __in_opt LPCBYTE prgDigest,
    __in DWORD cbDigest,
    __in BOOL fTemporaryPayload,
    __out IBurnPayload** ppPayload
    )
{
    UNREFERENCED_PARAMETER(cbSize);
    UNREFERENCED_PARAMETER(prgDigest);
    UNREFERENCED_PARAMETER(cbDigest);

    HRESULT hr = S_OK;

    CBurnPayload* pPayload = new CBurnPayload(sourceType, wzBundleId, wzSourcePath, 0, NULL, wzLocalName, wzSpecialPayloadPath, fTemporaryPayload, &hr);
    ExitOnNull(pPayload, hr, E_OUTOFMEMORY, "Failed to create new payload.");
    ExitOnFailure(hr, "Failed to initialize payload.");

    *ppPayload = pPayload;
    pPayload = NULL;

LExit:
    ReleaseObject(pPayload);

    return hr;
}

HRESULT BurnCreateEmbeddedPayload(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzSourcePath,
    __in DWORD64 dwContainerOffset,
    __in_z LPCWSTR wzEmbeddedId,
    __in_z LPCWSTR wzLocalName,
    __in_z_opt LPCWSTR wzSpecialPayloadPath,
    __in BOOL fTemporaryPayload,
    __out IBurnPayload** ppPayload
    )
{
    HRESULT hr = S_OK;

    CBurnPayload* pPayload = new CBurnPayload(SOURCE_TYPE_EMBEDDED, wzBundleId, wzSourcePath, dwContainerOffset, wzEmbeddedId, wzLocalName, wzSpecialPayloadPath, fTemporaryPayload, &hr);
    ExitOnNull(pPayload, hr, E_OUTOFMEMORY, "Failed to create new embedded payload.");
    ExitOnFailure(hr, "Failed to initialize embedded payload.");

    *ppPayload = pPayload;
    pPayload = NULL;

LExit:
    ReleaseObject(pPayload);

    return hr;
}
