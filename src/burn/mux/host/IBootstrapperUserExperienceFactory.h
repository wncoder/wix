//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperUserExperienceFactory.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Class interface for the managed UserExperienceFactory class.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "precomp.h"

DECLARE_INTERFACE_IID_(IBootstrapperUserExperienceFactory, IUnknown, "2965A12F-AC7B-43A0-85DF-E4B2168478A4")
{
    STDMETHOD(Create)(
        __deref_in const BURN_COMMAND *pCommand,
        __out IBurnUserExperience **ppUX
        );
};
