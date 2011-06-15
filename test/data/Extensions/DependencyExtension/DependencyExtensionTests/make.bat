@setlocal enableextensions

set PATH=%WIX_ROOT%\build\debug\x86;%PATH%

candle.exe A.wxs -out %~dp0Packages\A.wixobj -ext WixDependencyExtension
if "%errorlevel%" neq "0" exit /b 1

light.exe %~dp0Packages\A.wixobj -out %~dp0Packages\A.msi -ext WixDependencyExtension -sval
if "%errorlevel%" neq "0" exit /b 1

candle.exe B.wxs -out %~dp0Packages\B.wixobj -ext WixDependencyExtension
if "%errorlevel%" neq "0" exit /b 1

light.exe %~dp0Packages\B.wixobj -out %~dp0Packages\B.msi -ext WixDependencyExtension -sval
if "%errorlevel%" neq "0" exit /b 1

candle.exe BundleA.wxs -out %~dp0Packages\Bundles\BundleA.wixobj -ext WixDependencyExtension
if "%errorlevel%" neq "0" exit /b 1

light.exe %~dp0Packages\Bundles\BundleA.wixobj -out %~dp0Packages\Bundles\BundleA.exe -ext WixBalExtension -ext WixDependencyExtension -b packageA=%~dp0Packages\A.msi -sval
if "%errorlevel%" neq "0" exit /b 1

candle.exe BundleB.wxs -out %~dp0Packages\Bundles\BundleB.wixobj -ext WixDependencyExtension
if "%errorlevel%" neq "0" exit /b 1

light.exe %~dp0Packages\Bundles\BundleB.wixobj -out %~dp0Packages\Bundles\BundleB.exe -ext WixBalExtension -ext WixDependencyExtension -b packageA=%~dp0Packages\A.msi -b packageB=%~dp0Packages\B.msi -sval
if "%errorlevel%" neq "0" exit /b 1

dark.exe %~dp0Packages\Bundles\BundleA.exe -x %~dp0Packages\Bundles\BundleA\
if "%errorlevel%" neq "0" exit /b 1

dark.exe %~dp0Packages\Bundles\BundleB.exe -x %~dp0Packages\Bundles\BundleB\
if "%errorlevel%" neq "0" exit /b 1
