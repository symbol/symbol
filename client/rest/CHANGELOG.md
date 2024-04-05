# Changelog

All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [v2.4.4] - 05-Apr-2024

### Added

- Support to filter blocks by timestamp
- Metal decode support

### Changed

- Upgrade to MongoDb 6.x
- Update rest image to Nodejs 18

## [v2.4.3] - 06-Apr-2023

### Fixed

- Mongodb driver 4.x, fix mongodb shown as down in /node/health
- Reduced the Rest Gateway docker image size
- Exclude protocol treasury from the circulating supply
- Report MongoDB database size in /node/storage
- Report network inflation schedule in /network/inflation
- Report inflation reward at height in /network/inflation/at/:height

## [v2.4.2] - 26-Oct-2022

### Fixed

- Replace yarn with npm
- Remove spammer and tools subpackages
- Add nodeMetadata object to config that is served up over /node/metadata
- Replace js-sha3 with noble
- Cache parsed server configuration objects

## [v2.4.0] - 16-Nov-2021

### Fixed

- Fixed total, circulating and max supply.
- Remove cmc plugin and move `cmcRoutes` into `mosaic:supplyRoutes`.

## [v2.3.9] - 8-Nov-2021

### Added

- Added mosaic supply revocation transaction support.
- Added new catapult 1.0.3.0 new status codes.

## [v2.3.8] - 1-Nov-2021

### Added

- Removed private network type support.
- Renamed network names to from `public` to `mainnet` and from `publicTest` to `testnet`.

## [v2.3.7] - 31-Oct-2021

### Added

- Native HTTPS support by providing SSL certificates.
- `timestamp` in recipients payload.
- `timestamp` and `feeMultipler`in transactions payloads.

### Fixed

- Removed babel from local `catapult-sdk` module.
- Docker image migrated to ubuntu v20.04.

## [v2.3.6] - 24-May-2021

### Added

- Tokenomics endpoints.
- Symbol-bootstrap version in server version endpoints. This requires node operator to update rest.json configuration file with the correct bootstrap version details.

### Fixed

- Fixed zero median network transaction fee issue

## [v2.3.5] - 13-Mar-2021

### Updated

- Updated catapult-server (v1.0.0.0) status codes.

## [v2.3.4] - 5-Mar-2021

### Fixed

- Account endpoint pagination issue.

### Added

- Max zmq socket connection cap.

## [v2.3.3] - 15-Feb-2021

### Fixed

- Fixed duplicate alias names returned from namespace endpoint issue.
- Fixed duplicated status results. The status is now retrieved by priority.

### Changed

- Set default connection pool size to 10.
- Changed `node/storage` endpoint to use estimated counts from mongo collections.
- Allow all transaction groups to be queried by cosigners.
- Allow address aliases in web socket listener subscriptions.

## [v2.3.2] - 02-Feb-2021

### Added

- `FromHeight` and `ToHeight` to receipt search endpoint.

### Fixed

- Fixed issues on only multisig and aggregate initiator can query partial transactions.

## [v2.3.1] - 19-Jan-2021

### Fixed

- Fixed aggregate transaction codec issue.

**We are fixing the [multisig cosignature nofitifcation issue](https://github.com/nemtech/catapult-rest/issues/278) which will be included in the next release.

## [v2.3.0] - 14-Jan-2021

### Added

- `minFeeMultiplier` from the current connected Node in transaction fees endpoint
- Added new `nodePropertyFilePath` in `rest.json`.

### Updated

- Moved `config-network.properties` path config from `network` to `apiNode`.
- Updated `PrivateTest` network type from `0x80` to `0xA8`.
- Re tracked to catapult-server `main` branches.

## [v2.2.1] - 9-Dec-2020

### Fixed

- Fixed v1 voting key issue in finalization route.

## [v2.2.2] - 31-Dec-2020

### Updated

- Voting key v1 to track mainnet (catapult-server).

## [v2.2.0] - 8-Dec-2020

### Added

- Catapult-server finality (0.10.0.4 - testnet/v3) support.
- Added state model versions.
- Added state merkle proof endpoints (`/merkle`) which returns deserialized merkle-patricia tree.
- Added multi-version transaction support.
- Added Symbol Bootstrap support for development and unit testing purposes.
- Added V1 and V2 voting key link transaction support.

### Updated

- Updated `latest` flag in namespace's collection and endpoints.
- Updated finality proof endpoints to match the latest Testnet changes.
- Updated `bmTreeSignature` schema with only `top` and `bottom` levels.
- Updated server status codes.
- Simplified search endpoints.

### Fixed

- Fixed `transferMosaicId`, `fromHeight` and `toHeight` transaction search filters.

## [v2.1.0] - 25-Sept-2020

### Added

- Added `totalTransactionsCount` to the block meta.

### Changed

- Removed `type` interpretation from the transfer transaction messages.
- Renamed `numTransactions` and `numStatements` to `transactionsCount` and `statementsCount` in the block meta.

## [v2.0.0] - 21-Sept-2020

### Added

- New `fromHeight` and `toHeight` filters to the transaction endpoints.
- Added finalization information (latest finalized block) to the new `/chain/info` endpoint.
- New WS channel subscription available: `finalizedBlock`.
- Added endpoints to get finalization proof information by height and epoch.

### Changed

- Reviewed account and mosaic restrictions endpoints.
- Updated voting key link's finalization points to be epochs instead.
- Merged the old `/chain/height` and `/chain/score` endpoints into `/chain/info`.

### Fixed

- Added missing `level` field from the multisig graph endpoint.

## [v1.3.1] - 5-Sept-2020

### Fixed

- Fixed empty node health and server endpoints.

## [v1.3.0] - 1-Sept-2020

### Added

- Added a config option to set the mongodb connection pool size.

### Fixed

- Greatly improved paginated endpoints performance.

### Changed

- Removed `totalEntries` and `totalPages` from paginated results.

## [v1.2.1] - 21-Aug-2020

### Changed

- Removed `topic` (address) from transactionStatus WS responses.
- Removed `channelName` from WS transaction metadata.
- Wrapped WS responses so that the `topic` the client subscribed to is also returned.

## [v1.2.0] - 6-Aug-2020

### Added

- TLS installation notes.
- Dockerfile.
- Automatic Travis to DockerHub releases.
- Transaction statements endpoint now accepts multiple filtered `receitpType`s.

### Changed

- Reviewed hash lock, secret lock, namespace, account, metadata, and receipt endpoints.
- Renamed the `master` branch to `main`.

### Fixed

- Height comparison towards current chain height, for height related endpoints.
- Pagination offsets sometimes were being ignored.
- `namespaces/names` endpoint did not work for some provided addresses.

## [v1.1.3] - 27-Jun-2020

### Changed (since v1.0.20.50)

- New project versioning that allows pinning on compatible core server versions and makes it easier to track REST changes.
- Resized addresses from 25 to 24 bytes.

## [0.7.14] - 27-Mar-2019

### Added

- Receipts, AccountProperties, and AccountLink plugins were activated.

### Changed 

- Block schema field beneficiaryPublicKey has been renamed to beneficiary.
- Transaction schema fee field has been renamed to max_fee.
- createLong function from CatapultDB was moved to a new dbUtils file.

## [0.7.13] - 27-Feb-2019

### Changed

- LockHash secret hash from sha-512 to sha-256.

### Fixed

- Merkle tree endpoint does not return a server error when the transaction is inside the block.

### Removed

- Active condition from meta in mosaic mongodb collection.

## [0.7.12] - 27-Feb-2019

### Added

- Fallback when formatting receipts of unknown type.

### Changed

-  Namespace schema to add the new alias field.

## [0.7.11] - 8-Feb-2019

### Added

- Plugin to handle AccountPropertiesTransaction.
- Plugin to support AliasTransaction.
- Plugin to handle AccountLinkTransaction.
- Plugin to handle Receipts.
- Endpoint to retrieve the account properties associated with an account.
- Endpoint to get the receipts associated with a block.
- Endpoints returning transactions now allow defining if the results should be ordered in ascending or descending order (id).

### Changed

- Enumeration types for hashLock, secretLock, accountPropertiesMosaic, and accountPropertiesEntityType to match the catapult-server 0.2 entity types.
- Split mosaics from namespaces to match catapult-server 0.3.
- Status error codes  (status.js) to match catapult-server 0.3.
- Refactor of variables and file names.

### Removed

-  merkleRootHash field from the block schema.

## [0.7.8] - 3-Aug-2018

### Added

- Endpoint to get an audit path for a transaction Merkle tree.
- Endpoints to get node info and node time.

### Changed

- zeromq dependency is used instead of zmq.

## [0.7.7] - 17-May-2018

### Added
- Basic code coverage support (via nyc + coveralls) .

### Removed

- Script to increment the SDK version (incrementSdkVersion.py).

## [0.7.5] - 17-May-2018

### Added

- Initial code release.

[2.4.4]: https://github.com/symbol/symbol/compare/rest%2Fv2.4.3...rest%2Fv2.4.4
[2.4.3]: https://github.com/symbol/symbol/releases/tag/rest%2Fv2.4.3
