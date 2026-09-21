# escape=`

# Use the latest Windows Powershell image.
ARG FROM_IMAGE=mcr.microsoft.com/powershell:latest
FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1 /f

# Download the Visual Studio Build Tools bootstrapper.
ARG COMPILER_VERSION=18
RUN mkdir C:\Temp && cmd /S /V:ON /C "set CHANNEL=release& `
	if ""%COMPILER_VERSION%"" == ""18"" set CHANNEL=stable& `
	echo Using Visual Studio %COMPILER_VERSION% channel: !CHANNEL!& `
	curl.exe -fL --retry 5 --retry-delay 5 --retry-all-errors ""https://aka.ms/vs/%COMPILER_VERSION%/!CHANNEL!/vs_buildtools.exe"" -o C:\Temp\vs_buildtools.exe && `
	curl.exe -fL --retry 5 --retry-delay 5 --retry-all-errors ""https://aka.ms/vs/%COMPILER_VERSION%/!CHANNEL!/channel"" -o C:\Temp\VisualStudio.chman"

# Install MSVC C++ compiler, CMake, and MSBuild.
RUN C:\Temp\vs_buildtools.exe `
	--quiet --wait --norestart --nocache `
	--installPath C:\BuildTools `
	--channelUri C:\Temp\VisualStudio.chman `
	--installChannelUri C:\Temp\VisualStudio.chman `
	--add Microsoft.VisualStudio.Workload.VCTools `
	--add Microsoft.Component.MSBuild `
	--includeRecommended `
	|| IF "%ERRORLEVEL%"=="3010" EXIT 0

# Install scoop and git(required to update scoop)
RUN pwsh.exe -ExecutionPolicy RemoteSigned -Command $ErrorActionPreference = 'Stop'; `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\Temp\scoop.ps1'); `
	$command='c:\Temp\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	scoop install git

# Start developer command prompt with any other commands specified.
ENTRYPOINT ["C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&"]
