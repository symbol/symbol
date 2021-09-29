# enumeration of transaction types
enum TransactionType : uint32
	# transfer transaction
	TRANSFER = 0x0101

	# account key link trasaction
	# alternatively called importance transfer transaction
	ACCOUNT_KEY_LINK = 0x0801

	# multisig account modification transaction
	# alternatively called multisig consignatory modification transaction
	MULTISIG_ACCOUNT_MODIFICATION = 0x1001

	# multisig cosignature transaction
	# alternatively called multisig signature transaction
	MULTISIG_COSIGNATURE = 0x1002

	# multisig transaction
	MULTISIG_TRANSACTION = 0x01004

	# namespace registration transaction
	# alternatively called provision namespace transaction
	NAMESPACE_REGISTRATION = 0x2001

	# mosaic definition transaction
	# alternatively called mosaic definition creation transaction
	MOSAIC_DEFINITION = 0x4001

	# mosaic supply change transaction
	MOSAIC_SUPPLY_CHANGE = 0x4002
