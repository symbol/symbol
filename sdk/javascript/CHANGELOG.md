# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## next

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

[3.0.7]: https://github.com/symbol/sdk-python/compare/v3.0.0...v3.0.7
[3.0.0]: https://github.com/symbol/sdk-python/releases/tag/v3.0.0
