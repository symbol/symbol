# escape=`

ARG FROM_IMAGE=symbolplatform/symbol-server-compiler:windows-msvc-17

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

SHELL ["pwsh", "-command", "$ErrorActionPreference = 'Stop';"]

# Microsoft Visual C++ Redistributable packages for Visual Studio 2013 required by pyzbar
ADD https://aka.ms/highdpimfc2013x64enu c:\temp\vc_redist.x64.exe
RUN Start-Process -filepath C:\temp\vc_redist.x64.exe -ArgumentList "/install", "/passive", "/norestart" -Passthru | Wait-Process

RUN Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser; `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\scoop.ps1'); `
	$command='c:\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	del c:\scoop.ps1; `
	scoop install python git shellcheck openssl cmake cygwin; `
	python3 -m pip install --upgrade pip; `
	python3 -m pip install --upgrade gitlint wheel

# Install codecov
RUN Invoke-WebRequest -Uri https://uploader.codecov.io/latest/windows/codecov.exe -Outfile c:\Windows\System32\codecov.exe

ENV OPENSSL_ROOT_DIR='C:\Users\ContainerAdministrator\scoop\apps\openssl\current'
