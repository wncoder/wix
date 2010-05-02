pushd ..\dutil
nant dutil.build
popd
nant burn.build
