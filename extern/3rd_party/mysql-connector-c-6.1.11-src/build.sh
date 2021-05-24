mkdir build
if [ ! $? -eq 0 ]; then
	echo "Build folder does exist"
fi
cd build

#CXX=clang++
#CC=clang

CXX=g++5
CC=gcc5

export CXX
export CC

cmake .. -DCMAKE_BUILD_TYPE=$1 -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF
if [ ! $? -eq 0 ]; then
	echo "Cmake was not successfull"
	exit 1
fi
gmake -j8
if [ ! $? -eq 0 ]; then
	echo "Could not fully build"
	exit 2
fi

echo "build done!"