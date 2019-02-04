import "property/propertytypes.cats"
import "transaction.cats"

# account properties mosaic modification
struct MosaicPropertyModification
	inline PropertyModification
	value = MosaicId

# binary layout for an account properties mosaic transaction
struct MosaicPropertyTransactionBody
	# property type
	propertyType = PropertyType

	# number of modifications
	modificationsCount = uint8

	# property modifications
	modifications = array(MosaicPropertyModification, modificationsCount)

# binary layout for a non-embedded account properties mosaic transaction
struct MosaicPropertyTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4152

	inline Transaction
	inline MosaicPropertyTransactionBody

# binary layout for an embedded account properties mosaic transaction
struct EmbeddedMosaicPropertyTransaction
	inline EmbeddedTransaction
	inline MosaicPropertyTransactionBody
