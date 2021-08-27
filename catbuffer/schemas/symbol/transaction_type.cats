# enumeration of transaction types
enum TransactionType : uint16
	# account key link transaction
	account_key_link = 0x414C

	# node key link transaction
	node_key_link = 0x424C

	# aggregate complete transaction
	aggregate_complete = 0x4141

	# aggregate bonded transaction
	aggregate_bonded = 0x4241

	# voting key link transaction
	voting_key_link = 0x4143

	# vrf key link transaction
	vrf_key_link = 0x4243

	# hash lock transaction
	hash_lock = 0x4148

	# secret lock transaction
	secret_lock = 0x4152

	# secret proof transaction
	secret_proof = 0x4252

	# account metadata transaction
	account_metadata = 0x4144

	# mosaic metadata transaction
	mosaic_metadata = 0x4244

	# namespace metadata transaction
	namespace_metadata = 0x4344

	# mosaic definition transaction
	mosaic_definition = 0x414D

	# mosaic supply change transaction
	mosaic_supply_change = 0x424D

	# multisig account modification transaction
	multisig_account_modification = 0x4155

	# address alias transaction
	address_alias = 0x424E

	# mosaic alias transaction
	mosaic_alias = 0x434E

	# namespace registration transaction
	namespace_registration = 0x414E

	# account address restriction transaction
	account_address_restriction = 0x4150

	# account mosaic restriction transaction
	account_mosaic_restriction = 0x4250

	# account operation restriction transaction
	account_operation_restriction = 0x4350

	# mosaic address restriction transaction
	mosaic_address_restriction = 0x4251

	# mosaic global restriction transaction
	mosaic_global_restriction = 0x4151

	# transfer transaction
	transfer = 0x4154
