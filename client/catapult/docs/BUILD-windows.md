[Visual Studio]: https://visualstudio.microsoft.com/downloads/
[Build Tools for Visual Studio]: https://visualstudio.microsoft.com/downloads/
[vcpkg]: https://github.com/microsoft/vcpkg
[Conan]: https://conan.io/downloads.html
[Python]: https://www.python.org/downloads/windows/
[Git]: https://git-scm.com/download/win

# Building on Windows

This guide describes how to build catapult on Windows.
There are two supported build techniques on Windows:

- [Visual Studio] with vcpkg (easiest and recommended)
- [Visual Studio] with Conan (not integrated with Visual Studio, mostly command line)

The recommended approach is to use Visual Studio with support for CMake.
You can either use the full Visual Studio IDE or just [Build Tools for Visual Studio],
but in both cases you need to have CMake support installed.

- [Building with Visual Studio and vcpkg](#building-with-visual-studio-and-vcpkg)
  - [Prerequisites](#prerequisites)
  - [Setting up the environment](#setting-up-the-environment)
  - [Building from within the Visual Studio IDE](#building-from-within-the-visual-studio-ide)
  - [Building from the command line](#building-from-the-command-line)
  - [Verify Catapult build](#verify-catapult-build)
- [Building with Visual Studio and Conan](#building-with-visual-studio-and-conan)
  - [Prerequisites](#prerequisites-1)
  - [Setting up the environment](#setting-up-the-environment-1)
  - [Let Conan install dependencies](#let-conan-install-dependencies)
  - [Configure and build](#configure-and-build)
  - [Verify Catapult build](#verify-catapult-build-1)

## Building with Visual Studio and vcpkg

### Prerequisites

Install either [Visual Studio] or [Build Tools for Visual Studio] with C++ and CMake support.
The supported versions of Visual Studio are listed below due to the availability of CMake >= 3.25 which is required for the build.

* Visual Studio 17 2022 (>= 17.5)
* Visual Studio 18 2026

Required Visual Studio components:

- Desktop development with C++ (MSVC toolset)
- C++ CMake tools for Windows
- Windows 10/11 SDK
- At least 60 GB of free disk space for the build and dependencies (this is an estimate and can vary based on the number of dependencies and their sizes)

Additional requirements:

- [Git] >= 2.25 (if not already installed with Visual Studio)
- [vcpkg] package manager (to be installed separately)

> [!WARNING]
> ## ⚠️ DO NOT USE THE VCPKG BUNDLED WITH VISUAL STUDIO ⚠️
> For this project, **you must install vcpkg manually from a Git clone**.
> Do **not** use the Visual Studio bundled/integrated vcpkg installation.
>
> ✅ Supported: `git clone https://github.com/microsoft/vcpkg.git` + `bootstrap-vcpkg.bat`
>
> ❌ Not supported: vcpkg installed via Visual Studio installer/components. Please note that even if you have cloned vcpkg from Git but you also have the Visual Studio bundled vcpkg, you may run into issues because of conflicts between the two installations.

If you want to install [Git] manually, download it from <https://git-scm.com/download/win>.
Install it before the next steps because it is needed to clone catapult and optionally vcpkg.

### Setting up the environment

Install [vcpkg] manually from Developer PowerShell for Visual Studio (you can skip this step if you already have a manual vcpkg installation, but remember to set the `VCPKG_ROOT` environment variable as described below):
```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

After bootstrap completes, set an environment variable named `VCPKG_ROOT` to the full path where you cloned vcpkg (for example, `C:\vcpkg`).
To do this you MUST open a Developer PowerShell for Visual Studio in Administrator mode and run the following command:
```powershell
[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT','C:\vcpkg', 'Machine')
```

Again ... remember to change `C:\vcpkg` to the actual path where you cloned vcpkg.
Alternatively, you can set the environment variable through the Windows System Properties -> Environment Variables UI.

Open Visual Studio and clone the catapult repository using the built-in Git integration:
- Go to **Git -> Clone Repository**.
- In the **Repository location** enter the URL `https://github.com/symbol/symbol.git`
- In the **Path** field, choose a local directory where you want to clone the repository (for example, `C:\symbol`).
- Click **Clone** to start cloning the repository.

If you prefer to use a terminal, you can clone the repository using Developer PowerShell for Visual Studio.
Replace `<your_chosen_directory>` with the path where you want to clone the repository:
```powershell
git clone https://github.com/symbol/symbol.git <your_chosen_directory>
```

### Building from within the Visual Studio IDE

Open Visual Studio, then go to **File -> Open -> CMake...** and select the `CMakeLists.txt` file from the
`client\catapult` subdirectory of the `symbol` repository you cloned earlier.
This starts CMake configuration and downloads/builds dependencies through vcpkg.
This step can take a while depending on your hardware and internet connection.

You may want to choose the CMake preset from the toolbar before building.
For example, select `Release` to build the Release configuration.

When configuration finishes, the CMake output window shows `1> CMake generation finished.`.
At this point, the project is fully configured and ready to build.
You can now build the project by clicking on the `Build -> Build All` menu item or by pressing `Ctrl+Shift+B`.

The build process may take a while.
At the end of the build, check the output window for errors.
If there are issues, please open an issue.
The binaries will be available in the `build\<name>\bin` directory (e.g. `build\Release\bin` for Release configuration).

If you want to run unit tests, you can select the `Test Explorer` tab in Visual Studio and run the tests from there but :
- You must have built the Debug configuration to have the tests available.
- You may need to set the test adapter to `Google Test Adapter` in the Test Explorer
- You need to set the Working Directory for the tests to `$(SolutionDir)/build`.
- Do not enable parallel test execution as it may cause issues with the tests.

### Building from the command line

If you prefer the command line, **or if you installed Build Tools for Visual Studio only**,
use Developer PowerShell for Visual Studio and run the following commands from `symbol\client\catapult`:
```powershell
cmake --workflow --preset <name>
```

Where `<name>` is the name of the preset you want to build (e.g. `Release`).
This single command will configure and build the project in one step. 
The whole process may take a while depending on your hardware and internet connection.

Here is a list of available presets you can obtain by running `cmake --list-presets` from the `symbol\client\catapult` directory:
```
  "Debug"          - x64 Debug
  "Release"        - x64 Release
  "RelWithDebInfo" - x64 RelWithDebInfo
  "MinSizeRel"     - x64 MinSizeRel
```

### Verify Catapult build

By default, builds are created in the `build` directory of the `catapult` project.
Based on your preset, binaries are generated in the corresponding subdirectory.
For example, if you build Debug,
you will find the binaries in `build\Debug\bin`.

You can verify the build by running this command from Developer PowerShell for Visual Studio with current directory set to `symbol\client\catapult`:
```powershell
.\build\<name>\bin\catapult.tools.address.exe --help
```

You should see output similar to this:
```powershell
Address Inspector Tool
Copyright (c) Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
catapult version: 1.0.3.7 3d657205 [VisualStudio]

Address Inspector Tool options:
  -h [ --help ]     print help message
  -l [ --loggingConfigurationPath ] arg
                    path to the logging configuration file
  -n [ --network ] arg (=testnet)
                    network, possible values: testnet (default), mainnet
  -i [ --input ] arg
                    input value (comma-delimited) or file
  -o [ --output ] arg
                    (optional) output file
  -f [ --format ] arg (=pretty)
                    output format, possible values: pretty (default), csv
  --suppressConsole
                    true to suppress console output
  -m [ --mode ] arg mode, possible values: encoded, decoded, public, secret
```

Should you have built in Debug configuration, you have also unit tets built. You can run them with this command:
```powershell
cd build
ctest --test-dir=.\<name>\
```

## Building with Visual Studio and Conan

[Conan] is also supported as an alternative to vcpkg.
However, this approach is not integrated with Visual Studio IDE and is mostly command-line based.
Conan generates it own preset named `conan-default` that can be used to build the project with CMake.

### Prerequisites

- Same as above, except for vcpkg which is not needed for this approach.
- [Git] >= 2.25 (for cloning the repository)
- [Python] >= 3.8
- [Conan] package manager >= 2.0 (you can install it with `py -3 -m pip install --upgrade "conan>=2,<3"`)

Ensure Python is added to your system `PATH` so you can run it from any terminal.

### Setting up the environment

Firstly we need to ensure that Conan is configured to use the correct profile.
From Developer PowerShell for Visual Studio, run the following command to detect and set up the default profile:
```powershell
conan profile detect --name default --force
```
This will create a Conan profile named `default` with the appropriate settings matching Visual Studio environment.

Next, we need to add the Conan remote repository where catapult dependencies are hosted:
```powershell
conan remote add nemtech https://conan.symbol.dev/artifactory/api/conan/catapult
```

Now we're ready to clone the catapult repository either with Git integration or from terminal:
```powershell
git clone https://github.com/symbol/symbol.git <your_chosen_directory>
```

### Let Conan install dependencies

From Developer PowerShell for Visual Studio, navigate to `symbol\client\catapult` and run:
```powershell
conan install . --build=missing -s compiler.cppstd=17 -s build_type=Release
cd build
```
Where `build_type` can be `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel`.

This will create a `build` directory with the Conan configuration and generated files.
All dependencies are downloaded and built in the `build` directory as well.
This can take a while depending on your hardware and internet connection.

### Configure and build

Next, configure the project with CMake and build it:
```powershell
cmake --preset conan-default
cmake --build --preset conan-<build-type>
```
Where `<build-type>` is the build type you used in the Conan install step (e.g. `Release`) but in lowercase.

After a successful build, tools are available under `build\bin\<configuration>` (e.g. `build\bin\Release` for Release configuration).

### Verify Catapult build

You can verify the build by running this command from Developer PowerShell for Visual Studio with current directory set to `symbol\client\catapult`:
```powershell
.\build\bin\<configuration>\catapult.tools.address.exe --help
```
Where `<configuration>` is the build configuration you used (e.g. `Release`).

You should see output similar to this:
```powershell
Address Inspector Tool
Copyright (c) Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
catapult version: 1.0.3.7 3d657205 [VisualStudio]

Address Inspector Tool options:
  -h [ --help ]     print help message
  -l [ --loggingConfigurationPath ] arg
                    path to the logging configuration file
  -n [ --network ] arg (=testnet)
                    network, possible values: testnet (default), mainnet
  -i [ --input ] arg
                    input value (comma-delimited) or file
  -o [ --output ] arg
                    (optional) output file
  -f [ --format ] arg (=pretty)
                    output format, possible values: pretty (default), csv
  --suppressConsole
                    true to suppress console output
  -m [ --mode ] arg mode, possible values: encoded, decoded, public, secret
```
