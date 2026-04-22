[CMake]: http://cmake.org
[Conan]: https://conan.io

# Building with Conan

[Conan] is a C/C++ package manager. In this project, it is used to fetch and build third-party dependencies and make them available to CMake in a reproducible way.

- [Building on Linux/macOS](#building-conan-nix)
    - [Prerequisites](#nix-prerequisites)
- [Building on Windows](#building-with-conan-on-windows)

These instructions cover building catapult with Conan on Linux/macOS and Windows.

## Building with Conan on Linux/macOS {#building-conan-nix}

### Prerequisites {#nix-prerequisites}

- Python >= 3.8
- Conan >= 2.x

## Install Conan 2 with pip (Python 3 required)

Conan 2 is installed with `pip`, which requires Python 3 to be available in your environment.

```sh
python3 -m pip install --upgrade "conan>=2,<3"
conan --version
```

On Windows, if `python3` is not available, use:

```sh
py -3 -m pip install --upgrade "conan>=2,<3"
conan --version
```

## Common setup (all platforms)

```sh
conan remote add nemtech https://conan.symbol.dev/artifactory/api/conan/catapult
git clone https://github.com/symbol/symbol.git
cd symbol/client/catapult
```

## Building with Conan on *nix (Linux/macOS)

### Prerequisites

- **Linux (Ubuntu):**

  ```sh
  sudo apt update
  sudo apt install build-essential git cmake ninja-build pkg-config python3-full
  ```

- **macOS:**

  ```sh
  xcode-select --install
  brew install git cmake ninja pkg-config
  ```

- Install Conan: <https://conan.io/downloads.html>
- Create Conan profile:

  ```sh
  conan profile detect --name default
  ```

### Install dependencies

```sh
conan install . --build=missing -s build_type=Release
cd build/Release
```

`build_type` can be `Release`, `RelWithDebInfo`, or `Debug`.

### Configure and build

```sh
cmake --preset conan-release -G Ninja -DUSE_CONAN=ON ../../
ninja -j4
```

### Runtime library path

After build, binaries are in `build/bin`. Dependencies are in `build/deps` and must be visible at runtime.

- Linux:

  ```sh
  export LD_LIBRARY_PATH=$PWD/deps
  ```

- macOS:

  ```sh
  export DYLD_LIBRARY_PATH=$PWD/deps
  ```

### Optional install

```sh
sudo ninja install
```

You can customize the install location with `-DCMAKE_INSTALL_PREFIX=...` during CMake configure.

### Verify

```sh
bin/catapult.tools.address --help
```

## Building with Conan on Windows

### Prerequisites

1. Install [Visual Studio](https://visualstudio.microsoft.com/) and [Git for Windows](https://git-scm.com/download/win).
2. Run commands from a Visual Studio Native Tools Command Prompt.
3. Install Conan with pip (Python 3):

   ```sh
   py -3 -m pip install --upgrade "conan>=2,<3"
   ```

   Or use the installer from <https://conan.io/downloads.html>.

4. Create Conan profile:

   ```sh
   conan profile detect --name default
   ```

### Install dependencies

```sh
conan install . --build=missing -s compiler.cppstd=17 -s build_type=Release
cd build
```

`build_type` can be `Release`, `RelWithDebInfo`, or `Debug`.

### Configure and build

> **Note:** set `PYTHON_EXECUTABLE` to a Python 3 path (for generated headers). You can locate it with `where python3`.

```sh
cmake --preset conan-default -G "Visual Studio 17 2022" -A x64 -DUSE_CONAN=ON -DPYTHON_EXECUTABLE:FILEPATH=X:/python3x/python.exe ..
cmake --build . --target publish
msbuild /p:Configuration=Release /p:Platform=x64 /m ALL_BUILD.vcxproj
```

Ensure `Configuration` matches the Conan `build_type` used above.

After a successful build, tools are available under `build\bin\<configuration>`.

### Verify

```sh
bin\Release\catapult.tools.address --help
```
