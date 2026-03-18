# symbol-openapi

OpenAPI specification for catapult-rest.

## Requirements

* Node.js 18 LTS or higher

## Installation

1. Clone the `symbol-openapi` repository.

```
git clone https://github.com/symbol/symbol-openapi.git
```

2. Install dependencies.

```
npm install
```

## Commands

### Build

Compile the specification. The generated output is saved under the `_build` directory.

```
npm run build
```

### Test

Validate the specification using Redocly CLI.

```
npm test
```

### Postman

Generate a Postman collection from the built specification.

```
npm run postman
```

## Contributing

Before contributing please [read this](CONTRIBUTING.md).

## License

Copyright (c) 2018 Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp Licensed under the [GNU Lesser General Public License v3](LICENSE.txt)
