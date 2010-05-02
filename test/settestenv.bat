@echo off
rem Sets up the environment for a Wix developer to run tests

rem Set the default FLAVOR. If you want to test 'ship', then set FLAVOR=ship before running this script
if "%FLAVOR%" == "" set FLAVOR=debug

rem Set the default tasks, targets and MSBuild location if it's not already set
if "%WixTargetsPath%" == "" set WixTargetsPath=%WIX_ROOT%\build\%FLAVOR%\x86\wix.targets
if "%WixTasksPath%" == "" set WixTasksPath=%WIX_ROOT%\build\%FLAVOR%\x86\WixTasks.dll
if "%WixTestMSBuildDirectory%" == "" set WixTestMSBuildDirectory=%SystemRoot%\Microsoft.NET\Framework\v3.5
