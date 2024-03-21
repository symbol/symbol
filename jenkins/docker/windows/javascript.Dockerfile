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
	scoop install git shellcheck nodejs-lts cygwin rustup python; `
	python3 -m pip install --upgrade pip

# Set VS tools first in the path so the correct link.exe is used.
RUN Set-Content -Path c:\Users\ContainerAdministrator\.bash_profile  -Value 'export PATH=${VCToolsInstallDir}bin/Hostx64/x64:${PATH}'

# Install codecov
RUN Invoke-WebRequest -Uri https://uploader.codecov.io/latest/windows/codecov.exe -Outfile c:\Windows\System32\codecov.exe

# Restore the default Windows shell for correct setx processing.
SHELL ["cmd", "/S", "/C"]

RUN setx PATH "%PATH%;C:\Users\ContainerAdministrator\scoop\apps\rustup\current\.cargo\bin"
ENV CARGO_HOME=C:\Users\ContainerAdministrator\scoop\apps\rustup\current\.cargo

# Install wasm-pack
RUN cargo install wasm-pack

# install common python packages
RUN python3 -m pip install --upgrade gitlint isort lark pycodestyle pylint PyYAML wheel
