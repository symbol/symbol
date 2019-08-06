import "state/mosaic_restriction_types.cats"

# binary layout for a mosaic restriction serializer
struct MosaicAddressRestrictionEntry

    # id of the entity being restricted
	mosaicId = MosaicId

    # restriction address
	restrictionAddress = Address

    # key value set
	keyPairs = KeyValueSet


# binary layout for a mosaic restriction serializer
struct MosaicGlobalRestrictionEntry

    # id of the entity being restricted
	mosaicId = MosaicId

    # global key value restriction type pairs
	keyPairs = GlobalKeyValueSet


# binary layout for a mosaic restriction serializer
struct MosaicRestrictionEntry

    # type of restriction being place upon the entity
	entryType = EntryType

    # global mosaic rule
	globalEntry = MosaicGlobalRestrictionEntry if entryType equals global

    # address restriction rule
	addressEntry = MosaicAddressRestrictionEntry if entryType equals address
