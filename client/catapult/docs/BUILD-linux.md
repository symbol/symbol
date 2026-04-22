# Building on Linux

This guide describes how to build catapult on Linux.

## Prerequisites

- `git`
- `cmake` >= 3.23
- `python3` >= 3.8
- `conan` >= 2.x
- `ninja`
- C/C++ build tools (`gcc`/`g++` or `clang`)

Example (Ubuntu):

```sh
sudo apt update
sudo apt install -y build-essential git cmake ninja-build pkg-config python3-full python3-pip
python3 -m pip install --upgrade "conan>=2,<3"
```

## Step 1: Clone the repository

```sh
git clone https://github.com/symbol/symbol.git
cd symbol/client/catapult
```

## Step 2: Configure Conan

```sh
conan remote add nemtech https://conan.symbol.dev/artifactory/api/conan/catapult
conan profile detect --name default
```

## Step 3: Install dependencies

```sh
conan install . --build=missing -s build_type=Release
cd build/Release
```

`build_type` can be `Release`, `RelWithDebInfo`, or `Debug`.

## Step 4: Configure and build

```sh
cmake --preset conan-release -G Ninja -DUSE_CONAN=ON ../../
ninja -j4
```

## Step 5: Runtime library path

```sh
export LD_LIBRARY_PATH=$PWD/deps
```

## Step 6: Verify

```sh
bin/catapult.tools.address --help
```

## Optional: Manual dependency build

For a full source dependency build flow, see [Build manually](BUILD-manual.md).
