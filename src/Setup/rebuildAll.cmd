setlocal
set _B=
set _S=

:Parse
if ""=="%1" goto Go
if /I "burn"=="%1" set _B=build
if /I "ship"=="%1" set _S=-D:flavor=ship
shift
goto Parse

:Go
if ""=="%_B%" goto SkipBurn

pushd ..\dutil
nant dutil.build %_S%
popd
pushd ..\burn
nant burn.build %_S%
popd

:SkipBurn
pushd UX
nant wixux.build %_S%
popd
pushd Bundle
nant bundle.build %_S%
popd
endlocal