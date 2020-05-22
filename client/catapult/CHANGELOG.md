# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [0.9.5.1] - 22-May-2020

### Added
 - major: VRF support, harvesters need to register VRF key that is used to generate VRF proof for given block
 - harvest network fees

### Changed
 - unlock message behavior: message must specify vrf key, account must be linked to a node via NodeKeyLink
 - renamed AccountLinkTransaction to AccountKeyLinkTransaction (consistency)

### Fixed:
 - bugfix: issue #68, delay ChainedSocketReader in PacketReaders
 - bugfix: issue #69, mosaic divisibility validation
 - more minor fixes

### Removed
 - eventsource extension

## [0.9.4.1] - 23-April-2020

### Added
 - major: TLS support, changes are large, please refer to project documentation for details
 - bugfix: namespace extension must preserve root alias
 - bugfix: add timeout around accept (of incoming connection)

## [0.9.3.2] - 19-March-2020

### Changed
 - rocks upgraded to v6.6.4

## [0.9.3.1] - 05-March-2020

### Added
 - forcibly disconnect connections to/from newly banned node identity
 - use network fingerprint (id + generation hash) for nicer network separation
 - missing mongo index on beneficiaryPublicKey

### Changed
 - unit221b pentesting: use ephemeral keys in unlock messages, use HKDF for key derivation
 - unit221b pentesting: include S-part of signature in entity hash, to avoid "invalid tx announce" attack
 - model: moved TransactionStatus.Deadline before Status

### Fixed
 - bugfix: possible race in node selector
 - bugfix: unconfirmed transactions subscriptions

### Removed
 - unit221b pentesting: `SIGNATURE_SCHEME_*` defines, catapult now uses standard ed25519 derivation with sha512
 - in-source implementations of ripemd160, sha256, sha3, aes, use variants provided by openssl

## [0.9.2.1] - 23-January-2020

### Fixed
 - bugfix: incomplete nemesis block data inside mongo
 - bugfix: remove spurious check in CreateCacheBlockTouchObserver
 - bugfix: make unconditional update of activity information based on account balance
 - minor: do not migrate node identity key (when source is worse)
 - minor: reprocessing of aggregates with different set of cosigners

## [0.9.1.1] - 06-December-2019

### Added
 - Make nemesis epoch time configurable
 - Network setting: maxHarvesterBalance

### Fixed
 - Multiple recovery fixes
 - Avoid banning hosts defined as local network
 - Fixed point math based block time smoothing
 - Unlocked accounts duplicate handling
 - Propagate single blocks properly (unused mask)
 - Mongo: save only non-empty account restrictions
 - Mongo: missing transactionHash of aggregate transaction

## [0.9.0.1] - 08-November-2019

### Added
 - Support for `make install`

### Changed
 - Add hash variant name to binary descriptions
 - Realign transaction binary layouts to maximize alignment of fields
 - Add `AggregateTransaction::TransactionHash` that contains merkle hash of component transactions
 - Store `Version` and `Network` directly in `EntityBody`

### Fixed
 - GitHub Issues: #45 #46 #47 #48 #49 #50
 - Bug in harvester `StateHash` calculation when `transactionSelectionStrategy` is not `oldest`
 - Fix remaining UBSAN and TSAN warnings

## [0.8.0.3] - 02-October-2019

### Added
 - Ability for network to identify servers by resolved IP instead of public key
   - require single public key to map to single host at one time
 - Basic DoS protection
   - ban nodes that send transactions that fail stateless validation
   - ban nodes that exceed data threshold
   - close connections that send data unexpectedly
 - Monitoring of static nodes for IP changes

### Changed
 - Add support for clang 9 and gcc 9
 - Update all dependencies to latest versions
 - Final set of naming review changes (mongo, config, results)
 - Fix some UBSAN and TSAN warnings

### Fixed
 - Bug in delegated harvester unlocking message processing
 - Namespace and lock expiry receipt generation
 - Notify cosignatories' public keys in order to allow added cosignatories to get aggregate notifications
 - Potential deadlock in SpinReaderWriterLock

## [0.7.0.1] - 02-September-2019

### Added
 - TrustedHosts filter for diagnostic packet handlers
 - Prohibition of zero-based keys and derived addresses
 - Delegated harvester unlocking (via special encrypted message directed to node owner)
 - Dynamic rental fees

### Changed
 - Naming review changes:
   - catbuffer (models, validators, etc)
   - mongo naming review changes
   - config variable naming
 - Use donna ed25519 implementation, use batched signature verification
 - Use non-reversed private keys in `SIGNATURE_SCHEME_NIS1` - this change will require private keys used in catapult to be un-reversed, when switching from NIS1
 - Generate nemesis block statement
 - Turn coresystem into plugin

### Fixed
 - Bug in harvesting ut facade factory

## [0.6.0.1] - 26-July-2019

### Added
 - Restriction mosaic plugin
 - Metadata plugin
 - Outgoing address account restrictions

### Changed
 - Prevent creation of secret locks that violate restrictions at creation time
 - Add receipt when namespace transitions from active to locked
 - Allow configurable namespace depth and min namespace duration
 - Update AddressValidator to always check network byte

### Fixed
 - Bug in namespace lifetime grace period handling
 - Do not delete deactivated mosaics from state hash
 - Apply fee surplus to activity bucket total fees paid in harvester
 - Synchronize FileTransactionStatusStorage::flush to avoid race condition

## [0.5.0.1] - 28-June-2019

### Added
 - PoS+ consensus mechanism, see #26 for details

### Changed
 - Breaking, all transaction versions are now 1
 - Rename `property` transaction/plugin to `restriction account`
 - Renumber validation result codes
 - Add cmake component groups

### Fixed
 - Deadlock in broker process when small number of threads in thread pool

### Removed
 - Mongo mapping functions from model to dbmodel

## [0.4.0.1] - 31-May-2019

### Added
- New catapult.broker process that pushes changes into MongoDB and and ZMQ.
- New catapult.recovery process that repairs local state after ungraceful termination.
- catapult.server produces file-backed messages that are consumed by catapult.broker.
- Inflation support via new config-inflation.properties.
- Implementation of harvest fee sharing and beneficiary specification.

### Changed
- Prevent transactions from being replayed on different networks by prepending the network generation hash to transaction data prior to signing and verifying.
- Cosignatories must opt-in before being added to a multisig account.
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

[0.9.5.1]: https://github.com/nemtech/catapult-server/compare/v0.9.4.1...v0.9.5.1
[0.9.4.1]: https://github.com/nemtech/catapult-server/compare/v0.9.3.2...v0.9.4.1
[0.9.3.2]: https://github.com/nemtech/catapult-server/compare/v0.9.3.1...v0.9.3.2
[0.9.3.1]: https://github.com/nemtech/catapult-server/compare/v0.9.2.1...v0.9.3.1
[0.9.2.1]: https://github.com/nemtech/catapult-server/compare/v0.9.1.1...v0.9.2.1
[0.9.1.1]: https://github.com/nemtech/catapult-server/compare/v0.9.0.1...v0.9.1.1
[0.9.0.1]: https://github.com/nemtech/catapult-server/compare/v0.8.0.3...v0.9.0.1
[0.8.0.3]: https://github.com/nemtech/catapult-server/compare/v0.7.0.1...v0.8.0.3
[0.7.0.1]: https://github.com/nemtech/catapult-server/compare/v0.6.0.1...v0.7.0.1
[0.6.0.1]: https://github.com/nemtech/catapult-server/compare/v0.5.0.1...v0.6.0.1
[0.5.0.1]: https://github.com/nemtech/catapult-server/compare/v0.4.0.1...v0.5.0.1
[0.4.0.1]: https://github.com/nemtech/catapult-server/compare/v0.3.0.2...v0.4.0.1
[0.3.0.2]: https://github.com/nemtech/catapult-server/compare/v0.3.0.1...v0.3.0.2
[0.3.0.1]: https://github.com/nemtech/catapult-server/compare/v0.2.0.2...v0.3.0.1
[0.2.0.2]: https://github.com/nemtech/catapult-server/compare/v0.1.0.1...v0.2.0.2
[0.1.0.1]: https://github.com/nemtech/catapult-server/releases/tag/v0.1.0.1

