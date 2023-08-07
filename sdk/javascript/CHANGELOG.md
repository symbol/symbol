# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## next

## [3.1.0] - 7-Aug-2023

### Changed
 - Sync major and minor version with Python SDK

## [3.0.11] - 27-Jul-2023

### Added
 - TypeScript support via JSDoc documentation
 - lookupTransactionName for generating friendly transaction name from transaction type and version
 - (Symbol-only) special handling for encrypted messages created by Symbol wallets

### Fixed
 - (NEM-only) rename TransactionType enum value MULTISIG_TRANSACTION to MULTISIG

## [3.0.7] - 27-Apr-2023

### Changed
 - Network
   - toDatetime and fromDatetime promoted from NetworkTimestamp to Network
   - NetworkTimestamp moved into Network
   - Add epochTime to network description
   - Adjust testnet epoch time to match sai network
 - MerkleHashBuilder.py renamed to Merkle.py
 - Facade
   - Can be created around Network instance or name
 - Sign / verify canonical signature checks
 - TransactionFactory supports auto sorting transaction properties
 - Bip32.random for generating random mnemonic

### Added
 - isValidAddressString to Network for checking validity of an unparsed address
 - MessageEncoder for encrypting and decrypting messages
   - AesCbcCipher (NEM only) and AesGcmCipher implementations
   - SharedKey256 BaseArray derived type
 - Facade
   - SharedKey type
   - bip32Path function for returning BIP32 compatible path
   - (Symbol-only) cosignTransaction for cosigning Symbol transactions
 - Symbol
   - Functions for verifying Merkle proofs and Merkle patricia proofs
   - Utility function metadataUpdateValue for simplifying update of metadata values
 - Proper handling of catbuffer computed fields/properties
 - (NEM-only) Automatic population for fields levySize and messageEnvelopeSize
 - Wasm crypto packages for node and browser

### Fixed
 - (NEM-only) Add NonVerifiableMultisigTransactionV1 model required for signing 'multisig_transaction_v1'

## [3.0.0] - 02-Mar-2022

### Changed
 - complete SDK rewrite, see details in [readme](README.md)

[3.1.0]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.11...sdk%2Fjavascript%2Fv3.1.0
[3.0.11]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.7...sdk%2Fjavascript%2Fv3.0.11
[3.0.7]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.0...sdk%2Fjavascript%2Fv3.0.7
[3.0.0]: https://github.com/symbol/symbol/releases/tag/sdk%2Fjavascript%2Fv3.0.0
