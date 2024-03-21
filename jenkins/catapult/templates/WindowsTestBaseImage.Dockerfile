# escape=`

ARG FROM_IMAGE=mcr.microsoft.com/powershell:latest

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"

SHELL ["pwsh","-command", "$ErrorActionPreference = 'Stop';"]

RUN Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser; `
	(new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\scoop.ps1'); `
	$command='c:\scoop.ps1 -RunAsAdmin'; `
	iex $command; `
	del c:\scoop.ps1; `
	scoop install python git shellcheck; `
	python3 -m pip install --upgrade pip; `
	python3 -m pip install --upgrade colorama conan cryptography gitpython ply pycodestyle pylint PyYAML
	
RUN "'ACP', 'OEMCP', 'MACCP' | Set-ItemProperty 'HKLM:\SYSTEM\CurrentControlSet\Control\Nls\CodePage' -Name { $_ } 65001"; `
	Get-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Nls\CodePage'

RUN Set-ItemProperty 'HKLM:\Software\Microsoft\Command Processor' -Name Autorun 'chcp 65001'; `
	Get-ItemProperty -Path 'HKLM:\Software\Microsoft\Command Processor'

RUN '$OutputEncoding = [console]::InputEncoding = [console]::OutputEncoding = [System.Text.Utf8Encoding]::new()' + [Environment]::Newline + `
	(Get-Content -Raw $PROFILE.AllUsersCurrentHost -ErrorAction SilentlyContinue) | Set-Content -Encoding utf8 $PROFILE.AllUsersCurrentHost
	
ENTRYPOINT ["pwsh.exe", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'Continue'; $verbosePreference='Continue';"]
CMD ["pwsh.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
