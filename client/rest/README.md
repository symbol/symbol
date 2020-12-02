# catapult-rest

[![Build Status](https://api.travis-ci.com/nemtech/catapult-rest.svg?branch=main)](https://travis-ci.com/nemtech/catapult-rest)
[![Coverage Status](https://coveralls.io/repos/github/nemtech/catapult-rest/badge.svg?branch=main)](https://coveralls.io/github/nemtech/catapult-rest?branch=main)

Catapult REST gateway combines HTTP and WebSockets to perform read and write actions on the blockchain.

## Requirements

- Node.js 12 LTS
- [yarn][yarn] dependency manager
- [docker][docker]
- [symbol-bootstrap][symbol-bootstrap] 

## Installation

1. Validate you are able to run Bootstrap by following the [requirements](https://github.com/nemtech/symbol-bootstrap#requirements).

2. Install the project dependencies:

```
./yarn_setup.sh
```

3. Run a Symbol private network using Bootstrap:

```
cd rest
yarn build
yarn bootstrap-start
```

This Symbol network is a [light](https://github.com/nemtech/symbol-bootstrap#out-of-the-box-presets) preset network without rest. 
Rest will be running from source code, so you can test your changes! 
Mongo DB (27017), Server (7900) and Broker (7902) ports are open to localhost.

4. Run catapult-rest:

In another terminal:

```
yarn start:dev
```

If everything goes well, you should see catapult-rest running by opening ``http://localhost:3000/node/info`` in a new browser tab.

Alternatively, you can run bootstrap in `detached` mode to avoid opening a new terminal. 

```
cd rest
yarn build
yarn bootstrap-start-detached
yarn start:dev
yarn bootstrap-stop
```

Useful for test automation: 

```
cd rest
yarn build
yarn bootstrap-start-detached
yarn test
yarn bootstrap-stop
```

## Testnet

Another alternative, is having bootstrap creating a Testnet node without rest that you can run from code:

```
yarn bootstrap-start-testnet
```

In another terminal

```
yarn start:dev
```


## Usage

Please refer to the [documentation](https://nemtech.github.io/api.html) for more information.

## Versioning

Make sure you choose a [version compatible](COMPATIBILITY.md) with the [catapult-server][catapult-server] node you want to use it with.

Starting on `v1.1.0`, version numbers are described as follows:

`vX.Y.Z`

- X: This serves to lock for compatibility with `catapult-server`, thus it is safe to update by keeping this number without REST
losing server compatibility. Additionally, any breaking change to the server should require to upgrade this number.
- Y: This serves to lock on safe updates to this project, thus it is safe to update by keeping this number without worrying about
introducing breaking changes.
- Z: Represents minor changes progress, used to identify specific versions when reporting bugs, or to get extensions to the code.

## Contributing

Before contributing please [read this](CONTRIBUTING.md) and consider the following guidelines:
- Submit small and concise PRs that address a single and clear feature or issue
- Submit only fully tested code
- Split test scope areas with _Arrange/Act/Assert_ comments
- Use spontaneous comments only when necessary
- Follow linting rules - tests are set to fail if those aren't followed
- Notify or update related API resources of accepted changes ([OpenAPI](https://github.com/nemtech/symbol-openapi))

## License

Copyright (c) 2018 Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp Licensed under the [GNU Lesser General Public License v3](LICENSE)

[yarn]: https://yarnpkg.com/lang/en/
[catapult-server]: https://github.com/nemtech/catapult-server
[symbol-bootstrap]: https://github.com/nemtech/symbol-bootstrap
[docker]: https://www.docker.com
[api-node]: https://nemtech.github.io/server.html#installation
