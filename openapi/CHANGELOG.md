# CHANGELOG

All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.1] - 19-Oct-2021

Milestone: Symbol Mainnet

- fix: Address and UnresolvedAddress is Base32, not Hex.
- feat: Added feeMultiplier and timestamp to transaction's meta.
- feat: Added meta to transaction and resolution statements.
- feat: added deployment data to /node/server.

## [1.0.0] - 13-Mar-2021

Milestone: Symbol Mainnet

- Release for Symbol mainnet.

## [0.11.2] - 02-Feb-2021

Targets catapult-rest 2.3.2

### Added

- `FromHeight` and `ToHeight` filters in receipt search endpoint.

## [0.11.1] - 14-Jan-2021

Targets catapult-rest 2.3.0

### Added

- `minFeeMultiplier` in transaction fees endpoints from the current connected Node.

#### Updated

- Re tracked catapult `main` branch.
- Updated `PrivateTest` network type from `0x80` to `0xA8`.

## [0.10.5] - 10-Dec-2020

Targets catapult-rest 2.2.1

### Fixed

- Finalization proof schame v1 / v2 compatibility issue.

## [0.10.4] - 8-Dec-2020

Targets catapult-rest 2.2.0

### Added

- Added new core failure status.
- Added version field to states.
- Added ``/merkle`` endpoints and Merkle Tree for states.
- Added new block types.
- Added ``/node/unlockedaccount`` endpoint.
- Updated Finalization Proof dto for the latest Testnet server changes.
- Added Account restriction search ``/restrictions/account`` endpoint.
- Added get mosaic restriction by composite hash ``/restrictions/mosaic/{compositeHash}`` endpoint.
- Added get secret lock by composite hash ``/lock/secret/{compositeHash}`` endpoint.
- Added get metadata by composite hash ``/metadata/{compositeHash}`` endpoint.
- Added new IPv4 and IPv6 Role Types.
- Added new private Network Types.

### Updated

- Allowing hash lock searches without filters.
- Allowing secret lock searches without filters.
- Updated finalization proof schema added optional ``signatureSchema``.

### Removed

- ``id`` from namespace's meta.

## [0.10.0] - 25-Sep-2020

Targets catapult-rest 2.1.0

### Added

- ``/chain/info`` endpoint (with `latestFinalizedBlock` ).
- ``SecretLock`` and ``HashLock`` endpoints.
- ``fromHeight`` and ``toHeight`` search criteria for ``Transaction`` search.
- new ``Voting`` node role type.

### Removed

- ``/chain/height`` endpoint.
- ``/chain/score`` endpoints.
- ``totalEntries`` and ``totalPages`` from pagination.
- ``MessageDTO`` and ``MessageTypeEnum``.

### Changed

- Type of ``message`` property in transfer transaction to ``string``.
- Property names in ``BlockDTO``.
    1. Changed ``numTransactions`` to ``totalTransactionsCount``.
    2. Changed ``numStatements`` to ``statementsCount``.
    3. Added ``transactionsCount``.

## [0.9.6] - 14-Aug-2020

Targets catapult-rest 1.2.0

### Added

- allowing filtering by multiple receipt types in transaction statement search
- improved doc

## [0.9.5] - 22-Jul-2020

Targets catapult-rest 1.2.0

### Added

- statement pagination endpoints.
- metadata pagination endpoints.
- account pagination endpoints.
- namespace pagination endpoints.

### Removed

- ``/block/{height}/receipts`` endpoint.
- ``/metadata/mosaic/{mosaicId}/*`` endpoints.
- ``/metadata/namespace/{namespaceId}/*`` endpoints.
- ``/metadata/account/{address}/*`` endpoints.

## [0.9.4] - 30-Jun-2020

Targets catapult-rest 1.1.3

### Added

- 0.9.6.2 status errors.
- 0.9.6.2 network parameters.

## [0.9.3] - 25-Jun-2020

Targets catapult-rest 1.1.2

### Added

- Account state accountKeys changed to publicKeys.
- VotingAccountLinkTransaction finalization points.

### Fixed

- Long numbers support for Java code generators.

## [0.9.2] - 18-Jun-2020

Targets catapult-rest 1.1.0

### Added

- 0.9.6.1 status errors.
- 0.9.6.1 network parameters.
- New transactions, blocks, and mosaics paginable endpoints.

### Changed

- Schemas properties uses addresses in favour of public keys.
- Pluralized tranactions, blocks, and mosaics endpoints.
- Address schema has one less byte.
- ``/transaction/:hash`` endpoint becomes ``/transactions/:group/:hash``.
- ``/block/:height/transaction/:hash`` becomes ``/transactions/:group?height=:height``.
- ``/block/:height/limit/:limit`` becomes ``/transactions/:group?pageSize=:pageSize&id=:id&order=block``.
- ``/account/:accountId/transactions/:group`` becomes ``/transactions/:group/?address=:addres``.
- ``/transactions/status`` becomes ``/transactionStatus``.
- ``/transaction/cosignature/`` require to pass a new property ``version``.

## Removed

- POST ``/account/mosaics``.

## [0.8.11] - 21-Mar-2020

Targets catapult-rest 1.0.20.34

### Added

- catapult-server 0.9.5.1 status errors.

### Changed

- AccountKeyLinkTransaction remotePublicKey param renamed to linkedPublickey.

## [0.8.10] - 15-Mar-2020

Targets catapult-rest 1.0.20.34

### Added

- VRFKeyLinkTransaction DTO.
- VotingKeyLinkTransaction DTO.
- NodeKeyLinkTransaction DTO.

### Changed

- AccountLinkTransaction DTOs schema renamed to AccountKeyLink.
- BlockHeader DTO adds VRFProof.
- Rename generationHash to networkGenerationHashSeed.
- catapult-server 0.9.5.1 config parameters.

## [0.8.9] - 19-Mar-2020

Targets catapult-rest 1.0.20.22

### Added

- Automatic release management with TravisCI.

### Changed

- Parameters split in two folders "query" and "parameters".
- Parameters names are now unique.

## [0.8.7] - 09-Mar-2020

Targets catapult-rest 1.0.20.22

### Added

- ``/network/fees/rental`` endpoint.

### Changed

- ``/network/fees`` endpoint moved to ``/network/transaction``.
- Multi query param serialization from CSV to additional query params (e.g ``type=16974&type=16718``).
- TransactionNetworkFeesDTO average and median multipliers are now integers.

### Removed

- Format from integers.

## [0.8.6] - 02-Mar-2020

Targets catapult-rest 1.0.20.20

### Added

- ``/network/properties`` endpoint.

## [0.8.5] - 21-Feb-2020

Targets catapult-rest 1.0.20.19

### Added

- ``node/peers`` endpoint.

### Changed

- ``type`` query params validation with ``TransactionTypeEnum``.
-  Project was renamed to ``symbol-openapi``.

[0.9.5]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.9.4...v0.9.5
[0.9.4]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.9.3...v0.9.4
[0.9.3]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.9.2...v0.9.3
[0.9.2]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.11...v0.9.2
[0.8.11]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.10...v0.8.11
[0.8.10]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.9...v0.8.10
[0.8.9]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.7...v0.8.9
[0.8.7]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.6...v0.8.7
[0.8.6]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.5...v0.8.6
[0.8.5]: https://github.com/nemtech/symbol-openapi/releases/tag/v0.8.5

