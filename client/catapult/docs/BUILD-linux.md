[CMake]: https://cmake.org/download/
[vcpkg]: https://github.com/microsoft/vcpkg
[Conan]: https://conan.io/downloads.html
[Ninja]: https://ninja-build.org/
[Python]: https://www.python.org/downloads/
[Git]: https://git-scm.com/download/linux

# Building on Linux

This guide describes how to build catapult on Linux.
There are two supported build techniques on Linux:

- [CMake with vcpkg](#building-with-vcpkg) (easiest and recommended)
- [CMake with Conan](#building-with-conan) (used by CI; useful if you already use Conan)

Both techniques drive the same CMake build and use [Ninja] as the generator.

- [Building with vcpkg](#building-with-vcpkg)
  - [Prerequisites](#prerequisites)
  - [Setting up the environment](#setting-up-the-environment)
  - [Configure and build](#configure-and-build)
  - [Verify Catapult build](#verify-catapult-build)
- [Building with Conan](#building-with-conan)
  - [Prerequisites](#prerequisites-1)
  - [Setting up the environment](#setting-up-the-environment-1)
  - [Let Conan install dependencies](#let-conan-install-dependencies)
  - [Configure and build](#configure-and-build-1)
  - [Verify Catapult build](#verify-catapult-build-1)

## Building with vcpkg

### Prerequisites

A supported C++ toolchain:

- GCC >= 8 (GCC >= 11 strongly recommended), **or**
- Clang >= 14

Build tools:

- [CMake] >= 3.25
- [Ninja]
- [Git] >= 2.25
- [Python] 3.x (used to generate version headers and publish the SDK)
- [vcpkg] package manager (installed separately, see below)
- At least 60 GB of free disk space for the build and dependencies (this is an estimate and can vary)

On Debian/Ubuntu the build tools and the packages vcpkg needs to bootstrap and build dependencies from source can be installed with:

```sh
sudo apt update
sudo apt install -y build-essential git cmake ninja-build pkg-config python3 \
    curl zip unzip tar
```

> [!NOTE]
> If your distribution ships a CMake older than 3.25, install a newer CMake from the [CMake] downloads page (or your distribution's backports) before continuing.

### Setting up the environment

Install [vcpkg] from a Git clone (you can skip this step if you already have a vcpkg installation, but remember to set `VCPKG_ROOT` as described below):

```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

After bootstrap completes, point `VCPKG_ROOT` at the directory where you cloned vcpkg. Add it to your shell profile so it persists across sessions:

```sh
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

Change `$HOME/vcpkg` to the actual path where you cloned vcpkg. The vcpkg CMake presets are gated on `VCPKG_ROOT` being set, so configuration fails fast if it is missing.

Clone the catapult repository:

```sh
git clone https://github.com/symbol/symbol.git
cd symbol/client/catapult
```

### Configure and build

From `symbol/client/catapult`, configure and build in a single step with a workflow preset:

```sh
cmake --workflow --preset Release
```

This configures the project (resolving and building dependencies through vcpkg on the first run, which can take a while) and then builds it. The available presets can be listed with `cmake --list-presets`:

```
  "Debug"          - x64 Debug
  "Release"        - x64 Release
  "RelWithDebInfo" - x64 RelWithDebInfo
  "MinSizeRel"     - x64 MinSizeRel
```

If you prefer to run the steps separately:

```sh
cmake --preset Release         # configure
cmake --build --preset Release # build
```

### Verify Catapult build

Binaries are generated under `build/<preset>/bin` (for example `build/Release/bin`). Verify the build from `symbol/client/catapult`:

```sh
./build/Release/bin/catapult.tools.address --help
```

You should see the Address Inspector Tool help, listing its options (`--help`, `--network`, `--input`, `--output`, `--format`, `--mode`, ...). The version line includes the compiler tag, for example `catapult version: 1.0.3.7 <hash> [Gcc]`.

If you built the `Debug` preset, the unit tests are also built and can be run with [ctest]:

```sh
ctest --test-dir ./build/Debug
```

> [!NOTE]
> Do not enable parallel test execution; some tests rely on exclusive resources and may fail when run concurrently.

## Building with Conan

[Conan] is supported as an alternative to vcpkg and is the package manager used by the project's CI. It fetches and builds the third-party dependencies and exposes them to CMake through a generated preset named `conan-<build-type>`.

### Prerequisites

- The same C++ toolchain and build tools as above (GCC >= 8 / Clang >= 14, [CMake] >= 3.25, [Ninja], [Git], `pkg-config`), **except vcpkg is not needed**.
- [Python] >= 3.8
- [Conan] >= 2.0

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install -y build-essential git cmake ninja-build pkg-config python3 python3-pip
python3 -m pip install --upgrade "conan>=2,<3"
```

### Setting up the environment

Detect a default Conan profile and add the remote that hosts catapult's prebuilt dependencies:

```sh
conan profile detect --name default --force
conan remote add nemtech https://conan.symbol.dev/artifactory/api/conan/catapult
```

Clone the catapult repository:

```sh
git clone https://github.com/symbol/symbol.git
cd symbol/client/catapult
```

### Let Conan install dependencies

From `symbol/client/catapult`:

```sh
conan install . --build=missing -s compiler.cppstd=17 -s build_type=Release
```

`build_type` can be `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel`. Conan downloads or builds the dependencies and generates the CMake preset; this can take a while on the first run.

### Configure and build

```sh
cmake --preset conan-release
cmake --build --preset conan-release
```

The preset name is `conan-<build-type>` in lowercase (for example `conan-debug` for a `Debug` install).

### Verify Catapult build

Binaries are generated under `build/<build-type>/bin` (for example `build/Release/bin`), and the shared dependency libraries are placed in the matching `deps` directory. Make those libraries visible at runtime, then run a tool:

```sh
export LD_LIBRARY_PATH="$PWD/build/Release/deps"
./build/Release/bin/catapult.tools.address --help
```

You should see the Address Inspector Tool help. If you built a `Debug` configuration, run the unit tests with:

```sh
ctest --test-dir ./build/Debug
```

[ctest]: https://cmake.org/cmake/help/latest/manual/ctest.1.html
