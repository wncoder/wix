//-------------------------------------------------------------------------------------------------
// <copyright file="section.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
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
    DWORD cbEngineSize;     // stub + UX container + original certficiate
    DWORD64 qwBundleSize;   // stub + UX container + original certificate [+ attached containers* + final certificate]

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
    __out DWORD64* pqwSize,
    __out BOOL* pfPresent
    );

#if defined(__cplusplus)
}
#endif
