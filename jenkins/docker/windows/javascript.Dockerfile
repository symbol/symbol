# escape=`

ARG FROM_IMAGE=symbolplatform/symbol-server-compiler:windows-msvc-17

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

SHELL ["pwsh", "-command", "$ErrorActionPreference = 'Stop';"]

RUN Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser; `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\scoop.ps1'); `
	$command='c:\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	del c:\scoop.ps1; `
	scoop install python git shellcheck nodejs-lts cygwin rustup; `
	python3 -m pip install --upgrade pip; `
	python3 -m pip install --upgrade gitlint wheel

# Install codecov
RUN Invoke-WebRequest -Uri https://uploader.codecov.io/latest/windows/codecov.exe -Outfile c:\Windows\System32\codecov.exe

# Restore the default Windows shell for correct setx processing.
SHELL ["cmd", "/S", "/C"]

RUN setx PATH "%PATH%;C:\Users\ContainerAdministrator\scoop\apps\rustup\current\.cargo\bin"
ENV CARGO_HOME=C:\Users\ContainerAdministrator\scoop\apps\rustup\current\.cargo

# Install wasm-pack
RUN cargo install wasm-pack
