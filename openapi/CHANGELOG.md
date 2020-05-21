# CHANGELOG

All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
- Rename generationHash to networkGenerationHashSeed
- catapult-server 0.9.5.1 config parameters.

## [0.8.9] - 19-Mar-2020

Targets catapult-rest 1.0.20.22

### Added

- Automatic release management with TravisCI.

### Changed

- Parameters split in two folders "query" and "parameters"
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
