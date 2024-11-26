# Light-rest

## Introduction

The light version of the REST server provides limited endpoints compared to the full API node. The main differences between the two are:

- The light rest does not use MongoDB or broker services.
- Light REST only communicates with the server using a socket connection.

## Purpose

The purpose of the light rest is to reduce server resource usage, making it a cost-effective. it also offers users the ability to delegate harvesting to the peer node.

## Supported endpoints

- chain/info
- node/info
- node/peers
- node/unlockedaccount
- node/server

review the [REST documentation](https://docs.symbol.dev/api.html) for more information.