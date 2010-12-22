//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperApplicationFactory.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
