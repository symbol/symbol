import "mosaic.cats"
import "transaction.cats"

# enumeration of mosaic transfer fee types
enum MosaicTransferFeeType : uint32
	# fee represents an absolute value
	ABSOLUTE = 0x0001

	# fee is proportional to a percentile of the transferred mosaic
	PERCENTILE = 0x0002

# binary layout for a mosaic levy
struct MosaicLevy
	# mosaic fee type
	transfer_fee_type = MosaicTransferFeeType

	# [__value__] recipient address
	#
	# [size] recipient address size
	recipient_address = inline SizePrefixedAddress

	# levy mosaic
	mosaic_id = MosaicId

	# amount of levy mosaic to transfer
	fee = Amount

# binary layout for a mosaic property
# supported property names are: divisibility, initialSupply, supplyMutable, transferable
struct MosaicProperty
	# [__value__] property name
	#
	# [size] property name size
	name = inline SizePrefixedString

	# [__value__] property value
	#
	# [size] property value size
	value = inline SizePrefixedString

# binary layout for a mosaic definition
struct MosaicDefinition
	# public key of the mosaic definition owner

	# [__value__] owner public key
	#
	# [size] owner public key size
	owner_public_key = inline SizePrefixedPublicKey

	# mosaic id referenced by this definition
	id = MosaicId

	# [__value__] description
	#
	# [size] description size
	description = inline SizePrefixedString

	# number of properties
	properties_count = uint32

	# properties
	properties = array(MosaicProperty, properties_count)

	# size of the serialized levy
	levy_size = uint32

	# optional levy that is applied to transfers of this mosaic
	levy = MosaicLevy if 0xFFFFFFFF not equals levy_size

# binary layout for an importance transfer transaction
struct MosaicDefinitionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_DEFINITION)

	inline Transaction

	# mosaic definition
	mosaic_definition = MosaicDefinition

	# [__value__] mosaic rental fee sink public key
	#
	# [size] mosaic rental fee sink public key size
	rental_fee_sink = inline SizePrefixedAddress

	# mosaic rental fee
	rental_fee = Amount
