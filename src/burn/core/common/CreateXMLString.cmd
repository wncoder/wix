@if "%_echo%" == "" echo off
if "%1" == "" goto usage
if not exist %1 goto usage
if "%2" == "" goto usage

SET tmpfile=%temp%\%random%.tmp

:: convert to ascii file
call "%_NTDRIVE%%_NTROOT%\tools\DevDiv\BatchSetup\UnitoHex.exe" -u %1 %tmpfile%
:: Run awk to create the string
call "%_NTDRIVE%%_NTROOT%\tools\DevDiv\%_BuildArch%\awk.exe" -f "%~dp0EscapeBackSlashesAndQuotes.awk" %tmpfile% > "%2"
if errorlevel 1 goto usage
del %tmpfile%

goto :eof

:usage
echo.
echo          Converts the xml file into a string to be used in the unit tests
echo          in the EngineDataTests.cpp and UiDataTests.cpp
echo.
echo          %~nx0 ^<xml file^> ^<temp destination file^>
echo.
echo          example: %~nx0 UiInfo.xml tempfile.h
echo.
goto :eof

