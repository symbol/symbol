# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [0.4.0.1] - 31-May-2019

### Added
- New catapult.broker process that pushes changes into MongoDB and and ZMQ.
- New catapult.recovery process that repairs local state after ungraceful termination.
- catapult.server produces file-backed messages that are consumed by catapult.broker.
- Inflation support via new config-inflation.properties.
- Implementation of harvest fee sharing and beneficiary specification.

### Changed
- Prevent transactions from being replayed on different networks by prepending the network generation hash to transaction data prior to signing and verifying.
- Cosigners must opt-in before being added to a multisig account.
- Allow use of same secret with different recipients by adding Recipient to SecretLockTransaction.
- HashLockTransaction Mosaic supports aliases to to the currency mosaic.
- Allow aggregate bonded transaction lifetime to be configured independently of other transactions.

### Fixed
- Bugs in rollback causing potential fork.
- Bugs in state hash calculation causing potential fork.
- Bugs in receipts hash calculation causing potential fork.

### Removed
- Mosaic levy references.

## [0.3.0.2] - 27-Feb-2019

### Changed
- Upgrade boost from 1.64.0 to 1.69.0 and related changes around asio usage.
- Shift from c++14 to c++17.
- Optimized harvester performance.

### Fixed
- Fix harvester crash, CacheHeightView was not thread safe.

## [0.3.0.1] - 08-Feb-2019

### Added
- Receipts record any state-dependent change not observable from the block header or transaction data. The block header stores the root hash of all the receipts linked to the block.
- You can now attach namespaces and subnamespaces to addresses or mosaics using the AliasTransaction. A namespace can only be the alias of one account or mosaic at a time.
- Remote harvesting has been enabled. Remote harvesting enables an account to use a proxy private key that can be shared with a node securely to calculate new blocks.
- The base mosaic for currency is now configurable. This means other mosaic different than xem can be defined in private networks.
- Properties FeeMultiplier and BeneficiaryPublicKey were added to the block header.
- Transaction fee property was renamed to MaxFee.
- Dynamic fee handling - Transactions specify max fee and blocks set fee multiplier, which determines actual fee paid.
- Node reputation handling - When interacting with other nodes, each node will gather data, which will influence future partner node selection probability.

### Changed
- Mosaics are no longer tied to namespaces. This means a mosaic is no longer identified by the namespace. Instead, the identifier is a random uint64.
- Mosaics can be set to not expire. This means there is no need to renew them. However, namespaces have to be renewed after a given count of blocks.
- MosaicDefinitionTransaction comes with an additional 32-bit field called nonce. The nonce is a random value used to generate the mosaic ID.
- SecretLockTransaction now supports Op_Sha3_256, Op_Keccak_256, Op_Hash_160, Op_Hash_256 hash algorithms.

## [0.2.0.2] - 02-Nov-2018

### Added
- Accounts can be configured to receive transactions only from an allowed list of addresses. Similarly, an account can specify a list of addresses that it doesn’t want to receive transactions from. The same behavior can be applied to allow or block transactions containing a given mosaic id or only allowing sending certain transactions by type.
- Block header now stores the state root hash. Light clients will therefore be able to verify the state in a certain block rapidly.
- Nodes can store chain state in a RocksDB when configured. This is useful for networks with a large number of accounts, as it demands less memory from network nodes.

### Changed
- Lock plugin has been divided into two (lock hash and lock secret).

### Fixed
- Minor crash and bug fixes detected during the Catapult Developer Preview.

## [0.1.0.1] - 14-May-2018
### Added
- Initial code release.


[0.4.0.1]: https://github.com/nemtech/catapult-server/compare/v0.3.0.2...v0.4.0.1
[0.3.0.2]: https://github.com/nemtech/catapult-server/compare/v0.3.0.1...v0.3.0.2
[0.3.0.1]: https://github.com/nemtech/catapult-server/compare/v0.2.0.2...v0.3.0.1
[0.2.0.2]: https://github.com/nemtech/catapult-server/compare/v0.1.0.1...v0.2.0.2
[0.1.0.1]: https://github.com/nemtech/catapult-server/releases/tag/v0.1.0.1
