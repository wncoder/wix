//-------------------------------------------------------------------------------------------------
// <copyright file="buxcondition.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// Burn UX utility condition utility.
// </summary>
//-------------------------------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BUX_CONDITION
{
    LPWSTR sczCondition;
    LPWSTR sczMessage;
} BUX_CONDITION;


typedef struct _BUX_CONDITIONS
{
    BUX_CONDITION* rgConditions;
    DWORD cConditions;
} BUX_CONDITIONS;


/*******************************************************************
 BuxConditionsParseFromXml - loads the conditions from the UX manifest.

********************************************************************/
DAPI_(HRESULT) BuxConditionsParseFromXml(
    __in BUX_CONDITIONS* pConditions,
    __in IXMLDOMDocument* pixdManifest
    );


/*******************************************************************
 BuxConditionEvaluate - evaluates condition against the provided IBurnCore.

 NOTE: psczMessage is optional.
********************************************************************/
DAPI_(HRESULT) BuxConditionEvaluate(
    __in BUX_CONDITION* pCondition,
    __in IBurnCore* pCore,
    __out BOOL* pfResult,
    __out_z_opt LPWSTR* psczMessage
    );


/*******************************************************************
 BuxConditionsUninitialize - uninitializes any conditions previously loaded.

********************************************************************/
DAPI_(void) BuxConditionsUninitialize(
    __in BUX_CONDITIONS* pConditions
    );


#ifdef __cplusplus
}
#endif
