
/// <summary>
/// Invokes a managed custom action from native code by
/// extracting the package to a temporary working directory
/// then hosting the CLR and locating and calling the entrypoint.
/// </summary>
/// <param name="hSession">Handle to the installation session.
/// Passed to custom action entrypoints by the installer engine.</param>
/// <param name="szWorkingDir">Directory containing the CA binaries
/// and the CustomAction.config file defining the entrypoints.
/// This may be NULL, in which case the current module must have
/// a concatenated cabinet containing those files, which will be
/// extracted to a temporary directory.</param>
/// <param name="szEntryPoint">Name of the CA entrypoint to be invoked,
/// in the form "AssemblyName!Namespace.Class.Method"</param>
/// <returns>The value returned by the managed custom action method,
/// or ERROR_INSTALL_FAILURE if the CA could not be invoked.</returns>
int InvokeCustomAction(MSIHANDLE hSession,
	const wchar_t* szWorkingDir, const wchar_t* szEntryPoint);

/// <summary>
/// Macro for defining and exporting a custom action entrypoint.
/// </summary>
/// <param name="name">Name of the entrypoint as exported from
/// the DLL.</param>
/// <param name="method">Path to the managed custom action method,
/// in the form: "AssemblyName!Namespace.Class.Method"</param>
/// <remarks>
/// To prevent the exported name from being decorated, add
/// /EXPORT:name to the linker options for every entrypoint.
/// </remarks>
#define CUSTOMACTION_ENTRYPOINT(name,method) extern "C" int __stdcall \
	name(MSIHANDLE hSession) { return InvokeCustomAction(hSession, NULL, method); }

// TEMPLATE ENTRYPOINTS
// To be edited by the MakeSfxCA tool.

#define NULLSPACE \
L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

#define TEMPLATE_CA_ENTRYPOINT(id,sid) CUSTOMACTION_ENTRYPOINT( \
	CustomActionEntryPoint##id##, \
	L"CustomActionEntryPoint" sid NULLSPACE)

TEMPLATE_CA_ENTRYPOINT(000,L"000");
TEMPLATE_CA_ENTRYPOINT(001,L"001");
TEMPLATE_CA_ENTRYPOINT(002,L"002");
TEMPLATE_CA_ENTRYPOINT(003,L"003");
TEMPLATE_CA_ENTRYPOINT(004,L"004");
TEMPLATE_CA_ENTRYPOINT(005,L"005");
TEMPLATE_CA_ENTRYPOINT(006,L"006");
TEMPLATE_CA_ENTRYPOINT(007,L"007");
TEMPLATE_CA_ENTRYPOINT(008,L"008");
TEMPLATE_CA_ENTRYPOINT(009,L"009");
TEMPLATE_CA_ENTRYPOINT(010,L"010");
TEMPLATE_CA_ENTRYPOINT(011,L"011");
TEMPLATE_CA_ENTRYPOINT(012,L"012");
TEMPLATE_CA_ENTRYPOINT(013,L"013");
TEMPLATE_CA_ENTRYPOINT(014,L"014");
TEMPLATE_CA_ENTRYPOINT(015,L"015");
// Note: Keep in sync with EntryPoints.def
