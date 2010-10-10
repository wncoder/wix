//-------------------------------------------------------------------------------------------------
// <copyright file="dictutil.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Dict helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// These should all be primes, and spaced reasonably apart (currently each is about 4x the last)
const DWORD MAX_BUCKET_SIZES[] = {
    32779,
    131111,
    524341,
    2097709,
    8390857,
    33563437,
    134253719,
    537014927,
    2148059509
    };

// However many items are in the cab, let's keep the buckets at least 8 times that to avoid collisions
#define MAX_ITEMS_TO_BUCKETS_RATIO 8

enum DICT_TYPE
{
    DICT_INVALID = 0,
    DICT_EMBEDDED_KEY = 1,
    DICT_STRING_LIST = 2
};

struct STRINGDICT_STRUCT
{
    DICT_TYPE dtType;

    // Index into MAX_BUCKET_SIZES (array of primes), representing number of buckets we've allocated
    DWORD dwBucketSizeIndex;

    // Number of items currently stored in the dict buckets
    DWORD dwNumItems;

    // Byte offset of key within bucket value, for collision checking - see
    // comments above DictCreateEmbeddedKey() implementation for further details
    size_t cByteOffset;

    // The actual stored buckets
    void **ppvBuckets;

    // The actual stored items in the order they were added (used for auto freeing or enumerating)
    void **ppvItemList;
    DWORD dwNumItemsInList;
};

static DWORD StringHash(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString
    );
static BOOL IsMatchExact(
    __in const void *pvHandle,
    __in DWORD dwMatchIndex,
    __in_z LPCWSTR pszOriginalString
    );
static HRESULT GetValue(
    __in void *pvHandle,
    __in_z LPCWSTR pszString,
    __out_opt void **ppvValue
    );
static HRESULT GetInsertIndex(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString,
    __out DWORD *pdwOutput
    );
static HRESULT GetIndex(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString,
    __out DWORD *pdwOutput
    );

// The dict will store a set of keys (as wide-char strings) and a set of values associated with those keys (as void *'s).
// However, to support collision checking, the key needs to be represented in the "value" object (pointed to
// by the void *). The "stByteOffset" parameter tells this dict the byte offset of the "key" string pointer
// within the "value" object. Use the offsetof() macro to fill this out.
//
// Use DictAddValue() and DictGetValue() with this dictionary type.
extern "C" HRESULT DAPI DictCreateWithEmbeddedKey(
    __out STRINGDICT_HANDLE* psdHandle,
    __in DWORD dwNumExpectedItems,
    __in size_t cByteOffset
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(psdHandle, hr, E_INVALIDARG, "Handle not specified while creating dict");

    // Allocate the handle
    *psdHandle = static_cast<STRINGDICT_HANDLE>(MemAlloc(sizeof(STRINGDICT_STRUCT), FALSE));
    ExitOnNull(*psdHandle, hr, E_OUTOFMEMORY, "Failed to allocate dictionary object");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(*psdHandle);

    // Fill out the new handle's values
    shHandle->dtType = DICT_EMBEDDED_KEY;
    shHandle->cByteOffset = cByteOffset;
    shHandle->dwBucketSizeIndex = 0;
    shHandle->dwNumItems = 0;
    shHandle->dwNumItemsInList = 0;
    shHandle->ppvItemList = NULL;

    // Make shHandle->dwBucketSizeIndex point to the appropriate spot in the prime
    // array based on expected number of items and items to buckets ratio
    // Careful: the "-1" in "countof(MAX_BUCKET_SIZES)-1" ensures we don't end
    // this loop past the end of the array!
    while (shHandle->dwBucketSizeIndex < (countof(MAX_BUCKET_SIZES)-1) &&
           MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex] < dwNumExpectedItems * MAX_ITEMS_TO_BUCKETS_RATIO)
    {
        shHandle->dwBucketSizeIndex++;
    }

    // Finally, allocate our initial buckets
    shHandle->ppvBuckets = static_cast<void**>(MemAlloc(sizeof(void *) * MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex], TRUE));
    ExitOnNull(shHandle->ppvBuckets, hr, E_OUTOFMEMORY, "Failed to allocate buckets for dictionary");

LExit:
    return hr;
}

// The dict will store a set of keys, with no values associated with them. Use DictAddKey() and DictKeyExists() with this dictionary type.
extern "C" HRESULT DAPI DictCreateStringList(
    __out STRINGDICT_HANDLE* psdHandle,
    __in DWORD dwNumExpectedItems
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(psdHandle, hr, E_INVALIDARG, "Handle not specified while creating dict");

    // Allocate the handle
    *psdHandle = static_cast<STRINGDICT_HANDLE>(MemAlloc(sizeof(STRINGDICT_STRUCT), FALSE));
    ExitOnNull(*psdHandle, hr, E_OUTOFMEMORY, "Failed to allocate dictionary object");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(*psdHandle);

    // Fill out the new handle's values
    shHandle->dtType = DICT_STRING_LIST;
    shHandle->cByteOffset = 0;
    shHandle->dwBucketSizeIndex = 0;
    shHandle->dwNumItems = 0;
    shHandle->dwNumItemsInList = 0;
    shHandle->ppvItemList = NULL;

    // Make shHandle->dwBucketSizeIndex point to the appropriate spot in the prime
    // array based on expected number of items and items to buckets ratio
    // Careful: the "-1" in "countof(MAX_BUCKET_SIZES)-1" ensures we don't end
    // this loop past the end of the array!
    while (shHandle->dwBucketSizeIndex < (countof(MAX_BUCKET_SIZES)-1) &&
           MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex] < dwNumExpectedItems * MAX_ITEMS_TO_BUCKETS_RATIO)
    {
        shHandle->dwBucketSizeIndex++;
    }

    // Finally, allocate our initial buckets
    shHandle->ppvBuckets = static_cast<void**>(MemAlloc(sizeof(void *) * MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex], TRUE));
    ExitOnNull(shHandle->ppvBuckets, hr, E_OUTOFMEMORY, "Failed to allocate buckets for dictionary");

LExit:
    return hr;
}

// Todo: Dict should resize itself when (number of items) exceeds (number of buckets / MAX_ITEMS_TO_BUCKETS_RATIO)
extern "C" HRESULT DAPI DictAddKey(
    __in STRINGDICT_HANDLE sdHandle,
    __in_z LPCWSTR pszString
    )
{
    HRESULT hr = S_OK;
    DWORD dwIndex = 0;

    ExitOnNull(sdHandle, hr, E_INVALIDARG, "Handle not specified while adding value to dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while adding value to dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(sdHandle);

    if (DICT_STRING_LIST != shHandle->dtType)
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Tried to add key without value to wrong dictionary type! This dictionary type is: %d", shHandle->dtType);
    }

    hr = GetInsertIndex(shHandle, pszString, &dwIndex);
    ExitOnFailure(hr, "Failed to get index to insert into");

    shHandle->dwNumItems++;
    shHandle->dwNumItemsInList++;
    hr = MemEnsureArraySize(reinterpret_cast<void **>(&(shHandle->ppvItemList)), shHandle->dwNumItemsInList, sizeof(void *), 1000);
    ExitOnFailure(hr, "Failed to resize list of items in dictionary");

    hr = StrAllocString(reinterpret_cast<LPWSTR *>(&(shHandle->ppvBuckets[dwIndex])), pszString, 0);
    ExitOnFailure(hr, "Failed to allocate copy of string");

    shHandle->ppvItemList[shHandle->dwNumItems-1] = shHandle->ppvBuckets[dwIndex];

LExit:
    return hr;
}

// Todo: Dict should resize itself when (number of items) exceeds (number of buckets / MAX_ITEMS_TO_BUCKETS_RATIO)
extern "C" HRESULT DAPI DictAddValue(
    __in STRINGDICT_HANDLE sdHandle,
    __in_z LPCWSTR pszString,
    __in void *pvValue
    )
{
    HRESULT hr = S_OK;
    DWORD dwIndex = 0;

    ExitOnNull(sdHandle, hr, E_INVALIDARG, "Handle not specified while adding value to dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while adding value to dict");
    ExitOnNull(pvValue, hr, E_INVALIDARG, "Value not specified while adding value to dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(sdHandle);

    if (DICT_EMBEDDED_KEY != shHandle->dtType)
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Tried to add key/value pair to wrong dictionary type! This dictionary type is: %d", shHandle->dtType);
    }

    hr = GetInsertIndex(shHandle, pszString, &dwIndex);
    ExitOnFailure(hr, "Failed to get index to insert into");

    shHandle->dwNumItems++;
    shHandle->ppvBuckets[dwIndex] = pvValue;

LExit:
    return hr;
}

extern "C" HRESULT DAPI DictGetValue(
    __in STRINGDICT_HANDLE sdHandle,
    __in_z LPCWSTR pszString,
    __out void **ppvValue
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(sdHandle, hr, E_INVALIDARG, "Handle not specified while searching dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while searching dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(sdHandle);

    if (DICT_EMBEDDED_KEY != shHandle->dtType)
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Tried to lookup value in wrong dictionary type! This dictionary type is: %d", shHandle->dtType);
    }

    hr = GetValue(sdHandle, pszString, ppvValue);
    ExitOnFailure(hr, "Failed to call internal GetValue()");

LExit:
    return hr;
}

extern "C" HRESULT DAPI DictKeyExists(
    __in STRINGDICT_HANDLE sdHandle,
    __in_z LPCWSTR pszString
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(sdHandle, hr, E_INVALIDARG, "Handle not specified while searching dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while searching dict");

    // This works with either type of dictionary

    hr = GetValue(sdHandle, pszString, NULL);
    ExitOnFailure(hr, "Failed to call internal GetValue()");

LExit:
    return hr;
}

extern "C" void DAPI DictDestroy(
    __in STRINGDICT_HANDLE sdHandle
    )
{
    DWORD i;

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(sdHandle);

    if (DICT_STRING_LIST == shHandle->dtType)
    {
        for (i = 0; i < shHandle->dwNumItemsInList; i++)
        {
            ReleaseStr(reinterpret_cast<LPWSTR>(shHandle->ppvItemList[i]));
        }
    }

    ReleaseMem(shHandle->ppvItemList);
    ReleaseMem(shHandle->ppvBuckets);
    ReleaseMem(shHandle);
}

static DWORD StringHash(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString
    )
{
    DWORD result = 0;

    while (*pszString)
    {
        result = ~(*pszString++ * 509) + result * 65599;
    }

    return result % MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex];
}

static BOOL IsMatchExact(
    __in const void *pvHandle,
    __in DWORD dwMatchIndex,
    __in_z LPCWSTR pszOriginalString
    )
{
    const STRINGDICT_STRUCT *shHandle = static_cast<const STRINGDICT_STRUCT *>(pvHandle);

    const BYTE *lpByte = static_cast<const BYTE *>(shHandle->ppvBuckets[dwMatchIndex]);
    LPCWSTR pszMatchString = NULL;

    lpByte += shHandle->cByteOffset;
    pszMatchString = reinterpret_cast<LPCWSTR>(lpByte);

    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pszOriginalString, -1, pszMatchString, -1))
    {
        return TRUE;
    }

    return FALSE;
}

static HRESULT GetValue(
    __in void *pvHandle,
    __in_z LPCWSTR pszString,
    __out_opt void **ppvValue
    )
{
    HRESULT hr = S_OK;
    DWORD dwIndex = 0;

    ExitOnNull(pvHandle, hr, E_INVALIDARG, "Handle not specified while searching dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while searching dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(pvHandle);

    DWORD dwOriginalIndexCandidate = StringHash(shHandle, pszString);
    DWORD dwIndexCandidate = dwOriginalIndexCandidate;

    // If no match exists in the dict
    if (NULL == shHandle->ppvBuckets[dwIndexCandidate])
    {
        if (NULL != ppvValue)
        {
            *ppvValue = NULL;
        }
        ExitFunction1(hr = E_NOTFOUND);
    }

    hr = GetIndex(shHandle, pszString, &dwIndex);
    ExitOnFailure(hr, "Failed to find index to get");

    if (NULL != ppvValue)
    {
        *ppvValue = shHandle->ppvBuckets[dwIndexCandidate];
    }

LExit:
    if (FAILED(hr) && NULL != ppvValue)
    {
        *ppvValue = NULL;
    }

    return hr;
}

static HRESULT GetInsertIndex(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString,
    __out DWORD *pdwOutput
    )
{
    HRESULT hr = S_OK;
    DWORD dwOriginalIndexCandidate = StringHash(shHandle, pszString);
    DWORD dwIndexCandidate = dwOriginalIndexCandidate;

    // If we collide, keep iterating forward from our intended position, even wrapping around to zero, until we find an empty bucket
    while (NULL != shHandle->ppvBuckets[dwIndexCandidate])
    {
        dwIndexCandidate++;

        // If we got to the end of the array, wrap around to zero index
        if (dwIndexCandidate >= MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex])
        {
            dwIndexCandidate = 0;
        }

        // If we wrapped all the way back around to our original index, the dict is full - throw an error
        if (dwIndexCandidate == dwOriginalIndexCandidate)
        {
            // The dict table is full - this error seems to be a reasonably close match 
            hr = HRESULT_FROM_WIN32(ERROR_DATABASE_FULL);
            ExitOnFailure1(hr, "Failed to add item '%ls' to dict table because dict table is full of items", pszString);
        }
    }

    *pdwOutput = dwIndexCandidate;

LExit:
    return hr;
}

static HRESULT GetIndex(
    __in const STRINGDICT_STRUCT *shHandle,
    __in_z LPCWSTR pszString,
    __out DWORD *pdwOutput
    )
{
    HRESULT hr = S_OK;
    DWORD dwOriginalIndexCandidate = StringHash(shHandle, pszString);
    DWORD dwIndexCandidate = dwOriginalIndexCandidate;

    while (!IsMatchExact(shHandle, dwIndexCandidate, pszString))
    {
        dwIndexCandidate++;

        // If we got to the end of the array, wrap around to zero index
        if (dwIndexCandidate >= MAX_BUCKET_SIZES[shHandle->dwBucketSizeIndex])
        {
            dwIndexCandidate = 0;
        }

        // If no match exists in the dict
        if (NULL == shHandle->ppvBuckets[dwIndexCandidate])
        {
            ExitFunction1(hr = E_NOTFOUND);
        }

        // If we wrapped all the way back around to our original index, the dict is full and we found nothing, so return as such
        if (dwIndexCandidate == dwOriginalIndexCandidate)
        {
            ExitFunction1(hr = E_NOTFOUND);
        }
    }

    *pdwOutput = dwIndexCandidate;

LExit:
    return hr;
}
