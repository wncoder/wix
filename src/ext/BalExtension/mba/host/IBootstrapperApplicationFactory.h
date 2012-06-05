//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperApplicationFactory.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Class interface for the managed BootstrapperApplicationFactory class.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "precomp.h"

DECLARE_INTERFACE_IID_(IBootstrapperApplicationFactory, IUnknown, "2965A12F-AC7B-43A0-85DF-E4B2168478A4")
{
    STDMETHOD(Create)(
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND *pCommand,
        __out IBootstrapperApplication **ppApplication
        );
};
