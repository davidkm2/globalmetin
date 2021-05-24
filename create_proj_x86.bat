@echo off

if [%1]==[] goto usage

rmdir /s /q _vsproj

mkdir _vsproj

pushd _vsproj

cmake .. -A Win32 -DCMAKE_BUILD_TYPE=%1 -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF

popd

@echo Project make done!
goto :eof

:usage
@echo Usage: %0 ^<build_type^>
PAUSE
exit /B 1