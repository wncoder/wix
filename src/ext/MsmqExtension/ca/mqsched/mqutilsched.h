#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="mqutilsched.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    MSMQ Custom Action utility functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


HRESULT PcaGuidToRegFormat(
    LPWSTR pwzGuid,
    LPWSTR pwzDest,
    SIZE_T cchDest
    );
