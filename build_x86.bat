mkdir build

pushd build

cmake .. -A Win32 -DCMAKE_BUILD_TYPE=%1 -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF

cmake --build . --config %1 --target ALL_BUILD -- /m /property:GenerateFullPaths=true

popd

echo "build done!"