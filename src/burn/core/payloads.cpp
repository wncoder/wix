//-------------------------------------------------------------------------------------------------
// <copyright file="payloads.cpp" company="Microsoft">
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
//    Payload management functions for Burn bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define BURN_SECTION_NAME ".wixburn"
#define BURN_CONTAINER_MAGIC 0x00f14300
#define MANIFEST_CABINET_TOKEN L"0"

static HRESULT GetEmbeddedContainerHeader(
    __in_ecount(*pcbBuffer) BYTE* pbBuffer,
    __inout DWORD_PTR* pcbBuffer,
    __out DWORD* pdwVersion,
    __out DWORD* pdwOffset,
    __out DWORD* pdwSize,
    __out LPWSTR *psczBundleId,
    __out DWORD* pdwAttachedContainerOffset,
    __out DWORD* pdwAttachedContainerSize
    )
{
    HRESULT hr = S_OK;
    IMAGE_DOS_HEADER* pDosHeader = NULL;
    IMAGE_NT_HEADERS* pNtHeader = NULL;
    IMAGE_SECTION_HEADER* rgSections = NULL;
    DWORD cSections = 0;
    DWORD dwContainerSectionIndex = MAXDWORD;

    // validate arguments
    Assert(pbBuffer);
    Assert(pcbBuffer);
    Assert(pdwVersion);
    Assert(pdwOffset);
    Assert(pdwSize);
    Assert(pdwAttachedContainerOffset);
    Assert(pdwAttachedContainerSize);

    // First, make sure we have a valid DOS signature.
    if (*pcbBuffer < sizeof(IMAGE_DOS_HEADER))
    {
        *pcbBuffer = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS);

        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(pbBuffer);
    if (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find valid DOS image header in buffer.");
    }

    // Now, make sure we have a valid NT signature.
    if (*pcbBuffer < static_cast<DWORD>(pDosHeader->e_lfanew))
    {
        *pcbBuffer = pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);

        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pbBuffer + pDosHeader->e_lfanew);
    if (IMAGE_NT_SIGNATURE != pNtHeader->Signature )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find valid NT image header in buffer.");
    }

    // Finally, get into the section table and look for the Burn container header.
    DWORD dwNeeded;
    DWORD dwMultResult;
    hr = ::ULongAdd(pDosHeader->e_lfanew, sizeof(DWORD/*IMAGE_NT_HEADERS.Signature*/), &dwNeeded);
    ExitOnFailure(hr, "Failed to add lfanew to size of DWORD for needed byte count");

    hr = ::ULongAdd(dwNeeded, sizeof(IMAGE_FILE_HEADER), &dwNeeded);
    ExitOnFailure(hr, "Failed to add size of IMAGE_FILE_HEADER to needed byte count");

    hr = ::ULongAdd(dwNeeded, pNtHeader->FileHeader.SizeOfOptionalHeader, &dwNeeded);
    ExitOnFailure(hr, "Failed to add size of optional header to needed byte count");

    hr = ::ULongMult(pNtHeader->FileHeader.NumberOfSections, sizeof(IMAGE_SECTION_HEADER), &dwMultResult);
    ExitOnFailure(hr, "Failed to multiply number of sections by size of image section header");

    hr = ::ULongAdd(dwNeeded, dwMultResult, &dwNeeded);
    ExitOnFailure(hr, "Failed on final addition for needed byte count");

    if (*pcbBuffer < dwNeeded)
    {
        *pcbBuffer = dwNeeded;
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    rgSections = reinterpret_cast<IMAGE_SECTION_HEADER*>(reinterpret_cast<BYTE*>(pNtHeader) + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + pNtHeader->FileHeader.SizeOfOptionalHeader);
    cSections = pNtHeader->FileHeader.NumberOfSections;
    for (dwContainerSectionIndex = 0; dwContainerSectionIndex < cSections; ++dwContainerSectionIndex)
    {
        if (0 == memcmp(rgSections[dwContainerSectionIndex].Name, BURN_SECTION_NAME, sizeof(rgSections[dwContainerSectionIndex].Name)))
        {
            break;
        }
    }

    if (cSections <= dwContainerSectionIndex)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find Burn container header in buffer.");
    }

    hr = ::ULongAdd(rgSections[dwContainerSectionIndex].PointerToRawData, rgSections[dwContainerSectionIndex].SizeOfRawData, &dwNeeded);
    ExitOnFailure(hr, "Failed to add pointer to add while determining bytes needed");
    if (*pcbBuffer < dwNeeded)
    {
        *pcbBuffer = dwNeeded;
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    // we've arrived at the container header
    const DWORD* pBurnContainerHeader = reinterpret_cast<const DWORD*>(pbBuffer + rgSections[dwContainerSectionIndex].PointerToRawData);
    if (BURN_CONTAINER_MAGIC != *pBurnContainerHeader)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure1(hr, "Failed to find valid Burn container header signature, found: 0x%x", *pBurnContainerHeader);
    }

    *pdwVersion = *++pBurnContainerHeader;
    *pdwOffset = *++pBurnContainerHeader;
    *pdwSize = *++pBurnContainerHeader;
    *pdwAttachedContainerOffset = *++pBurnContainerHeader;
    *pdwAttachedContainerSize = *++pBurnContainerHeader;

    // These match the defaults defined in StubSection.cpp
    // If they're still set at these values, then the values must have been left alone, which means we don't have an attached container
    if (MAXDWORD == *pdwOffset && MAXDWORD == *pdwSize && MAXDWORD == *pdwAttachedContainerOffset && MAXDWORD == *pdwAttachedContainerSize)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    const GUID* pBundleGuidFromHeader = reinterpret_cast<GUID*>(pbBuffer + rgSections[dwContainerSectionIndex].PointerToRawData + 4*sizeof(DWORD));

    WCHAR wzBundleGuidFromHeader[39] = { };
    if (0 == ::StringFromGUID2(*pBundleGuidFromHeader, wzBundleGuidFromHeader, countof(wzBundleGuidFromHeader)))
    {
        ExitOnFailure(hr = E_INVALIDARG, "Failed to convert bundle GUID to a string.");
    }

    hr = StrAllocString(psczBundleId, wzBundleGuidFromHeader, 0);
    ExitOnFailure(hr, "Failed to copy bundle id.");

LExit:
    return hr;
}

HRESULT PayloadGetManifest(
    __in_z_opt LPCWSTR wzSourcePath,
    __out LPBYTE* pbManifest,
    __out DWORD* pcbManifest,
    __out_z LPWSTR* psczBundleId,
    __out DWORD* pdwContainerOffset,
    __out DWORD* pdwAttachedContainerOffset
    )
{
    HRESULT hr = S_OK;
    DWORD dwVersion = 0;
    DWORD dwOffset = 0;
    DWORD dwSize = 0;
    DWORD dwAttachedContainerOffset = 0;
    DWORD dwAttachedContainerSize = 0;
    BYTE* pbData = NULL;
    LPWSTR sczFileName = NULL;
    LPWSTR sczBundleId = NULL;
    LPWSTR sczTempFolder = NULL;
    LPWSTR sczTempManifest = NULL;

    // validate arguments
    Assert(pbManifest);
    Assert(pcbManifest);
    Assert(psczBundleId);
    Assert(pdwContainerOffset);
    Assert(pdwAttachedContainerOffset);

    if (!wzSourcePath)
    {
        hr = PathForCurrentProcess(&sczFileName, NULL);
        ExitOnFailure(hr, "Failed to get path for current process.");
    }

    // keep reading data and passing it to GetEmbeddedManifestHeader until
    // we've read enough to get the manifest header section.
    *pcbManifest = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS);
    for (;;)
    {
        ReleaseNullMem(pbData);
        hr = FileReadPartial(&pbData, pcbManifest, wzSourcePath ? wzSourcePath : sczFileName, FALSE, 0, *pcbManifest, TRUE);
        ExitOnFailure(hr, "Failed to read header data.");

        hr = GetEmbeddedContainerHeader(pbData, pcbManifest, &dwVersion, &dwOffset, &dwSize, &sczBundleId, &dwAttachedContainerOffset, &dwAttachedContainerSize);
        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
        {
            continue;
        }
        else if (SUCCEEDED(hr))
        {
            break;
        }
        else
        {
            ExitOnFailure(hr, "Failed to locate manifest header.");
        }
    }

    if (0x1 != dwVersion)
    {
        ExitOnFailure1(hr = E_INVALIDARG, "Unknown manifest version: %d", dwVersion);
    }

    // now extract the manifest (always payload '0')
    ReleaseNullMem(pbData);

    // TODO: Extract from the cabinet into a memory blob rather than to a file and then to a blob.
    hr = PathCreateTempDirectory(NULL, L"BURN%03x", 999, &sczTempFolder);
    ExitOnFailure(hr, "Failed to create temporary directory for extracted manifest.");

    hr = CabExtract(wzSourcePath ? wzSourcePath : sczFileName, MANIFEST_CABINET_TOKEN, sczTempFolder, NULL, NULL, dwOffset);
    ExitOnFailure(hr, "Failed to extract manifest.");

    hr = PathConcat(sczTempFolder, MANIFEST_CABINET_TOKEN, &sczTempManifest);
    ExitOnFailure(hr, "Failed to create temporary .");

    hr = FileRead(&pbData, pcbManifest, sczTempManifest);
    ExitOnFailure(hr, "Failed to read manifest.");

    *psczBundleId = sczBundleId;
    sczBundleId = NULL;
    *pdwContainerOffset = dwOffset;
    *pbManifest = pbData;
    pbData = NULL;
    *pdwAttachedContainerOffset = dwAttachedContainerOffset;

LExit:
    if (sczTempFolder)
    {
        DirEnsureDelete(sczTempFolder, TRUE, TRUE); //ignore failures
        ReleaseStr(sczTempFolder);
    }

    ReleaseMem(pbData);
    ReleaseStr(sczTempManifest);
    ReleaseStr(sczBundleId);
    ReleaseStr(sczFileName);

    return hr;
}
