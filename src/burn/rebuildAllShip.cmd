pushd ..\dutil
nant -D:flavor=ship dutil.build
popd
nant -D:flavor=ship burn.build
