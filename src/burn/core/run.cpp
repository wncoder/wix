//-------------------------------------------------------------------------------------------------
// <copyright file="run.cpp" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#include <bits.h>
#include <winhttp.h>

#include "fakeatl.h"
#include "IronMan.h"
#include "..\common\IronManAssert.h"

// Declare the static members of ModuleUtils used by self-extraction
CPath IronMan::ModuleUtils::m_pthParameterFileFolder;
CPath IronMan::ModuleUtils::m_pthUxFolder;
CPath IronMan::ModuleUtils::m_pthIncludedPayloadsFolder;
bool IronMan::ModuleUtils::m_fHasEmbeddedContent = false;
bool IronMan::ModuleUtils::m_fHasFoldersSet = false;
CSimpleArray<CString> IronMan::ModuleUtils::m_AttachedPayloadLocalNames;
// Acceptable certificates for verifying payloads
IronMan::AcceptableCertificates IronMan::Configuration::m_acceptableCertificates;

// function definitions

extern "C" HRESULT Run(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    vpEngineState = pEngineState;
    return IronMan::Main().Run();
}
