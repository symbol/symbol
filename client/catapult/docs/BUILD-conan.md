# Building with Conan

Following instructions should work on Mac, Linux (Ubuntu 20.04) and Windows.

## Prerequisites

* **On Linux**:

  1. Install the compiler and build dependencies:

     ```sh
     sudo apt update
     sudo apt install build-essential git cmake ninja-build pkg-config
     ```

  2. Install [Conan](https://conan.io/downloads.html).

  3. Set the right C++ ABI for Conan:

     ```sh
     conan profile new default --detect
     conan config set general.revisions_enabled=True
     ```
     
    For gcc compiler:
    ```sh
    conan profile update settings.compiler.libcxx=libstdc++11 default
    ```

    For clang compiler:
    ```sh
    conan profile update settings.compiler.libcxx=libc++ default
    ```

* **On Windows**:

  1. Install [Visual Studio](https://visualstudio.microsoft.com/) and [Git for Windows](https://git-scm.com/download/win).

     Run all commands from a command prompt that has access to Visual Studio and Git. This can be accomplished by using the "Native Tools Command Prompt" shortcut installed by Visual Studio on the Start Menu.

  2. Install [Conan](https://conan.io/downloads.html).

* **On Mac**:

  1. Install the compiler:

     ```sh
     xcode-select —install
     ```

  2. Install build dependencies:

     ```sh
     brew install git cmake ninja pkg-config
     ```

  3. Install [Conan](https://conan.io/downloads.html).

  4. Set the right config for Conan:

      ```sh
      conan profile new default --detect
      conan config set general.revisions_enabled=True
      ```

  5. Modify `conanfile.txt` (Only for Mac M1):
      - Change `boost:without_math=True`.
      - Add `rocksdb:enable_sse = False` under `rocksdb:shared = True`.

## Step 1: Build dependencies

While Conan will be building and installing packages, you might want to go for a ☕ (or lunch),
as this will probably take *a bit*.

```sh
conan remote add nemtech https://catapult.jfrog.io/artifactory/api/conan/symbol-conan

git clone https://github.com/symbol/symbol.git
cd symbol/client/catapult

mkdir _build && cd _build
CONAN_REVISIONS_ENABLED=1 conan install .. --build missing
```

## Step 2: Build catapult

### Windows + Visual Studio

> **NOTE:**
> Make sure to use the correct ``PYTHON_EXECUTABLE`` path! Python3 is required for the build to produce some header files. If Python3 cannot be found you won't notice until more than one hour into the build process because of some missing headers. You can find your Python3 path by running ``where python3``.

* Generate project files for Visual Studio 2022:

  ```sh
  cmake -G "Visual Studio 17 2022" -A x64 -DUSE_CONAN=ON -DPYTHON_EXECUTABLE:FILEPATH=X:/python3x/python.exe ..
  ```

* Generate project files for Visual Studio 2019:

  ```sh
  cmake -G "Visual Studio 16 2019"  -A x64 -DUSE_CONAN=ON -DPYTHON_EXECUTABLE:FILEPATH=X:/python3x/python.exe ..
  ```

* Build:

  ```sh
  cmake --build . --target publish
  msbuild /p:Configuration=RelWithDebInfo /p:Platform=x64 /m ALL_BUILD.vcxproj
  ```

  After building successfully, the tools in ``_build\bin`` are ready to use. All runtime dependencies have been copied into the same folder so Windows will find them.

* Verify:

  Check that the tools are working correctly by running:

  ```sh
  bin\catapult.tools.address --help
  ```

### Linux and macOS

* Build:

  ```sh
  cmake -DUSE_CONAN=ON -DCMAKE_BUILD_TYPE=Release -G Ninja ..
  ninja publish
  ninja -j4
  ```

  Once the build finishes successfully, the tools in ``_build/bin`` are ready to use. However, the dependencies in ``_build/deps`` must be accessible so make sure to add this folder to the ``LD_LIBRARY_PATH`` environment variable (Linux) or ``DYLD_LIBRARY_PATH`` (Mac).

  One way of doing this is by running this from the ``_build`` directory:

  ```sh
  export LD_LIBRARY_PATH=$PWD/deps
  ```

  You will need to run this line every new session, unless you add it at the end of your ``~/.bashrc`` or ``~/.profile`` files.

* Install (Optional):

  The catapult tools can be made available globally by running:

  ```sh
  sudo ninja install
  ```

  > **NOTE:**
  > You can change the default installation location by passing ``-DCMAKE_INSTALL_PREFIX=...`` to ``cmake`` in the Build step. In this case you might not require ``sudo``.

* Verify:

  Check that the tools are working correctly by running:

  ```sh
  bin/catapult.tools.address --help
  ```
