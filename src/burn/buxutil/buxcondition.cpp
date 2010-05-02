//-------------------------------------------------------------------------------------------------
// <copyright file="buxcondition.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// Burn UX utility condition utility.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes


DAPI_(HRESULT) BuxConditionsParseFromXml(
    __out BUX_CONDITIONS* pConditions,
    __in IXMLDOMDocument* pixdManifest
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* pNode = NULL;
    BUX_CONDITION* prgConditions = NULL;
    DWORD cConditions = 0;

    hr = XmlSelectNodes(pixdManifest, L"/UxManifest/WixBuxCondition", &pNodeList);
    ExitOnFailure(hr, "Failed to select all conditions.");

    hr = pNodeList->get_length(reinterpret_cast<long*>(&cConditions));
    ExitOnFailure(hr, "Failed to get the condition count.");

    prgConditions = static_cast<BUX_CONDITION*>(MemAlloc(sizeof(BUX_CONDITION) * cConditions, TRUE));
    ExitOnNull(prgConditions, hr, E_OUTOFMEMORY, "Failed to allocate memory for payloads.");

    DWORD iCondition = 0;
    while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, NULL)))
    {
        hr = XmlGetAttributeEx(pNode, L"Condition", &prgConditions[iCondition].sczCondition);
        ExitOnFailure(hr, "Failed to get message for condition.");

        hr = XmlGetAttributeEx(pNode, L"Message", &prgConditions[iCondition].sczMessage);
        ExitOnFailure(hr, "Failed to get message for condition.");

        ++iCondition;
        ReleaseNullObject(pNode);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    pConditions->cConditions = cConditions;
    pConditions->rgConditions = prgConditions;
    prgConditions = NULL;

LExit:
    ReleaseMem(prgConditions);
    ReleaseObject(pNode);
    ReleaseObject(pNodeList);
    return hr;
}


DAPI_(HRESULT) BuxConditionEvaluate(
    __in BUX_CONDITION* pCondition,
    __in IBurnCore* pCore,
    __out BOOL* pfResult,
    __out_z_opt LPWSTR* psczMessage
    )
{
    HRESULT hr = S_OK;
    DWORD_PTR cchMessage = 0;

    hr = pCore->EvaluateCondition(pCondition->sczCondition, pfResult);
    ExitOnFailure(hr, "Failed to evaluate condition with bootstrapper engine.");

    if (psczMessage)
    {
        if (*psczMessage)
        {
            hr = StrMaxLength(*psczMessage, &cchMessage);
            ExitOnFailure(hr, "Failed to get length of message.");
        }

        hr = pCore->FormatString(pCondition->sczMessage, *psczMessage, reinterpret_cast<DWORD*>(&cchMessage));
        if (E_MOREDATA == hr)
        {
            ++cchMessage;

            hr = StrAlloc(psczMessage, cchMessage);
            ExitOnFailure(hr, "Failed to allocate string for condition's formatted message.");

            hr = pCore->FormatString(pCondition->sczMessage, *psczMessage, reinterpret_cast<DWORD*>(&cchMessage));
        }
        ExitOnFailure(hr, "Failed to format condition's message.");
    }

LExit:
    return hr;
}


DAPI_(void) BuxConditionsUninitialize(
    __in BUX_CONDITIONS* pConditions
    )
{
    for (DWORD i = 0; i < pConditions->cConditions; ++i)
    {
        ReleaseStr(pConditions->rgConditions[i].sczMessage);
        ReleaseStr(pConditions->rgConditions[i].sczCondition);
    }

    ReleaseMem(pConditions->rgConditions);
    memset(pConditions, 0, sizeof(BUX_CONDITIONS));
}
