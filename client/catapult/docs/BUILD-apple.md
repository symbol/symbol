# Building on Apple (macOS)

This guide describes how to build catapult on macOS.

## Prerequisites

- `git`
- `cmake` >= 3.23
- `python3` >= 3.8
- `conan` >= 2.x
- `ninja`
- Xcode command line tools

Install required tools:

```sh
xcode-select --install
brew install git cmake ninja pkg-config python
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
export DYLD_LIBRARY_PATH=$PWD/deps
```

## Step 6: Verify

```sh
bin/catapult.tools.address --help
```
