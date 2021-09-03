# binary layout for a namespace registration transaction
struct NamespaceRegistrationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_REGISTRATION)

	# [__value__] mosaic rental fee sink public key
	#
	# [size] mosaic rental fee sink public key size
	rental_fee_sink = inline SizePrefixedAddress

	# mosaic rental fee
	rental_fee = Amount

	# [__value__] new namespace name
	#
	# [size] new namespace name size
	name = inline SizePrefixedString

	# size of the parent namespace name
	parent_name_size = uint32

	# parent namespace name
	parent_name = array(int8, parent_name_size) if 0xFFFFFFFF not equals parent_name_size
