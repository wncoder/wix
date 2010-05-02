@echo off
set CURRENTDIR=%~dp0

rem This file will run the Wix Tests

rem --- attempt to find MSTEST ---
rem Use MSTest from VS 2008

if %PROCESSOR_ARCHITECTURE% == AMD64 set MSTESTPFILES=%ProgramFiles(x86)%
if %PROCESSOR_ARCHITECTURE% == IA64 set MSTESTPFILES=%ProgramFiles(x86)%
if %PROCESSOR_ARCHITECTURE% == x86 set MSTESTPFILES=%ProgramFiles%

pushd %MSTESTPFILES%
if "%MSTEST%" == "" if exist "Microsoft Visual Studio 9.0\Common7\IDE\mstest.exe" set MSTEST=%MSTESTPFILES%\Microsoft Visual Studio 9.0\Common7\IDE\mstest.exe
popd

if "%MSTEST%" == "" goto :NoMSTest

rem --- MSTest.exe was found. Continue. ---

setlocal
set FLAVOR=debug
set VERBOSE=false

rem --- parse the arguments ---

:Parse_Args
if /i [%1]==[] goto :End_Parse_Args
if /i [%1]==[debug] set FLAVOR=%1& shift& goto :Parse_Args
if /i [%1]==[ship] set FLAVOR=%1& shift& goto :Parse_Args
if /i [%1]==[-all] set _T=%_T% /test:*& shift& goto :Parse_Args
if /i [%1]==[/all] set _T=%_T% /test:*& shift& goto :Parse_Args
if /i [%1]==[all] set _T=%_T% /test:*& shift& goto :Parse_Args
if /i [%1]==[-resultsfile] set RESULTSFILE=/resultsfile:%2 & shift& shift& goto :Parse_Args
if /i [%1]==[/resultsfile] set RESULTSFILE=/resultsfile:%2 & shift& shift& goto :Parse_Args
if /i [%1]==[-smoke] set TESTLIST=/testlist:SmokeTests& shift& goto :Parse_Args
if /i [%1]==[/smoke] set TESTLIST=/testlist:SmokeTests& shift& goto :Parse_Args
if /i [%1]==[smoke] set TESTLIST=/testlist:SmokeTests& shift& goto :Parse_Args
if /i [%1]==[-testlist] set TESTLIST=/testlist:%2& shift& shift& goto :Parse_Args
if /i [%1]==[/testlist] set TESTLIST=/testlist:%2& shift& shift& goto :Parse_Args
if /i [%1]==[-v] set VERBOSE=true & shift& goto :Parse_Args
if /i [%1]==[/v] set VERBOSE=true & shift& goto :Parse_Args
if /i [%1]==[-?] goto :Help
if /i [%1]==[/?] goto :Help
if /i [%1]==[-help] goto :Help
if /i [%1]==[/help] goto :Help
set _T=%_T% /test:%1& shift& goto :Parse_Args
rem echo.Invalid argument: '%1'
goto :Help
:End_Parse_Args

rem --- attempt to find the tests ---
set WIXTESTS=%CURRENTDIR%..\build\%FLAVOR%\x86\WixTests.dll
if not exist %WIXTESTS% goto :NoWixTests

rem --- run the tests ---

echo Using MSTest from %MSTEST%

set WixTestBinDirectory=%WIX_ROOT%\build\%FLAVOR%\x86
echo Running the tests against the Wix binaries in %WixTestBinDirectory%

call %WIX_ROOT%\test\settestenv.bat
echo Finished setting the test environment

if %VERBOSE% == true (set DETAIL=/detail:errormessage /detail:errorstacktrace /detail:stderr /detail:stdout)

echo on
call "%MSTEST%" /runconfig:localtestrun.testrunconfig /testmetadata:wixtests.vsmdi %TESTLIST% %_T% %DETAIL% %RESULTSFILE%
@echo off
goto :Cleanup

rem --- MSTest was not found ---

:NoMSTest
echo MSTest.exe version 9.0 could not be found. To run the tests, you must manually set the MSTEST environment variable to the full path of MSTest.exe.
goto :Cleanup

rem --- The test binaries were not found ---

:NoWixTests
echo The test binaries at %WIXTESTS% were not found. They must be built with the WiX binaries and require the Visual Studio Test Tools to be installed on your machine.
goto :Cleanup

:Help
echo.
echo test.bat - Runs tests against the WiX core toolset
echo.
echo Usage: test.bat [debug^|ship] [options] [test1 test2 ...]
echo.
echo Options:
echo   flavor               Sets the flavor to either debug (default) or ship
echo   -all                 Runs all of the tests
echo   -smoke               Runs the smoke tests
echo   -testlist ^<testlist^>  Run a specified test list
echo   -resultsfile ^<file^>   Specifies the location of the results file
echo   -v                   Displays verbose output
echo   -?                   Shows this help
echo.
goto :Cleanup

:Cleanup
endlocal