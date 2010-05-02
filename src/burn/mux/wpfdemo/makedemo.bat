@if "%_echo%" == "" echo off
@setlocal

if /i "%1" == "ship" (set BurnFlavour=ship) else set BurnFlavour=debug
set IMLGK=%WIX_ROOT%\build\%BurnFlavour%\x86
set Demo=%WIX_ROOT%\src\burn\demo
set WpfDemo=%WIX_ROOT%\src\burn\mux\wpfdemo

set Binaries=burnstub.exe mux.dll muxhost.dll muxmodel.dll wpfview.dll
for %%B in (%Binaries%) do copy /Y "%IMLGK%\%%B" %WpfDemo%

copy /Y %Demo%\some_local.msi %WpfDemo%\some.msi

set Pdbs=burnstub.pdb mux.pdb muxhost.pdb muxmodel.dll wpfview.pdb
for %%P in (%Pdbs%) do copy /Y "%IMLGK%\%%P" %WpfDemo%
