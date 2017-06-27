/**
 * Catapult model entity types.
 * @enum {numeric}
 * @exports model/EntityType
 */
const EntityType = {
	/** Transfer transaction. */
	transfer: 0x4101,

	/** Register namespace transaction. */
	registerNamespace: 0x4201,

	/** Mosaic definition transaction. */
	mosaicDefinition: 0x4202,

	/** Mosaic levy change transaction. */
	mosaicLevyChange: 0x4203,

	/** Mosaic supply change transaction. */
	mosaicSupplyChange: 0x4204,

	/** Modify multisig account transaction. */
	modifyMultisigAccount: 0x4401,

	/** Aggregate transaction. */
	aggregate: 0x4801
};

export default EntityType;
