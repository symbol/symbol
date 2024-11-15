# Light-rest

## Introduction

This is a lightweight version of the REST server. It provides a limited set of endpoints compared to the full API node. The main differences between the light rest and the full API node are:

- The light rest does not use MongoDB or broker services.
- it communicates with the peer node using a socket connection.

## Purpose

The purpose of the light rest is to reduce server resource usage, making it a cost-effective. it also offers users the ability to delegate harvesting to the peer node.

## Supported endpoints

- chain/info
- node/info
- node/peers
- node/unlockedaccount
- node/server

review the [REST documentation](https://docs.symbol.dev/api.html) for more information.