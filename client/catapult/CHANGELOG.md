# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.3.7] - 04-Apr-2024

### Changed
 - update catapult dependencies
 - expose configuration to allow node operator to manage RocksDB log files

### Fixed
 - (BREAKING FORK) fix secret lock pruning bug that, in certain circumstances, can lead to duplicate locks being added

## [1.0.3.6] - 17-Mar-2023

### Changed
 - require connections to always present certificate (`ssl::verify_fail_if_no_peer_cert` #463)
 - MosaicSupplyRevocation was not marking source address as affected by the transaction (#480)
 - AddResolvedAddresses is not properly applying resolutions from previous transactions (#607)
   - This requires a database resync for transactions created before this fix.

## [1.0.3.5] - 09-Nov-2022

### Changed
- update to use openssl 3.0.7

## [1.0.3.4] - 21-Oct-2022

### Fixed
 - (BREAKING FORK) register aggregate transaction hash validator and require aggregate version 2 after fork

## [1.0.3.3] - 15-Mar-2022

### Changed
 - use openssl 1.1.1n

## [1.0.3.1] - 15-Nov-2021

### Added
 - update addressgen to generate mnemonics too

### Changed
 - revert Cyprus fork-specific changes

## [1.0.3.0] - 05-Nov-2021

### Added
 - new MosaicFlag called Revokable. Inspired by [this idea](https://github.com/symbol/NIP/issues/57)
 - new MosaicSupplyRevocationTransaction.

### Fixed
 - removes previous touchpoints of centralization. 

## [1.0.2.0] - 14-Sep-2021

### Added
 - support for Mac M1 builds

### Fixed
 - (BREAKING FORK) voting statistics in ImportanceBlockFooter
 - total voting weight calculation in FinalizationContext
 - crash when voting node runs out of voting keys

### Changed
 - (BREAKING FORK) debit fee after processing custom transaction notifications
 - backup voting messages sent by client for diagnostic and recovery purposes

## [1.0.1.0] - 26-May-2021

### Fixed
 - recover delegated harvesters on resync, #173
 - reapply transactions oldest first when resolving fork
 - fix reentrancy failures in diagnostics build
 - server should make files writeable when copying seed, #168
 - rollback bug leading to crashes in testnet

### Changed
 - add Failure_Link_Start_Epoch_Invalid when voting link is attempted with zero StartEpoch, #170
 - upgrade dependencies to latest versions
 - gcc 11 support

## [1.0.0.0] - 12-Mar-2021

Mainnet launch.

## [0.10.0.8] - 26-Feb-2021

### Added
 - sdk: basic support for BIP32 and BIP39

### Fixed
 - fix deadlock between timesync and node selection, #162
 - addressgen - fix matching logic when no substrings match

## [0.10.0.7] - 15-Feb-2021

### Added
 - tool: importer to populate database from block files
 - tool: verify to check the validity of blocks before import

### Fixed
 - breaking: fix overflow in CalculateTransactionFee, #151
 - recovery process needs to drop orphaned documents associated with previous block, #155
 - check linked public key (remote) instead of main public key, #142
 - minor issues: #153, #158, #159

### Changed
 - allow storing multiple payloads per file in FileDatabase to reduce inode usage, #152

## [0.10.0.6] - 02-Feb-2021

### Added
 - mongo: new config-database settings writeTimeout and maxDropBatchSize
 - new config-node setting: maxTimeBehindPullTransactionsStart - delay transaction pulls and processing of pushes until node is close to being synced
 - allow remote links in linker tool

### Fixed
 - local remote harvesting account should not require node link #142
 - bypass MaxTransactionValidator for nemesis block
 - prevent resolution statements from being created during chain undo #148

### Changed
 - breaking: split address tool into two separate tools: tool.addressgen (vanity generator) and tool.address (converter)

## [0.10.0.5] - 14-Jan-2021

### Fixed
 - deep rollback
   - insert gaps into AccountState stacks to fix deep rollback bug, #121
   - prune newer account histories during rollback, #120
   - write importance files to importance/wip and add commit step, #119
 - importance block processing
   - guarantee HighValueAccountCommitObserver is executed AFTER all other state-changing observers, #118
   - fix calculation of VotingEligibleAccountsCount to only include currently eligible accounts
   - fix potential deadlock when harvesting importance block, #137
 - fix infinite loop in CompareChains when remote returns less than configured hashes, #126

### Changed
 - update boost to version 75
 - pull finalization proofs more aggressively when unfinalizedBlocksDuration is 0
 - UT and PT handling
   - add MinDeadline filter to PT and UT requests
   - only propagate valid PTs and UTs
   - punish stateful transaction failures
   - add configuration to ban nodes that send a lot of bad transactions
 - memory enhancements
   - change PT and UT limits from count to size
   - add memory limits on dispatcher queues
   - (mongo) cap size of transactionStatuses collection, #135

## [0.10.0.4] - 04-Dec-2020

### Added
 - ipv6 support #63
 - versions to all state primary documents in mongo #113 #115
 - clang 11.0.1 support
 - gcc 10.2.0 support

### Fixed
 - finalization fork resolution #102
 - add ListenInterface #55
 - resolve confirmed transaction addresses #82
 - recovery finalization support (EnableRevoteOnBoot), #90
 - check node version when adding to node container #97
 - allow non-voting nodes to pull finalization proofs more aggressively #99
 - fix mongo indexes
 - non-voting node with higher importances does not get rolled back when a fork is resolved #108
 - reduce allocations
   - in PatriciaTree
   - using custom memory pool with OpenSSL
 - Trail-of-Bits: cosmetic changes
 - minor issues: #93, #96, #98, #100, #101, #104, #105, #116

### Changed
 - change voting key tree into key list
 - change voting key link transaction, to use shorter keys
 - mongo namespace meta.active -> meta.latest
 - upgrade dependencies to latest versions
 - drop use of boost::filesystem and boost::thread_group
 - add extended importance blocks to allow trustless verification of finalization proofs #103
 - only serialize non-empty AccountRestrictions #114

## [0.10.0.3] - 25-Sep-2020

### Fixed
 - bugfix: importance files should not be spooled to disk when delta is detached

### Changed
- removed phantom message.type from transfer transaction mongo mapping
- apply maxTransactionsPerBlock against embedded transactions in addition to top-level transactions
- Trail-of-Bits: set and check directory permissions
- Trail-of-Bits: strip RPATHs from built modules by default

## [0.10.0.2] - 22-Sep-2020

### Fixed
 - bugfix: validation of nemesis block containing transactions dependent on DynamicFeeMultiplier
 - bugfix: fix rollback when importance information is spooled to disk
 - bugfix: allow same finalization proof to be received from proof and message sync
 - bugfix: make time based (hash cache) pruning finalization aware
 - reduce network traffic by requesting finalization messages within range
 - optimize compare chains logic to do binary search of remote hashes

## [0.10.0.1] - 18-Sep-2020

### Added
 - deterministic finalization extension based on GRANDPA!

### Changed
 - #85, validate nemesis block on server boot
 - tie voting keys to finalization epochs instead of finalization points
 - spool importance information to disk when finalization is enabled in order to allow deep rollbacks
 - make all state { lock hash, lock secret, namespace } independent of `maxRollbackBlocks` setting
 - change chain comparision to allow syncing when finalization has stalled
 - change "chain info" references to "chain statistics" everywhere
 - refactor crypto code to allow different types of signature schemes for transaction processing and voting
 - generalize one-time signatures tree into a tree that supports reusable (short-term) keys

### Fixed
 - bugfix: #84, fix block generation in `maximize-fee` mode

## [0.9.6.4] - 27-Jul-2020

### Changed
 - allow nemesis block to contain balance transfers from non-nemesis account
 - nemgen enhancements to support public network

### Fixed
 - bugfix: credit main account not remote when hash lock expires
 - bugfix, Trail-of-Bits: UB in container access
 - bugfix, Trail-of-Bits: missing `O_CLOEXEC` flag
 - bugfix: high value addresses tracking

## [0.9.6.3] - 10-Jul-2020

### Fixed
 - bugfix: mosaics inside Balances not correctly ordered after removing optimizedId in 0.9.6.2

## [0.9.6.2] - 23-Jun-2020

### Added
 - one-time signatures tree

### Changed
 - voting key link transaction requires finalization points
 - allow `maxVotingKeysPerAccount` voting key links
 - track voter-eligible accounts
 - state entries indexed by address
 - change the way pruning works (move to BlockChainSyncConsumer)
 - minor: add VerifiableEntity::Size to database
 - minor: binary address format has 24-bytes

### Fixed
 - nodes cannot update identity keys in host-identity network
 - recovery crash in reapplyBlocks caused by inconsistent BlockStatisticCache contents

## [0.9.5.1] - 22-May-2020

### Added
 - major: VRF support, harvesters need to register VRF key that is used to generate VRF proof for given block
 - harvest network fees

### Changed
 - unlock message behavior: message must specify vrf key, account must be linked to a node via NodeKeyLink
 - renamed AccountLinkTransaction to AccountKeyLinkTransaction (consistency)

### Fixed
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
 - Naming review changes
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

[1.0.3.7]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.6...client%2Fcatapult%2Fv1.0.3.7
[1.0.3.6]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.5...client%2Fcatapult%2Fv1.0.3.6
[1.0.3.5]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.4...client%2Fcatapult%2Fv1.0.3.5
[1.0.3.4]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.3...client%2Fcatapult%2Fv1.0.3.4
[1.0.3.3]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.1...client%2Fcatapult%2Fv1.0.3.3
[1.0.3.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.3.0...client%2Fcatapult%2Fv1.0.3.1
[1.0.3.0]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.2.0...client%2Fcatapult%2Fv1.0.3.0
[1.0.2.0]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.1.0...client%2Fcatapult%2Fv1.0.2.0
[1.0.1.0]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv1.0.0.0...client%2Fcatapult%2Fv1.0.1.0
[0.10.0.8]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.7...client%2Fcatapult%2Fv0.10.0.8
[0.10.0.7]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.6...client%2Fcatapult%2Fv0.10.0.7
[0.10.0.6]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.5...client%2Fcatapult%2Fv0.10.0.6
[0.10.0.5]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.4...client%2Fcatapult%2Fv0.10.0.5
[0.10.0.4]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.3...client%2Fcatapult%2Fv0.10.0.4
[0.10.0.3]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.2...client%2Fcatapult%2Fv0.10.0.3
[0.10.0.2]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.10.0.1...client%2Fcatapult%2Fv0.10.0.2
[0.10.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.6.4...client%2Fcatapult%2Fv0.10.0.1
[0.9.6.4]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.6.3...client%2Fcatapult%2Fv0.9.6.4
[0.9.6.3]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.6.2...client%2Fcatapult%2Fv0.9.6.3
[0.9.6.2]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.5.1...client%2Fcatapult%2Fv0.9.6.2
[0.9.5.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.4.1...client%2Fcatapult%2Fv0.9.5.1
[0.9.4.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.3.2...client%2Fcatapult%2Fv0.9.4.1
[0.9.3.2]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.3.1...client%2Fcatapult%2Fv0.9.3.2
[0.9.3.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.2.1...client%2Fcatapult%2Fv0.9.3.1
[0.9.2.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.1.1...client%2Fcatapult%2Fv0.9.2.1
[0.9.1.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.9.0.1...client%2Fcatapult%2Fv0.9.1.1
[0.9.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.8.0.3...client%2Fcatapult%2Fv0.9.0.1
[0.8.0.3]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.7.0.1...client%2Fcatapult%2Fv0.8.0.3
[0.7.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.6.0.1...client%2Fcatapult%2Fv0.7.0.1
[0.6.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.5.0.1...client%2Fcatapult%2Fv0.6.0.1
[0.5.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.4.0.1...client%2Fcatapult%2Fv0.5.0.1
[0.4.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.3.0.2...client%2Fcatapult%2Fv0.4.0.1
[0.3.0.2]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.3.0.1...client%2Fcatapult%2Fv0.3.0.2
[0.3.0.1]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.2.0.2...client%2Fcatapult%2Fv0.3.0.1
[0.2.0.2]: https://github.com/symbol/symbol/compare/client%2Fcatapult%2Fv0.1.0.1...client%2Fcatapult%2Fv0.2.0.2
[0.1.0.1]: https://github.com/symbol/symbol/releases/tag/client%2Fcatapult%2Fv0.1.0.1
