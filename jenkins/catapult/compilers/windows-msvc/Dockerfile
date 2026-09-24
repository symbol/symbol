# escape=`

# Base image is the windows version from jenkins/catapult/versions.properties.
ARG FROM_IMAGE=mcr.microsoft.com/windows/servercore:ltsc2025
FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1 /f

# Download the Visual Studio Build Tools bootstrapper.
ARG COMPILER_VERSION=18
RUN mkdir C:\Temp && cmd /S /V:ON /C " `
		(if "%COMPILER_VERSION%" == "18" (set CHANNEL=stable) else (set CHANNEL=release)) && `
		(set BOOTSTRAPPER_URL=https://aka.ms/vs/%COMPILER_VERSION%/!CHANNEL!/vs_buildtools.exe) && `
		echo Downloading from !BOOTSTRAPPER_URL! && `
		curl.exe -fL --retry 5 --retry-delay 5 --retry-all-errors "!BOOTSTRAPPER_URL!" -o C:\Temp\vs_buildtools.exe"


# Install MSVC C++ compiler, CMake, and MSBuild.
RUN cmd /S /V:ON /C "C:\Temp\vs_buildtools.exe `
	--quiet --wait --norestart --nocache `
	--installPath C:\BuildTools `
	--add Microsoft.VisualStudio.Workload.VCTools `
	--add Microsoft.Component.MSBuild `
	--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
	--add Microsoft.VisualStudio.Component.VC.CMake.Project `
	--add Microsoft.VisualStudio.Component.VC.ASAN `
	--add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
	--includeRecommended `
	& if !ERRORLEVEL! == 3010 (exit /b 0) else (exit /b !ERRORLEVEL!) && `
    del /F vs_buildtools.exe"

# Install scoop, git (required to update scoop) and PowerShell 7.
RUN powershell.exe -ExecutionPolicy RemoteSigned -Command $ErrorActionPreference = 'Stop'; `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\Temp\scoop.ps1'); `
	$command='c:\Temp\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	Remove-Item -Force 'c:\Temp\scoop.ps1'; `
	scoop install git pwsh

# Start developer command prompt with any other commands specified.
ENTRYPOINT ["C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&"]
