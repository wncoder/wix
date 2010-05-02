#include "precomp.h"

#include "uxcore.h"

HRESULT __stdcall GetVariableNumeric(
        __in_z LPCWSTR wzVariable,
        __out LONGLONG* pllValue
        )
{
	return CChainerUXCore::gBurnCore->GetVariableNumeric(wzVariable, pllValue);
}

HRESULT __stdcall GetVariableString(
        __in_z LPCWSTR wzVariable,
        __out_ecount_opt(*pcchValue) LPWSTR wzValue,
        __inout DWORD* pcchValue
        )
{
	return CChainerUXCore::gBurnCore->GetVariableString(wzVariable, wzValue, pcchValue);
}

HRESULT __stdcall GetVariableVersion(
        __in_z LPCWSTR wzVariable,
        __out DWORD64* pqwValue
        )
{
	return CChainerUXCore::gBurnCore->GetVariableVersion(wzVariable, pqwValue);

}

HRESULT __stdcall SetVariableNumeric(
        __in_z LPCWSTR wzVariable,
        __in LONGLONG llValue
        )
{
	return CChainerUXCore::gBurnCore->SetVariableNumeric(wzVariable, llValue);

}

HRESULT __stdcall SetVariableString(
        __in_z LPCWSTR wzVariable,
        __in_z LPCWSTR wzValue
        )
{
	return CChainerUXCore::gBurnCore->SetVariableString(wzVariable, wzValue);

}

HRESULT __stdcall SetVariableVersion(
        __in_z LPCWSTR wzVariable,
        __in DWORD64 qwValue
        )
{
	return CChainerUXCore::gBurnCore->SetVariableVersion(wzVariable, qwValue);
}

HRESULT __stdcall FormatString(
        __in_z LPCWSTR wzIn,
        __out_ecount_opt(*pcchOut) LPWSTR wzOut,
        __inout DWORD* pcchOut
        )
{
	return CChainerUXCore::gBurnCore->FormatString(wzIn, wzOut, pcchOut);

}

HRESULT __stdcall EscapeString(
        __in_z LPCWSTR wzIn,
        __out_ecount_opt(*pcchOut) LPWSTR wzOut,
        __inout DWORD* pcchOut
        )
{
	return CChainerUXCore::gBurnCore->EscapeString(wzIn, wzOut, pcchOut);
	
}

HRESULT __stdcall EvaluateCondition(
        __in_z LPCWSTR wzCondition,
        __out BOOL* pf
        )
{
	return CChainerUXCore::gBurnCore->EvaluateCondition(wzCondition, pf);

}

HRESULT __stdcall Log(
        __in BURN_LOG_LEVEL level,
        __in_z LPCWSTR wzMessage
        )
{
	return CChainerUXCore::gBurnCore->Log(level, wzMessage);
}

HRESULT __stdcall Elevate(
        __in_opt HWND hwndParent
        )
{
	return CChainerUXCore::gBurnCore->Elevate(hwndParent);
}

HRESULT __stdcall Detect()
{
	return CChainerUXCore::gBurnCore->Detect();

}

HRESULT __stdcall Plan(
        __in BURN_ACTION action
        )
{
	return CChainerUXCore::gBurnCore->Plan(action);

}

HRESULT __stdcall Apply(
        __in_opt HWND hwndParent
        )
{
	return CChainerUXCore::gBurnCore->Apply(hwndParent);

}

HRESULT __stdcall Suspend()
{
	return CChainerUXCore::gBurnCore->Suspend();

}

HRESULT __stdcall Reboot()
{
	return CChainerUXCore::gBurnCore->Reboot();

}

HRESULT __stdcall SetSource(
        __in    LPCWSTR wzSourcePath
        )
{
	return CChainerUXCore::gBurnCore->SetSource(wzSourcePath);


}