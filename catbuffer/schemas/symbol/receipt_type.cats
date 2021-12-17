# Enumeration of receipt types.
enum ReceiptType : uint16
	# Mosaic rental fee receipt.
	MOSAIC_RENTAL_FEE = 0x124D

	# Namespace rental fee receipt.
	NAMESPACE_RENTAL_FEE = 0x134E

	# Harvest fee receipt.
	HARVEST_FEE = 0x2143

	# Hash lock completed receipt.
	LOCK_HASH_COMPLETED = 0x2248

	# Hash lock expired receipt.
	LOCK_HASH_EXPIRED = 0x2348

	# Secret lock completed receipt.
	LOCK_SECRET_COMPLETED = 0x2252

	# Secret lock expired receipt.
	LOCK_SECRET_EXPIRED = 0x2352

	# Hash lock created receipt.
	LOCK_HASH_CREATED = 0x3148

	# Secret lock created receipt.
	LOCK_SECRET_CREATED = 0x3152

	# Mosaic expired receipt.
	MOSAIC_EXPIRED = 0x414D

	# Namespace expired receipt.
	NAMESPACE_EXPIRED = 0x414E

	# Namespace deleted receipt.
	NAMESPACE_DELETED = 0x424E

	# Inflation receipt.
	INFLATION = 0x5143

	# Transaction group receipt.
	TRANSACTION_GROUP = 0xE143

	# Address alias resolution receipt.
	ADDRESS_ALIAS_RESOLUTION = 0xF143

	# Mosaic alias resolution receipt.
	MOSAIC_ALIAS_RESOLUTION = 0xF243
