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

struct STRINGDICT_STRUCT
{
    // Index into MAX_BUCKET_SIZES (array of primes), representing number of buckets we've allocated
    DWORD dwBucketSizeIndex;

    // Number of items currently stored in the dict array
    DWORD dwNumItems;

    // Byte offset of key within bucket value, for collision checking - see
    // comments above DictCreate() implementation for further details
    size_t cByteOffset;

    // The actual stored buckets
    void **ppvBuckets;
};

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

// The dict will store a set of keys (as wide-char strings) and a set of values associated with those keys (as void *'s).
// However, to support collision checking, the key needs to be represented in the "value" object (pointed to
// by the void *). The "stByteOffset" parameter tells this dict the byte offset of the "key" string pointer
// within the "value" object. Use the offsetof() macro to fill this out.
extern "C" HRESULT DAPI DictCreate(
    __out STRINGDICT_HANDLE* ppvHandle,
    __in DWORD dwNumExpectedItems,
    __in size_t cByteOffset
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(ppvHandle, hr, E_INVALIDARG, "Handle not specified while creating dict");

    // Allocate the handle
    *ppvHandle = static_cast<STRINGDICT_HANDLE>(MemAlloc(sizeof(STRINGDICT_STRUCT), FALSE));
    ExitOnNull(*ppvHandle, hr, E_OUTOFMEMORY, "Failed to allocate dictionary object");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(*ppvHandle);

    // Fill out the new handle's values
    shHandle->cByteOffset = cByteOffset;
    shHandle->dwBucketSizeIndex = 0;
    shHandle->dwNumItems = 0;

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
extern "C" HRESULT DAPI DictAdd(
    __in void *pvHandle,
    __in_z LPCWSTR pszString,
    __in void *pvValue
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(pvHandle, hr, E_INVALIDARG, "Handle not specified while adding value to dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while adding value to dict");
    ExitOnNull(pvValue, hr, E_INVALIDARG, "Value not specified while adding value to dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(pvHandle);
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

    shHandle->ppvBuckets[dwIndexCandidate] = pvValue;
    shHandle->dwNumItems++;

LExit:
    return hr;
}

extern "C" HRESULT DAPI DictGet(
    __in void *pvHandle,
    __in_z LPCWSTR pszString,
    __out void **ppvValue
    )
{
    HRESULT hr = S_OK;

    ExitOnNull(pvHandle, hr, E_INVALIDARG, "Handle not specified while searching dict");
    ExitOnNull(pszString, hr, E_INVALIDARG, "String not specified while searching dict");
    ExitOnNull(ppvValue, hr, E_INVALIDARG, "Value pointer not specified while searching dict");

    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(pvHandle);
    DWORD dwOriginalIndexCandidate = StringHash(shHandle, pszString);
    DWORD dwIndexCandidate = dwOriginalIndexCandidate;

    // If no match exists in the dict
    if (NULL == shHandle->ppvBuckets[dwIndexCandidate])
    {
        *ppvValue = NULL;
        ExitFunction1(hr = S_FALSE);
    }

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
            *ppvValue = NULL;
            ExitFunction1(hr = S_FALSE);
        }

        // If we wrapped all the way back around to our original index, the dict is full and we found nothing, so return as such
        if (dwIndexCandidate == dwOriginalIndexCandidate)
        {
            *ppvValue = NULL;
            ExitFunction1(hr = S_FALSE);
        }
    }

    // If we got here, we found a match!
    *ppvValue = shHandle->ppvBuckets[dwIndexCandidate];

LExit:
    return hr;
}

extern "C" void DAPI DictDestroy(
    __in void *pvHandle
    )
{
    STRINGDICT_STRUCT *shHandle = static_cast<STRINGDICT_STRUCT *>(pvHandle);

    ReleaseMem(shHandle->ppvBuckets);
    ReleaseMem(shHandle);
}

