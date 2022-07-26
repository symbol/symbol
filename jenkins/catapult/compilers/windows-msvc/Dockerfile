# escape=`

# Use the latest Windows Server Core 2022 image.
ARG FROM_IMAGE=mcr.microsoft.com/windows/servercore:ltsc2022
FROM ${FROM_IMAGE}

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

# Download the Visual Studio Build Tools bootstrapper.
ARG BUILDTOOLS_URL=https://aka.ms/vs/17/release/vs_buildtools.exe
ADD ${BUILDTOOLS_URL} C:\Temp\vs_buildtools.exe

# Use the latest release channel. For more control, specify the location of an internal layout.
ARG CHANNEL_URL=https://aka.ms/vs/17/release/channel
ADD ${CHANNEL_URL} C:\TEMP\VisualStudio.chman

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

# Install Python and Git.
RUN powershell.exe -ExecutionPolicy RemoteSigned `
  (new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\Temp\scoop.ps1'); `
  $command='c:\Temp\scoop.ps1 -RunAsAdmin'; `
  iex $command; `
  scoop install python git

RUN python3 -m pip install -U "conan>=1.33.0"

# Start developer command prompt with any other commands specified.
ENTRYPOINT ["C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&"]
