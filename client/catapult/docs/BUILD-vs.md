[Visual Studio]: https://www.visualstudio.com/downloads
[vcpkg]: https://github.com/microsoft/vcpkg
[Python]: https://www.python.org/downloads/windows/
[Git]: https://git-scm.com/download/win

# Building with VisualStudio 2019

Following instructions are meant for integrated builds using VisualStudio on Windows only. This instructions **do not appy to VSCode**

## Mandatory version requirements
- [Git] version >= 2.25
- [Python] version >= 3.8
- [Visual Studio] 2019+ (2022 recommended): tested on 2022 (17.9.6)
- [vcpkg] package manager >= 2024.04.26 (instructions for install below)

**Important: Ensure your VS setup includes CMAKE support**

## Setting up the environment

### Obtaining [vcpkg]

1. From a terminal window clone the vcpkg repository in a directory of your choice. If you omit to specify a directory, the repository will be cloned in a sub-directory of your current working dir named `vcpkg`.

```shell
git clone https://github.com/microsoft/vcpkg.git [<your_chosen_directory>]
cd vcpkg
```

2. Run the bootstrap script to build the vcpkg executable
```shell
.\bootstrap-vcpkg.bat
```

3. Set a global environment variable named 'VCPKG_ROOT' to the full path where you have cloned [vcpkg] in point 1 (e.g. C:\vcpkg). This is needed for the CMake scripts to find the vcpkg toolchain file.

### Obtaining the source code
From a terminal window (or using Git for Windows if you have it) clone the Symbol repository
```shell
git clone https://github.com/symbol/symbol.git
```

## Build Catapult

1. Open your [Visual Studio] instance.
2. Go to `File -> Open -> CMake...` and select the `CMakeLists.txt` file from the subdirectory `client\catapult` of the `symbol` repository you cloned in the previous step.							
This will trigger the CMake configuration process which will also download and build the necessary dependencies using [vcpkg]. This is a long process depending on the capabilities of your hardware and internet connection. Be patient !!
3. Wait for the CMake configuration to finish. You will see a bunch of messy messages in your CMake output window. Once it's done, you will a final line saying `1> CMake generation finished.`.	
4. The project is now fully configured and ready to build. 
5. You can now build the project by clicking on the `Build -> Build All` menu item. This will build the entire project's binaries. This is also a long process. Be patient !!

At the end of the build procedure check for any errors in the output window. Should there be some please let us know opening an issue.

### Verify Catapult build

By default builds are created in the `build` directory of the `catapult` project. On behalf of your build type (i.e. Debug or Release) you will find the binaries in the corresponding sub-directory. For example if you built the Debug configuration, you will find the binaries in `build\X64-Debug\bin`.
Directly from within Visual Studio access the Developer Powershell (it opens with current work directory set to the catapult project) and run the following command to start the catapult server:
```shell
.\build\x64-Debug\bin\Debug\catapult.tools.address.exe --help
```
If something like this appears
```shell
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
you're all set and have successfully built Catapult