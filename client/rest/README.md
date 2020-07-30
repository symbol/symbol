# catapult-rest

[![Build Status](https://api.travis-ci.com/nemtech/catapult-rest.svg?branch=main)](https://travis-ci.com/nemtech/catapult-rest)
[![Coverage Status](https://coveralls.io/repos/github/nemtech/catapult-rest/badge.svg?branch=main)](https://coveralls.io/github/nemtech/catapult-rest?branch=main)

Catapult REST gateway combines HTTP and WebSockets to perform read and write actions on the blockchain.

## Requirements

- Node.js 12 LTS
- [yarn][yarn] dependency manager
- [catapult-server][catapult-server] configured as an [API or Dual node][api-node]
- MongoDB 4.2

## Installation

1. Make sure the configuration file ``rest/resources/rest.json`` matches the API node connection details.

| Parameter | Description | Example  |
|-|-|-|
| db.url | MongoDB connection URL. | mongodb://localhost:27017/ |
| apiNode.host | API node connection host. | 127.0.0.1 |
| apiNode.port | API node connection port. | 7900 |
| websocket.mq.host | ZeroMQ connection host. |  127.0.0.1 |
| websocket.mq.port | ZeroMQ connection port. |  7902 |

*Note:* catapult-rest has to reach the API node, ZeroMQ, and MongoDB ports. If you are running catapult-server on a VPS, you can bind the ports to your local development environment creating an **SSH tunnel**: ``ssh -L 27017:localhost:27017 -L 7900:localhost:7900 -L 7902:localhost:7902 -p 2357 <USER>@<VPS_IP>``

2. Catapult uses **TLS 1.3** to provide secure connections and identity assurance between the nodes.
To generate and self sign the certificates, you can download and run the script [cert-generate.sh](https://github.com/tech-bureau/catapult-service-bootstrap/blob/master/common/ruby/script/cert-generate.sh).

```ssh
mkdir certificate
cd certificate
curl https://raw.githubusercontent.com/tech-bureau/catapult-service-bootstrap/master/common/ruby/script/cert-generate.sh --output cert-generate.sh
chmod 777 cert-generate.sh
./cert-generate.sh
```
Alternatively, you could use a certificate issued by a certification authority (CA).
Then, edit ``rest/resources/rest.json`` TLS configuration:

| Parameter | Description | Example  |
|-|-|-|
| apiNode.tlsClientCertificatePath | TLS client certificate path. | /certificate/node.crt.pem |
| apiNode.tlsClientKeyPath | TLS client key certificate path.  | /certificate/node.key.pem |
| apiNode.tlsCaCertificatePath| TLS CA certificate path. | /certificate/ca.cert.pem |

3. Install the project dependencies:

```
./yarn_setup.sh
```

4. Run catapult-rest:

```
cd rest
yarn build
yarn start resources/rest.json
```

If everything goes well, you should see catapult-rest running by opening ``localhost:3000/node/info`` in a new browser tab.

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
[api-node]: https://nemtech.github.io/server.html#installation
