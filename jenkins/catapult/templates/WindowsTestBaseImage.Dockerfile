# escape=`

ARG FROM_IMAGE=mcr.microsoft.com/powershell:latest

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

SHELL ["pwsh","-command"]

RUN pwsh.exe -ExecutionPolicy RemoteSigned -Command `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\scoop.ps1'); `
	$command='c:\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	del c:\scoop.ps1; `
	scoop install python git shellcheck; `
	python3 -m pip install --upgrade pip; `
	python3 -m pip install --upgrade colorama cryptography gitpython ply pycodestyle pylint pylint-quotes PyYAML
	
ENTRYPOINT ["pwsh.exe", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'Continue'; $verbosePreference='Continue';"]
CMD ["pwsh.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
