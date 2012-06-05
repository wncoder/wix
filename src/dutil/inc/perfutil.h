#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="perfutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Performance helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// structs


// functions
void DAPI PerfInitialize(
    );
void DAPI PerfClickTime(
    __out_opt LARGE_INTEGER* pliElapsed
    );
double DAPI PerfConvertToSeconds(
    __in const LARGE_INTEGER* pli
    );

#ifdef __cplusplus
}
#endif
