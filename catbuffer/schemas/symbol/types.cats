# A quantity of mosaics in [absolute units](/concepts/mosaic.html#divisibility).
#
# It can only be positive or zero. Negative quantities must be indicated by other means
# (See for example MosaicSupplyChangeTransaction and MosaicSupplyChangeAction).
using Amount = uint64

# A time lapse, expressed in number of blocks.
using BlockDuration = uint64

# Multiplier applied to the size of a transaction to obtain its fee, in
# [absolute units](/concepts/mosaic.html#divisibility).
#
# See the [fees documentation](/concepts/fees.html).
using BlockFeeMultiplier = uint32

# How hard it was to harvest this block.
#
# The initial value is 1e14 and it will remain like this as long as blocks are generated every
# `blockGenerationTargetTime` seconds
# ([network property](/guides/network/configuring-network-properties.html)).
#
# If blocks start taking more or less time than the configured value, the difficulty will be adjusted
# (in the range of 1e13 to 1e15) to try to hit the target time.
#
# See the [Technical Reference](/symbol-technicalref/main.pdf) section 8.1.
using Difficulty = uint64

# Index of a [finalization](/concepts/block.html#finalization) epoch.
#
# The first epoch is number 1 and contains only the first block
# (the [Nemesis](/concepts/block.html#block-creation) block).
# Epoch duration (in blocks) is defined by the `votingSetGrouping` network property.
using FinalizationEpoch = uint32

# A particular point in time inside a [finalization](/concepts/block.html#finalization) epoch.
#
# See the [Technical Reference](/symbol-technicalref/main.pdf) section 15.2.
using FinalizationPoint = uint32

# Index of a block in the blockchain.
#
# The first block (the [Nemesis](/concepts/block.html#block-creation) block)
# has height 1 and each subsequent block increases height by 1.
using Height = uint64

# [Importance score](/concepts/consensus-algorithm.html#importance-score) for an account.
#
# See also ImportanceHeight and ImportanceSnapshot.
using Importance = uint64

# Block height at which an Importance was calculated.
using ImportanceHeight = uint64

# Either a MosaicId or a NamespaceId.
#
# The **most**-significant bit of the first byte is 0 for MosaicId's and 1 for NamespaceId's.
using UnresolvedMosaicId = uint64

# A [Mosaic](/concepts/mosaic.html) identifier.
using MosaicId = uint64

# Number of milliseconds elapsed since the creation of the
# [Nemesis](/concepts/block.html#block-creation) block.
#
# The Nemesis block creation time can be found in the `epochAdjustment` field returned by
# the [/network/properties](/symbol-openapi/v1.0.1/#operation/getNetworkProperties)
# REST endpoint. This is the number of seconds elapsed since the [UNIX epoch](https://en.wikipedia.org/wiki/Unix_time)
# and it is always 1615853185 for Symbol's MAINNET.
using Timestamp = uint64

# Either an Address or a NamespaceId.
#
# The **least**-significant bit of the first byte is 0 for Addresses and 1 for NamespaceId's.
using UnresolvedAddress = binary_fixed(24)

# An [address](/concepts/cryptography.html#address)
# identifies an account and is derived from its PublicKey.
using Address = binary_fixed(24)

# A 32-byte (256 bit) hash.
#
# The exact algorithm is unspecified as it can change depending on where it is used.
using Hash256 = binary_fixed(32)

# A 64-byte (512 bit) hash.
#
# The exact algorithm is unspecified as it can change depending on where it is used.
using Hash512 = binary_fixed(64)

#  A 32-byte (256 bit) integer derived from a private key.
#
# It serves as the public identifier of the [key pair](/concepts/cryptography.html#key-pair)
# and can be disseminated widely. It is used to prove that an entity was signed with the paired private key.
using PublicKey = binary_fixed(32)

# A PublicKey used for voting during the
# [finalization process](/concepts/block.html#finalization).
using VotingPublicKey = binary_fixed(32)

# A 64-byte (512 bit) array certifying that the signed data has not been modified.
#
# Symbol currently uses [Ed25519](https://ed25519.cr.yp.to/) signatures.
using Signature = binary_fixed(64)

# A quantity of a certain mosaic.
struct Mosaic
	# Mosaic identifier.
	mosaic_id = MosaicId

	# Mosaic amount.
	amount = Amount

# A quantity of a certain mosaic, specified either through a MosaicId or an alias.
struct UnresolvedMosaic
	# Unresolved mosaic identifier.
	mosaic_id = UnresolvedMosaicId

	# Mosaic amount.
	amount = Amount

# Link actions.
enum LinkAction : uint8
	# Unlink an account.
	UNLINK = 0x00

	# Link an account.
	LINK = 0x01
