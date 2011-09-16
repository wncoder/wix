//-------------------------------------------------------------------------------------------------
// <copyright file="section.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// structs

typedef struct _BURN_SECTION
{
    DWORD cbStub;
    DWORD cbEngineSize; // stub + UX container + original certficiate

    DWORD dwChecksumOffset;
    DWORD dwCertificateTableOffset;
    DWORD dwOriginalChecksumAndSignatureOffset;

    DWORD dwOriginalChecksum;
    DWORD dwOriginalSignatureOffset;
    DWORD dwOriginalSignatureSize;

    DWORD dwFormat;
    DWORD cContainers;
    DWORD* rgcbContainers;
} BURN_SECTION;


HRESULT SectionInitialize(
    __in BURN_SECTION* pSection
    );
void SectionUninitialize(
    __in BURN_SECTION* pSection
    );
HRESULT SectionGetAttachedContainerInfo(
    __in BURN_SECTION* pSection,
    __in DWORD iContainerIndex,
    __in DWORD dwExpectedType,
    __out DWORD64* pqwOffset,
    __out DWORD64* pqwSize
    );

#if defined(__cplusplus)
}
#endif
