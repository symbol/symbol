# enumeration of transaction types
enum TransactionType : uint16
	# account key link transaction
	ACCOUNT_KEY_LINK = 0x414C

	# node key link transaction
	NODE_KEY_LINK = 0x424C

	# aggregate complete transaction
	AGGREGATE_COMPLETE = 0x4141

	# aggregate bonded transaction
	AGGREGATE_BONDED = 0x4241

	# voting key link transaction
	VOTING_KEY_LINK = 0x4143

	# vrf key link transaction
	VRF_KEY_LINK = 0x4243

	# hash lock transaction
	HASH_LOCK = 0x4148

	# secret lock transaction
	SECRET_LOCK = 0x4152

	# secret proof transaction
	SECRET_PROOF = 0x4252

	# account metadata transaction
	ACCOUNT_METADATA = 0x4144

	# mosaic metadata transaction
	MOSAIC_METADATA = 0x4244

	# namespace metadata transaction
	NAMESPACE_METADATA = 0x4344

	# mosaic definition transaction
	MOSAIC_DEFINITION = 0x414D

	# mosaic supply change transaction
	MOSAIC_SUPPLY_CHANGE = 0x424D

	# multisig account modification transaction
	MULTISIG_ACCOUNT_MODIFICATION = 0x4155

	# address alias transaction
	ADDRESS_ALIAS = 0x424E

	# mosaic alias transaction
	MOSAIC_ALIAS = 0x434E

	# namespace registration transaction
	NAMESPACE_REGISTRATION = 0x414E

	# account address restriction transaction
	ACCOUNT_ADDRESS_RESTRICTION = 0x4150

	# account mosaic restriction transaction
	ACCOUNT_MOSAIC_RESTRICTION = 0x4250

	# account operation restriction transaction
	ACCOUNT_OPERATION_RESTRICTION = 0x4350

	# mosaic address restriction transaction
	MOSAIC_ADDRESS_RESTRICTION = 0x4251

	# mosaic global restriction transaction
	MOSAIC_GLOBAL_RESTRICTION = 0x4151

	# transfer transaction
	TRANSFER = 0x4154
