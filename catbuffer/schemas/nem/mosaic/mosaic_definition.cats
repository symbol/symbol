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

	# levy mosaic id size
	mosaic_id_size = sizeof(uint32, mosaic_id)

	# levy mosaic id
	mosaic_id = MosaicId

	# amount of levy mosaic to transfer
	fee = Amount

# binary layout for a mosaic property
# supported property names are: divisibility, initialSupply, supplyMutable, transferable
@is_size_implicit
struct MosaicProperty
	# [__value__] property name
	#
	# [size] property name size
	name = inline SizePrefixedString

	# [__value__] property value
	#
	# [size] property value size
	value = inline SizePrefixedString

# binary layout for a size prefixed mosaic property
struct SizePrefixedMosaicProperty
	# property size
	property_size = sizeof(uint32, property)

	# property value
	property = MosaicProperty

# binary layout for a mosaic definition
@is_size_implicit
struct MosaicDefinition
	# [__value__] owner public key
	#
	# [size] owner public key size
	owner_public_key = inline SizePrefixedPublicKey

	# mosaic id size
	id_size = sizeof(uint32, id)

	# mosaic id referenced by this definition
	id = MosaicId

	# [__value__] description
	#
	# [size] description size
	description = inline SizePrefixedString

	# number of properties
	properties_count = uint32

	# properties
	properties = array(SizePrefixedMosaicProperty, properties_count)

	# size of the serialized levy
	@sizeref(levy, 0)
	levy_size = uint32

	# optional levy that is applied to transfers of this mosaic
	levy = MosaicLevy if 0x00000000 not equals levy_size

# shared content between verifiable and non-verifiable mosaic definition transactions
inline struct MosaicDefinitionTransactionBody
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_DEFINITION)

	# mosaic definition size
	mosaic_definition_size = sizeof(uint32, mosaic_definition)

	# mosaic definition
	mosaic_definition = MosaicDefinition

	# [__value__] mosaic rental fee sink public key
	#
	# [size] mosaic rental fee sink public key size
	rental_fee_sink = inline SizePrefixedAddress

	# mosaic rental fee
	rental_fee = Amount

# binary layout for a mosaic definition transaction (V1, latest)
struct MosaicDefinitionTransactionV1
	inline Transaction
	inline MosaicDefinitionTransactionBody

# binary layout for a non-verifiable mosaic definition transaction (V1, latest)
struct NonVerifiableMosaicDefinitionTransactionV1
	inline NonVerifiableTransaction
	inline MosaicDefinitionTransactionBody
