using MosaicNonce = uint32

# Enumeration of mosaic property flags.
@is_bitwise
enum MosaicFlags : uint8
	# No flags present.
	NONE = 0x00

	# Mosaic supports supply changes through a MosaicSupplyChangeTransaction even
	# when mosaic creator only owns a partial supply.
	#
	# If the mosaic creator owns the totality of the supply, it can be changed even
	# if this flag is not set.
	SUPPLY_MUTABLE = 0x01

	# Mosaic supports TransferTransaction between arbitrary accounts.
	# When not set, this mosaic can only be transferred to or from the mosaic creator.
	TRANSFERABLE = 0x02

	# Mosaic supports custom restrictions configured by the mosaic creator.
	#
	# See MosaicAddressRestrictionTransaction and MosaicGlobalRestrictionTransaction.
	RESTRICTABLE = 0x04

	# Mosaic supports revocation of tokens by the mosaic creator.
	REVOKABLE = 0x08

# Enumeration of mosaic supply change actions.
enum MosaicSupplyChangeAction : uint8
	# Decreases the supply.
	DECREASE = 0x00

	# Increases the supply.
	INCREASE = 0x01
