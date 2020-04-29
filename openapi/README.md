# symbol-openapi

[![Build Status](https://travis-ci.com/nemtech/symbol-openapi.svg?branch=master)](https://travis-ci.com/nemtech/symbol-openapi)

OpenAPI specification for catapult-rest.

## Requirements

* Node.js 12 LTS

## Installation

1. Clone the ``symbol-openapi`` repository.
```
git clone https://github.com/nemtech/symbol-openapi.git
```

2. Install ``swagger-cli`` globally.

```
npm install -g @apidevtools/swagger-cli
```

## Commands

### Build

Compile the specification.
The generated output is saved under ``_build`` directory.

```
npm run build
```

### Test

Check if the specification is valid. 

```
npm run test
```

## Contributing

Before contributing please [read this](CONTRIBUTING.md).

## License

Copyright (c) 2018-present NEM 
Licensed under the [Apache License 2.0](https://github.com/nemtech/nem2-docs/blob/master/LICENSE)
