//-------------------------------------------------------------------------------------------------
// <copyright file="IBurnPayload.h" company="Microsoft">
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

#pragma once

enum SOURCE_TYPE
{
    SOURCE_TYPE_NONE,
    SOURCE_TYPE_EXTERNAL,
    SOURCE_TYPE_EMBEDDED,
    SOURCE_TYPE_DOWNLOAD,
};

DECLARE_INTERFACE_IID_(IBurnPayload, IUnknown, "e1e09b8a-3fca-11dd-8291-001d09081dd9")
{
    STDMETHOD_(DWORD, Index)() PURE;

    STDMETHOD(Clear)() PURE;

    STDMETHOD(IsCached)(
        __out BOOL* pfCached
        ) PURE;

    STDMETHOD(GetWorkingHandle)(
        __out HANDLE* phWorking
        ) PURE;

    STDMETHOD(GetWorkingPath)(
        __out LPWSTR* psczPath
        ) PURE;
    
    STDMETHOD(GetResumePath)(
        __out LPWSTR* psczPath
        ) PURE;

    STDMETHOD(GetCompletedPath)(
        __out LPWSTR* psczPath
        ) PURE;

    STDMETHOD(GetSource)(
        __out_opt SOURCE_TYPE* pSourceType,
        __out_opt LPWSTR* psczPath
        ) PURE;

    STDMETHOD(GetLocalName)(
        __out_opt LPWSTR* psczLocalName
        ) PURE;

    STDMETHOD(GetEmbeddedSource)(
        __out DWORD64* pdwContainerOffset,
        __out_z LPWSTR* psczEmbeddedId
        ) PURE;

    STDMETHOD(SetCompletedRoot)(
        __in LPCWSTR wzCacheRoot
        ) PURE;

    STDMETHOD_(void, SetIndex)(
        __in DWORD dwId
        ) PURE;

    STDMETHOD_(DWORD64, Size)() PURE;
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
    );

HRESULT BurnCreateEmbeddedPayload(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzSourcePath,
    __in DWORD64 dwContainerOffset,
    __in_z LPCWSTR wzEmbeddedId,
    __in_z LPCWSTR wzLocalName,
    __in_z_opt LPCWSTR wzSpecialPayloadPath,
    __in BOOL fTemporaryPayload,
    __out IBurnPayload** ppPayload
    );
