# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [v1.2.0] - 16-Jul-2020
### Added
- TLS installation notes.

### Changed
- Reviewed hash lock, secret lock, namespace, account, metadata, and receipt endpoints.

### Fixed
- Height comparison towards current chain height, for height related endpoints.

## [v1.1.3] - 27-Jun-2020
### Changed (since v1.0.20.50)
- New project versioning that allows pinning on compatible core server versions and makes it easier to track REST changes.
- Resized addresses from 25 to 24 bytes.

## [0.7.14] - 27-Mar-2019
### Added
-  Receipts, AccountProperties, and AccountLink plugins were activated.

### Changed 
-  Block schema field beneficiaryPublicKey has been renamed to beneficiary.
-  Transaction schema fee field has been renamed to max_fee.
-  createLong function from CatapultDB was moved to a new dbUtils file.

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

[0.7.14]: https://github.com/nemtech/catapult-rest/compare/v0.7.13...v0.7.14
[0.7.13]: https://github.com/nemtech/catapult-rest/compare/v0.7.12...v0.7.13
[0.7.12]: https://github.com/nemtech/catapult-rest/compare/v0.7.11...v0.7.12
[0.7.11]: https://github.com/nemtech/catapult-rest/compare/v0.7.8...v0.7.11
[0.7.8]: https://github.com/nemtech/catapult-rest/compare/v0.7.7...v0.7.8
[0.7.7]: https://github.com/nemtech/catapult-rest/compare/v0.7.5...v0.7.7
[0.7.5]: https://github.com/nemtech/catapult-rest/releases/tag/v0.7.5
