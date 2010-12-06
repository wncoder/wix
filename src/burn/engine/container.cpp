//-------------------------------------------------------------------------------------------------
// <copyright file="container.cpp" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// constants

#define BURN_SECTION_NAME ".wixburn"
#define BURN_CONTAINER_MAGIC 0x00f14300
#define MANIFEST_CABINET_TOKEN L"0"


// structs

typedef struct _BURN_CONTAINER_HEADER
{
    DWORD dwMagic;
    DWORD dwVersion;
    DWORD cContainerInfo;
    DWORD dwFormat;
    struct {
        DWORD64 qwOffset;
        DWORD64 qwSize;
    } rgContainerInfo[1];
} BURN_CONTAINER_HEADER;


// internal function declarations

static HRESULT GetAttachedContainerInfo(
    __in HANDLE hFile,
    __in DWORD iContainerIndex,
    __out DWORD* pdwFormat,
    __out DWORD64* pqwOffset,
    __out DWORD64* pqwSize
    );


// function definitions

extern "C" HRESULT ContainersParseFromXml(
    __in BURN_CONTAINERS* pContainers,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR scz = NULL;

    // select container nodes
    hr = XmlSelectNodes(pixnBundle, L"Container", &pixnNodes);
    ExitOnFailure(hr, "Failed to select container nodes.");

    // get container node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get container node count.");

    if (!cNodes)
    {
        ExitFunction();
    }

    // allocate memory for searches
    pContainers->rgContainers = (BURN_CONTAINER*)MemAlloc(sizeof(BURN_CONTAINER) * cNodes, TRUE);
    ExitOnNull(pContainers->rgContainers, hr, E_OUTOFMEMORY, "Failed to allocate memory for container structs.");

    pContainers->cContainers = cNodes;

    // parse search elements
    for (DWORD i = 0; i < cNodes; ++i)
    {
        BURN_CONTAINER* pContainer = &pContainers->rgContainers[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
        ExitOnFailure(hr, "Failed to get next node.");

        // TODO: Read type from manifest. Today only CABINET is supported.
        pContainer->type = BURN_CONTAINER_TYPE_CABINET;

        // @Id
        hr = XmlGetAttributeEx(pixnNode, L"Id", &pContainer->sczId);
        ExitOnFailure(hr, "Failed to get @Id.");

        // @Primary
        hr = XmlGetYesNoAttribute(pixnNode, L"Primary", &pContainer->fPrimary);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Primary.");
        }

        // @Attached
        hr = XmlGetYesNoAttribute(pixnNode, L"Attached", &pContainer->fAttached);
        if (E_NOTFOUND != hr || pContainer->fPrimary) // if it is a primary container, it has to be attached
        {
            ExitOnFailure(hr, "Failed to get @Attached.");
        }

        // @AttachedIndex
        hr = XmlGetAttributeNumber(pixnNode, L"AttachedIndex", &pContainer->dwAttachedIndex);
        if (E_NOTFOUND != hr || pContainer->fAttached) // if it is an attached container it must have an index
        {
            ExitOnFailure(hr, "Failed to get @AttachedIndex.");
        }

        // @SourcePath
        hr = XmlGetAttributeEx(pixnNode, L"SourcePath", &pContainer->sczSourcePath);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @SourcePath.");
        }

        // @DownloadUrl
        hr = XmlGetAttributeEx(pixnNode, L"DownloadUrl", &pContainer->downloadSource.sczUrl);
        if (E_NOTFOUND != hr || (!pContainer->fPrimary && !pContainer->sczSourcePath)) // if the package is not a primary package, it must have a source path or a download url
        {
            ExitOnFailure(hr, "Failed to get @DownloadUrl. Either @SourcePath or @DownloadUrl needs to be provided.");
        }

        // @FileSize
        hr = XmlGetAttributeEx(pixnNode, L"FileSize", &scz);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @FileSize.");

            hr = StrStringToUInt64(scz, 0, &pContainer->qwFileSize);
            ExitOnFailure(hr, "Failed to parse @FileSize.");
        }

        // @Hash
        hr = XmlGetAttributeEx(pixnNode, L"Hash", &pContainer->sczHash);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Hash.");
        }

        // prepare next iteration
        ReleaseNullObject(pixnNode);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

    return hr;
}

extern "C" void ContainersUninitialize(
    __in BURN_CONTAINERS* pContainers
    )
{
    if (pContainers->rgContainers)
    {
        for (DWORD i = 0; i < pContainers->cContainers; ++i)
        {
            BURN_CONTAINER* pContainer = &pContainers->rgContainers[i];

            ReleaseStr(pContainer->sczId);
            ReleaseStr(pContainer->sczHash);
            ReleaseStr(pContainer->sczSourcePath);
            ReleaseStr(pContainer->downloadSource.sczUrl);
            ReleaseStr(pContainer->downloadSource.sczUser);
            ReleaseStr(pContainer->downloadSource.sczPassword);
        }
        MemFree(pContainers->rgContainers);
    }

    // clear struct
    memset(pContainers, 0, sizeof(BURN_CONTAINERS));
}

extern "C" HRESULT ContainerOpenUX(
    __in BURN_CONTAINER_CONTEXT* pContext
    )
{
    HRESULT hr = S_OK;
    BURN_CONTAINER container = { };

    // open attached container
    container.type = BURN_CONTAINER_TYPE_CABINET;
    container.fPrimary = TRUE;
    container.fAttached = TRUE;
    container.dwAttachedIndex = 0;

    hr = ContainerOpen(pContext, &container, NULL);
    ExitOnFailure(hr, "Failed to open attached container.");

LExit:
    return hr;
}

extern "C" HRESULT ContainerOpen(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in BURN_CONTAINER* pContainer,
    __in_opt LPCWSTR wzFilePath
    )
{
    HRESULT hr = S_OK;
    DWORD dwFormat = 0;
    LPWSTR sczExecutablePath = NULL;
    LARGE_INTEGER li = { };

    // initialize context
    pContext->type = pContainer->type;

    // for a primary container, get the executable module path
    if (pContainer->fPrimary)
    {
        hr = PathForCurrentProcess(&sczExecutablePath, NULL);
        ExitOnFailure(hr, "Failed to get path for executing module.");

        wzFilePath = sczExecutablePath;
    }
    else if (!wzFilePath)
    {
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Must provide a path for a non-primary container.");
    }

    // open container file
    pContext->hFile = ::CreateFileW(wzFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    ExitOnInvalidHandleWithLastError1(pContext->hFile, hr, "Failed to open file: %ls", wzFilePath);

    // if it is a container attached to an executable, read container header
    if (pContainer->fAttached)
    {
        hr = GetAttachedContainerInfo(pContext->hFile, pContainer->dwAttachedIndex, &dwFormat, &pContext->qwOffset, &pContext->qwSize);
        ExitOnFailure(hr, "Failed to get container info.");

        // verify format
        if (dwFormat != (DWORD)pContainer->type)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnFailure(hr, "Invalid container format.");
        }

        // seek to container offset
        li.QuadPart = (LONGLONG)pContext->qwOffset;
        if (!::SetFilePointerEx(pContext->hFile, li, NULL, FILE_BEGIN))
        {
            ExitWithLastError(hr, "Failed to move file pointer to container offset.");
        }
    }
    else
    {
        hr = FileSizeByHandle(pContext->hFile, (LONGLONG*)&pContext->qwSize);
        ExitOnFailure1(hr, "Failed to check size of file %ls by handle", wzFilePath);
    }

    // open the archive
    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractOpen(pContext, wzFilePath);
        break;
    }
    ExitOnFailure(hr, "Failed to open container.");

LExit:
    ReleaseStr(sczExecutablePath);

    return hr;
}

extern "C" HRESULT ContainerNextStream(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __inout_z LPWSTR* psczStreamName
    )
{
    HRESULT hr = S_OK;

    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractNextStream(pContext, psczStreamName);
        break;
    }

//LExit:
    return hr;
}

extern "C" HRESULT ContainerStreamToFile(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in_z LPCWSTR wzFileName
    )
{
    HRESULT hr = S_OK;

    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractStreamToFile(pContext, wzFileName);
        break;
    }

//LExit:
    return hr;
}

extern "C" HRESULT ContainerStreamToBuffer(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __out BYTE** ppbBuffer,
    __out SIZE_T* pcbBuffer
    )
{
    HRESULT hr = S_OK;

    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractStreamToBuffer(pContext, ppbBuffer, pcbBuffer);
        break;
    }

//LExit:
    return hr;
}

extern "C" HRESULT ContainerSkipStream(
    __in BURN_CONTAINER_CONTEXT* pContext
    )
{
    HRESULT hr = S_OK;

    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractSkipStream(pContext);
        break;
    }

//LExit:
    return hr;
}

extern "C" HRESULT ContainerClose(
    __in BURN_CONTAINER_CONTEXT* pContext
    )
{
    HRESULT hr = S_OK;

    // close container
    switch (pContext->type)
    {
    case BURN_CONTAINER_TYPE_CABINET:
        hr = CabExtractClose(pContext);
        ExitOnFailure(hr, "Failed to close cabinet.");
        break;
    }

LExit:
    ReleaseFile(pContext->hFile);

    if (SUCCEEDED(hr))
    {
        memset(pContext, 0, sizeof(BURN_CONTAINER_CONTEXT));
    }

    return hr;
}

extern "C" HRESULT ContainerFindById(
    __in BURN_CONTAINERS* pContainers,
    __in_z LPCWSTR wzId,
    __out BURN_CONTAINER** ppContainer
    )
{
    HRESULT hr = S_OK;
    BURN_CONTAINER* pContainer = NULL;

    for (DWORD i = 0; i < pContainers->cContainers; ++i)
    {
        pContainer = &pContainers->rgContainers[i];

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pContainer->sczId, -1, wzId, -1))
        {
            *ppContainer = pContainer;
            ExitFunction1(hr = S_OK);
        }
    }

    hr = E_NOTFOUND;

LExit:
    return hr;
}


// internal function definitions

static HRESULT GetAttachedContainerInfo(
    __in HANDLE hFile,
    __in DWORD iContainerIndex,
    __out DWORD* pdwFormat,
    __out DWORD64* pqwOffset,
    __out DWORD64* pqwSize
    )
{
    HRESULT hr = S_OK;
    DWORD cbRead = 0;
    LARGE_INTEGER li = { };
    IMAGE_DOS_HEADER dosHeader = { };
    IMAGE_NT_HEADERS ntHeader = { };
    IMAGE_SECTION_HEADER sectionHeader = { };
    BURN_CONTAINER_HEADER* pContainerHeader = NULL;

    //
    // First, make sure we have a valid DOS signature.
    //

    // read DOS header
    if (!::ReadFile(hFile, &dosHeader, sizeof(IMAGE_DOS_HEADER), &cbRead, NULL))
    {
        ExitWithLastError(hr, "Failed to read DOS header.");
    }
    if (sizeof(IMAGE_DOS_HEADER) > cbRead || IMAGE_DOS_SIGNATURE != dosHeader.e_magic)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find valid DOS image header in buffer.");
    }

    //
    // Now, make sure we have a valid NT signature.
    //

    // seek to new header
    li.QuadPart = dosHeader.e_lfanew;
    if (!::SetFilePointerEx(hFile, li, NULL, FILE_BEGIN))
    {
        ExitWithLastError(hr, "Failed to seek to NT header.");
    }

    // read NT header
    if (!::ReadFile(hFile, &ntHeader, sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER), &cbRead, NULL))
    {
        ExitWithLastError(hr, "Failed to read NT header.");
    }
    if ((sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER)) > cbRead || IMAGE_NT_SIGNATURE != ntHeader.Signature)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find valid NT image header in buffer.");
    }

    //
    // Finally, get into the section table and look for the Burn container header.
    //

    // seek past optional headers
    li.QuadPart = ntHeader.FileHeader.SizeOfOptionalHeader;
    if (!::SetFilePointerEx(hFile, li, NULL, FILE_CURRENT))
    {
        ExitWithLastError(hr, "Failed to seek past optional headers.");
    }

    // read sections one by one until we find our section
    for (DWORD i = 0; ; ++i)
    {
        // read section
        if (!::ReadFile(hFile, &sectionHeader, sizeof(IMAGE_SECTION_HEADER), &cbRead, NULL))
        {
            ExitWithLastError1(hr, "Failed to read image section header, index: %u", i);
        }
        if (sizeof(IMAGE_SECTION_HEADER) > cbRead)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnFailure1(hr, "Failed to read complete image section header, index: %u", i);
        }

        // compare header name
        C_ASSERT(sizeof(sectionHeader.Name) == sizeof(BURN_SECTION_NAME) - 1);
        if (0 == memcmp(sectionHeader.Name, BURN_SECTION_NAME, sizeof(sectionHeader.Name)))
        {
            break;
        }

        // fail if we hit the end
        if (i + 1 >= ntHeader.FileHeader.NumberOfSections)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnFailure(hr, "Failed to find Burn section.");
        }
    }

    //
    // We've arrived at the container header.
    //

    // check size of section
    if (sizeof(BURN_CONTAINER_HEADER) > sectionHeader.SizeOfRawData)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure1(hr, "Failed to read container header, data to short: %u", sectionHeader.SizeOfRawData);
    }

    // allocate buffer for container header
    pContainerHeader = (BURN_CONTAINER_HEADER*)MemAlloc(sectionHeader.SizeOfRawData, TRUE);
    ExitOnNull(pContainerHeader, hr, E_OUTOFMEMORY, "Failed to allocate buffer for container header.");

    // seek to container header
    li.QuadPart = sectionHeader.PointerToRawData;
    if (!::SetFilePointerEx(hFile, li, NULL, FILE_BEGIN))
    {
        ExitWithLastError(hr, "Failed to seek to container header.");
    }

    // read container header
    if (!::ReadFile(hFile, pContainerHeader, sectionHeader.SizeOfRawData, &cbRead, NULL))
    {
        ExitWithLastError(hr, "Failed to read container header.");
    }
    if (sectionHeader.SizeOfRawData > cbRead)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to read complete container header.");
    }

    // validate version of container header
    if (0x00000001 != pContainerHeader->dwVersion)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure1(hr, "Failed to read container header, unsupported version: %08x", pContainerHeader->dwVersion);
    }

    // validate container info
    if (iContainerIndex >= pContainerHeader->cContainerInfo)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure1(hr, "Failed to find container info, too few elements: %u", pContainerHeader->cContainerInfo);
    }

    // get container format
    *pdwFormat = pContainerHeader->dwFormat;

    // get container offset and size
    *pqwOffset = pContainerHeader->rgContainerInfo[iContainerIndex].qwOffset;
    *pqwSize = pContainerHeader->rgContainerInfo[iContainerIndex].qwSize;

LExit:
    ReleaseMem(pContainerHeader);

    return hr;
}
