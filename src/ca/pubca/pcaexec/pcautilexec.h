//-------------------------------------------------------------------------------------------------
// <copyright file="pcautilexec.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Public Custom Action utility functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


HRESULT PcaActionDataMessage(
	DWORD cArgs,
	...
	);
HRESULT PcaAccountNameToSid(
	LPCWSTR pwzAccountName,
	PSID* ppSid
	);
HRESULT PcaSidToAccountName(
	PSID pSid,
	LPWSTR* ppwzAccountName
	);
HRESULT PcaBuildAccountName(
	LPCWSTR pwzDomain,
	LPCWSTR pwzName,
	LPWSTR* ppwzAccount
	);
HRESULT PcaGuidFromString(
	LPCWSTR pwzGuid,
	GUID* pGuid
	);
