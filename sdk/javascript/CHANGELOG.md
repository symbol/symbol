# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## next

## [3.2.3] - 04-Dec-2024

### Added
- add toJson() to the generated model classes to get a JSON-safe representation of the model.
- add extractSigningPayload() to facade to get signing payload which allows signing by hardware keys.
- add cosignTransactionHash() to facade and account to allow cosigning of transaction hash.

### Changed
- export bytesToBigInt(), bytesToInt(), deepCompare(), intToBytes(), isHexString() and tryParseUint() from sdk via utils
- (BREAKING NEM) NEM cosignature transaction multisig_transaction_hash field renamed to other_transaction_hash

### Fixed
- (NEM) add non-verifiable cosignature transaction to allow signing of the cosignature transaction.

## [3.2.2] - 28-May-2024

### Added
- generate basic TS-style documentation using typedoc to provide class-level documentation for SDK
- add useful functions from the SDK V2
  - create address alias from namespace id
  - extract namespaceID from address alias
  - create address from decoded address hex string (REST format)
- add metadataGenerateKey() since there is no built-in way to convert a string to a metadata key.
- Added SymbolAccount class
  - add signTransaction() to the SymbolAccount to match facade functionality
  - add cosignTransaction() to the SymbolAccount to match facade functionality 
  - to improve discoverability, add a helper function to SymbolAccount, which creates a MessageEncoder used to encrypt/encode messages
- add SymbolPublicAccount class
  - add properties address and publicKey for better discoverability
- Added NemAccount class
  - add signTransaction() to the NemAccount to match facade functionality
  - to improve discoverability, add a helper function to NemAccount, which creates a MessageEncoder used to encrypt/encode messages
- Add NemPublicAccount class
  - add properties address and publicKey for better discoverability
- add a createPublicAccount() and createAccount() to the facade for both NEM and Symbol
- add deserialize() function to TransactionFactory in SDK for better discoverability

### Changed
- improve typescript support by generating type-annotated descriptors that accept strongly typed arguments
- calculating fee for aggregate transaction is not easy to configure via createTransactionFromTypedDescriptor.  Add cosignatureCount optional parameter to SymbolFacade createTransactionFromTypedDescriptor

### Fixed
- NetworkTimestamp.timestamp is BigInt but models.Timestamp (NEM) is a Number that leads to a mismatch during creation.  Add logic to BaseValue to automatically coerce input of Number|BigInt to the desired underlying type when possible.

## [3.2.1] - 22-Apr-2024

### Fixed
- now() does not return the correct timestamp since bitwise OR operation only works with signed 32-bit integers
- package subpath exports are not working correctly with certain JS/TS environments

## [3.2.0] - 09-Apr-2024

### Added
- add now() to facade for getting current timestamp
- add alternative pure JavaScript implementation for all WASM logic
- add static getter that can be used instead of constructor in TS
- use exports in package.json

### Changed
- split single entry point into default, nem and symbol for better discoverability
- make symbol-crypto-wasm-node an optional dependency

### Fixed
- improve codegen to significantly reduce size of generated models
- use globalThis instead of global for WebAssembly for browser compatibility
- mm-snap requires default export to be last
- update Cipher.js to support ReactNative environment
- update rust code to use dalek 4
- prune customized tweetnacl implementation
- search through nc module to find transaction class name to workaround minification dropping 'constructor.name'

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

[3.2.3]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.2.2...sdk%2Fjavascript%2Fv3.2.3
[3.2.2]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.2.1...sdk%2Fjavascript%2Fv3.2.2
[3.2.1]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.2.0...sdk%2Fjavascript%2Fv3.2.1
[3.2.0]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.1.0...sdk%2Fjavascript%2Fv3.2.0
[3.1.0]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.11...sdk%2Fjavascript%2Fv3.1.0
[3.0.11]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.7...sdk%2Fjavascript%2Fv3.0.11
[3.0.7]: https://github.com/symbol/symbol/compare/sdk%2Fjavascript%2Fv3.0.0...sdk%2Fjavascript%2Fv3.0.7
[3.0.0]: https://github.com/symbol/symbol/releases/tag/sdk%2Fjavascript%2Fv3.0.0
