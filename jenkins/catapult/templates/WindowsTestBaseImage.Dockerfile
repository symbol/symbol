# escape=`

FROM {{BASE_IMAGE}}
MAINTAINER Catapult Development Team
SHELL ["cmd", "/S", "/C"]

RUN powershell.exe -ExecutionPolicy RemoteSigned `
  (new-object net.webclient).DownloadFile('https://get.scoop.sh','c:\scoop.ps1'); `
  $command='c:\scoop.ps1 -RunAsAdmin'; `
  iex $command; `
  del c:\scoop.ps1; `
  scoop install python git shellcheck; `
  python3 -m pip install --upgrade pip; `
  python3 -m pip install --upgrade colorama cryptography gitpython ply pycodestyle pylint pylint-quotes PyYAML

CMD ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
