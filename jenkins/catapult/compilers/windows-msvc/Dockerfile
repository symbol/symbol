# escape=`

# Use the latest Windows Server Core 2022 image.
ARG FROM_IMAGE=mcr.microsoft.com/windows/servercore:ltsc2022
FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1 /f

# Download the Visual Studio Build Tools bootstrapper.
ARG COMPILER_VERSION=17
ADD "https://aka.ms/vs/${COMPILER_VERSION}/release/vs_buildtools.exe" C:\Temp\vs_buildtools.exe

# Use the latest release channel. For more control, specify the location of an internal layout.
ADD "https://aka.ms/vs/${COMPILER_VERSION}/release/channel" C:\Temp\VisualStudio.chman

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
RUN powershell.exe -ExecutionPolicy RemoteSigned `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\Temp\scoop.ps1'); `
	$command='c:\Temp\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	scoop install git

# Start developer command prompt with any other commands specified.
ENTRYPOINT ["C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&"]
