NOTE: In both linux and windows commands we're using `\` as marker for line continuations

Building on Ubuntu 18.04 (LTS)
===

Prerequisites
---

```sh
sudo apt install autoconf libtool cmake curl git xz-utils \
    libatomic-ops-dev libunwind-dev g++ gdb libgflags-dev \
    libsnappy-dev ninja-build python3 python3-ply
```

Boost
---

```sh
curl -o boost_1_64_0.tar.gz -SL \
    https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
tar -xzf boost_1_64_0.tar.gz

## WARNING: below use $HOME rather than ~ - boost scripts might treat it literally
mkdir boost-build-1.64.0
cd boost_1_64_0
./bootstrap.sh --prefix=${HOME}/boost-build-1.64.0
./b2 --prefix=${HOME}/boost-build-1.64.0 -j 4 stage release
./b2 install --prefix=${HOME}/boost-build-1.64.0
```

Gtest
---

```sh
git clone https://github.com/google/googletest.git googletest.git
cd googletest.git
git checkout release-1.8.0

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
make
sudo make install
```

Google benchmark
---

```sh
git clone https://github.com/google/benchmark.git google.benchmark.git
cd google.benchmark.git
git checkout v1.4.1

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_GTEST_TESTS=OFF ..
make
sudo make install
```

Mongo
---

mongo-c

```sh
git clone https://github.com/mongodb/mongo-c-driver.git mongo-c-driver.git
cd mongo-c-driver.git
git checkout 1.13.0

mkdir _build && cd _build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

mongocxx
```sh
git clone https://github.com/mongodb/mongo-cxx-driver.git mongo-cxx-driver.git
cd mongo-cxx-driver.git
git checkout r3.4.0

### apply a patch...
sed -i 's/kvp("maxAwaitTimeMS", count)/kvp("maxAwaitTimeMS", static_cast<int64_t>(count))/' \
    src/mongocxx/options/change_stream.cpp

mkdir _build && cd _build
cmake -DLIBBSON_DIR=/usr/local -DBOOST_ROOT=~/boost-build-1.64.0 \
    -DLIBMONGOC_DIR=/usr/local -DBSONCXX_POLY_USE_BOOST=1 \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

ZMQ
---

NOTE: we're currently NOT coupled to any specific ZMQ version, this might change in future

libzmq
```sh
git clone git://github.com/zeromq/libzmq.git libzmq.git
cd libzmq.git
# on master already

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

cppzmq
```sh
git clone https://github.com/zeromq/cppzmq.git cppzmq.git
cd cppzmq.git
# on master already

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

Rocks
---

Currently ubuntu 18.04 has gflags in version 2.2.1 and snappy in version 1.1.7 which are OK

NOTE: we're currently NOT coupled to any specific rocksdb version, this might change in future

rocks
```sh
git clone https://github.com/facebook/rocksdb.git rocksdb.git
cd rocksdb.git

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

CATAPULT
---

```sh
git clone https://github.com/nemtech/catapult-server.git
cd catapult-server

mkdir _build && cd _build
cmake -DBOOST_ROOT=~/boost-build-1.64.0 -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja publish
ninja -j4
```

Building on Windows
===

Prerequisites
---

 * cmake
 * git
 * python 3.x
 * Visual Studio Community 2017

NOTE: building instructions should be executed from `VS x64 native command prompt`

NOTE: following instructions use X:\devlibs as a destination location for libraries

Boost
---

```bat
bootstrap.bat
set path=X:\devlibs\2017\boost.bin\boost\bin;%path%
b2 install --prefix="X:\devlibs\2017\boost.bin"
b2 address-model=64 --build-dir="X:\devlibs\2017\boost.build" toolset=msvc \
    --build-type=complete --stagedir="X:\devlibs\2017\boost.bin" \
    stage release --without-python
```

Gtest
---

```bat
git clone https://github.com/google/googletest.git googletest.git
cd googletest.git
git checkout release-1.8.1

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" -Dgtest_force_shared_crt=ON \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\googletest ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

Google benchmark
---

```bat
git clone https://github.com/google/benchmark.git google.benchmark.git
cd google.benchmark.git
git checkout v1.4.1

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" \
    -DBENCHMARK_ENABLE_GTEST_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\googlebench ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```


Mongo
---

mongo-c

```bat
git clone https://github.com/mongodb/mongo-c-driver.git mongo-c-driver.git
cd mongo-c-driver.git
git checkout 1.13.0

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" -DENABLE_EXTRA_ALIGNMENT=OFF \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\libmongoc ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

mongocxx
```bat
git clone https://github.com/mongodb/mongo-cxx-driver.git mongo-cxx-driver.git
cd mongo-cxx-driver.git
git checkout r3.4.0

# CMAKE_PREFIX_PATH - required for finding mongo-c
mkdir _build && cd _build
cmake.exe -G "Visual Studio 15 2017 Win64" \
    -DCMAKE_PREFIX_PATH=X:\devlibs\2017\libmongoc \
    -DBOOST_ROOT=X:\devlibs\2017\boost.bin \
    -DBSONCXX_POLY_USE_BOOST=1 \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\libmongocxx ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

ZMQ
---

NOTE: we're currently NOT coupled to any specific ZMQ version, this might change in future

libzmq
```bat
git clone git://github.com/zeromq/libzmq.git libzmq.git
cd libzmq.git
# on master already

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\libzmq ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

NOTE: mind that the zeroMq dir has `CMake` suffix

cppzmq
```bat
git clone https://github.com/zeromq/cppzmq.git cppzmq.git
cd cppzmq.git
# on master already

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" \
    -DZeroMQ_DIR=X:\devlibs\2017\libzmq\CMake \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\cppzmq ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

Rocks
---

NOTE: we're currently NOT coupled to any specific rocksdb version, this might change in future
NOTE: on windows master usually does not build, so we're using FB branches

gflags
```bat
wget https://github.com/gflags/gflags/archive/v2.2.2.zip
unzip v2.2.2.zip
cd gflags-2.2.2

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\gflags ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

snappy
```bat
git clone https://github.com/google/snappy.git snappy.git
cd snappy.git
git checkout 1.1.7

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\snappy
    -DBUILD_SHARED_LIBS=ON ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

rocks
```bat
git clone https://github.com/facebook/rocksdb.git rocksdb.git
cd rocksdb.git
git checkout -B "5.18.fb" "origin/5.18.fb">
```

edit `thirdparty.inc`
```cmake
set(GFLAGS_HOME X:/devlibs/2017/gflags)
set(GFLAGS_INCLUDE ${GFLAGS_HOME}/Include)
set(GFLAGS_LIB_DEBUG ${GFLAGS_HOME}/does_not_exist.lib)
set(GFLAGS_LIB_RELEASE ${GFLAGS_HOME}/Lib/gflags_static.lib)
...
...
set(SNAPPY_HOME X:/devlibs/2017/snappy)
set(SNAPPY_INCLUDE ${SNAPPY_HOME}/include)
set(SNAPPY_LIB_DEBUG ${SNAPPY_HOME}/lib/does_not_exist.lib)
set(SNAPPY_LIB_RELEASE ${SNAPPY_HOME}/lib/snappy.lib)
```

```bat
set path=X:\PATH-TO-GIT-DIRECTORY\bin;%path%
mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" \
    -DROCKSDB_INSTALL_ON_WINDOWS=ON -DGFLAGS=1 -DWITH_SNAPPY=1 -DWITH_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=X:\devlibs\2017\rocksdb ..
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 INSTALL.vcxproj
```

CATAPULT
---

full cmake:
```bat
git clone https://github.com/nemtech/catapult-server.git catapult-server.git
cd catapult-server.git

mkdir _build && cd _build
cmake -G "Visual Studio 15 2017 Win64" \
    -DBOOST_ROOT=X:\devlibs\2017\boost.bin \
    -DGTEST_ROOT=X:\devlibs\2017\googletest \
    -Dbenchmark_DIR=X:\devlibs\2017\googlebench\lib\cmake\benchmark \
    -DCMAKE_PREFIX_PATH=X:\devlibs\2017\libmongocxx;X:\devlibs\2017\libmongoc \
    -DZeroMQ_DIR=X:\devlibs\2017\libzmq\CMake \
    -Dcppzmq_DIR=X:\devlibs\2017\cppzmq\share\cmake\cppzmq \
    -DRocksDB_DIR=X:\devlibs\2017\rocksdb\lib\cmake\rocksdb \
    -DPYTHON_EXECUTABLE:FILEPATH=X:/python36/python.exe \
    ..

REM run publish task (required to build tools)
cmake --build . --target publish

REM open catapult_server.sln OR build from command line:
msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 ALL_BUILD.vcxproj
```
