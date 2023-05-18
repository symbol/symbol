# Symbol Monorepo

In Q1 2021, we consolidated a number of projects into this repository.
It includes our specialized binary payload DSL (parser and schemas), clients and sdks.

| component | lint | build | test | coverage | package |
|-----------|------|-------|------|----------| ------- |
| [@catbuffer/parser](catbuffer/parser) | [![lint][catbuffer-parser-lint]][catbuffer-job] || [![test][catbuffer-parser-test]][catbuffer-job] <br> [![vectors][catbuffer-parser-vectors]][catbuffer-job] | [![][catbuffer-parser-cov]][catbuffer-parser-cov-link] | [![][catbuffer-package]][catbuffer-package-link] |
|||||||
| [@client/catapult](client/catapult) | [![lint][client-catapult-lint]][client-catapult-job] | [![build][client-catapult-build]][client-catapult-job] | [![build][client-catapult-test]][client-catapult-job] | [![][client-catapult-cov]][client-catapult-cov-link] |
| [@client/rest](client/rest) | [![lint][client-rest-lint]][client-rest-job] || [![test][client-rest-test]][client-rest-job] | [![][client-rest-cov]][client-rest-cov-link] |
|||||||
| [@sdk/javascript](sdk/javascript) | [![lint][sdk-javascript-lint]][sdk-javascript-job] | [![build][sdk-javascript-build]][sdk-javascript-job] | [![test][sdk-javascript-test]][sdk-javascript-job] <br> [![examples][sdk-javascript-examples]][sdk-javascript-job] <br> [![vectors][sdk-javascript-vectors]][sdk-javascript-job] | [![][sdk-javascript-cov]][sdk-javascript-cov-link] | [![][sdk-javascript-package]][sdk-javascript-package-link] |
| [@sdk/python](sdk/python) | [![lint][sdk-python-lint]][sdk-python-job] | [![build][sdk-python-build]][sdk-python-job] | [![test][sdk-python-test]][sdk-python-job] <br> [![examples][sdk-python-examples]][sdk-python-job] <br> [![vectors][sdk-python-vectors]][sdk-python-job] | [![][sdk-python-cov]][sdk-python-cov-link] | [![][sdk-python-package]][sdk-python-package-link] |
|||||||
| [@linters](linters) | [![lint][linters-lint]][linters-job] |||||
| [@jenkins](jenkins) | [![lint][jenkins-lint]][jenkins-job] |||||

## Full Coverage Report

Detailed version can be seen on [codecov.io][symbol-cov-link].

[![][symbol-cov]][symbol-cov-link]

[symbol-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graphs/tree.svg
[symbol-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev

[catbuffer-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fcatbuffer-parser/activity/?branch=dev
[catbuffer-parser-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fcatbuffer-parser%2Fdev%2F&config=catbuffer-parser-lint
[catbuffer-parser-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fcatbuffer-parser%2Fdev%2F&config=catbuffer-parser-test
[catbuffer-parser-vectors]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fcatbuffer-parser%2Fdev%2F&config=catbuffer-parser-vectors
[catbuffer-parser-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=catbuffer-parser
[catbuffer-parser-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/catbuffer/parser
[catbuffer-package]: https://img.shields.io/pypi/v/catparser
[catbuffer-package-link]: https://pypi.org/project/catparser

[client-catapult-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fclient-catapult/activity?branch=dev
[client-catapult-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fclient-catapult%2Fdev%2F&config=client-catapult-lint
[client-catapult-build]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fclient-catapult%2Fdev%2F&config=client-catapult-build
[client-catapult-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fclient-catapult%2Fdev%2F&config=client-catapult-test
[client-catapult-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=client-catapult
[client-catapult-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/client/catapult

[client-rest-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fclient-rest/activity?branch=dev
[client-rest-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fclient-rest%2Fdev%2F&config=client-rest-lint
[client-rest-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fclient-rest%2Fdev%2F&config=client-rest-test
[client-rest-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=client-rest
[client-rest-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/client/rest

[sdk-javascript-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript/activity?branch=dev
[sdk-javascript-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript%2Fdev%2F&config=sdk-javascript-lint
[sdk-javascript-build]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript%2Fdev%2F&config=sdk-javascript-build
[sdk-javascript-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript%2Fdev%2F&config=sdk-javascript-test
[sdk-javascript-examples]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript%2Fdev%2F&config=sdk-javascript-examples
[sdk-javascript-vectors]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-javascript%2Fdev%2F&config=sdk-javascript-vectors
[sdk-javascript-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=sdk-javascript
[sdk-javascript-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/sdk/javascript
[sdk-javascript-package]: https://img.shields.io/npm/v/symbol-sdk-javascript
[sdk-javascript-package-link]: https://www.npmjs.com/package/symbol-sdk-javascript

[sdk-python-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fsdk-python/activity?branch=dev
[sdk-python-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-python%2Fdev%2F&config=sdk-python-lint
[sdk-python-build]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-python%2Fdev%2F&config=sdk-python-build
[sdk-python-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-python%2Fdev%2F&config=sdk-python-test
[sdk-python-examples]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-python%2Fdev%2F&config=sdk-python-examples
[sdk-python-vectors]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fsdk-python%2Fdev%2F&config=sdk-python-vectors
[sdk-python-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=sdk-python
[sdk-python-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/sdk/python
[sdk-python-package]: https://img.shields.io/pypi/v/symbol-sdk-python
[sdk-python-package-link]: https://pypi.org/project/symbol-sdk-python

[jenkins-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fjenkins/activity?branch=dev
[jenkins-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fjenkins%2Fdev%2F&config=jenkins-lint

[linters-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Flinters/activity?branch=dev
[linters-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Flinters%2Fdev%2F&config=linters-lint
