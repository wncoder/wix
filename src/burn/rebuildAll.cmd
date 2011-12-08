setlocal
pushd ..\dutil
if /I "ship"=="%1" set _S=-D:flavor=ship
nant dutil.build %_S%
popd
nant burn.build %_S%
endlocal