/**
 * Catapult model entity types.
 * @enum {numeric}
 * @exports model/EntityType
 */
const EntityType = {
	/** Transfer transaction. */
	transfer: 0x4154,

	/** Register namespace transaction. */
	registerNamespace: 0x414E,

	/** Mosaic definition transaction. */
	mosaicDefinition: 0x414D,

	/** Mosaic supply change transaction. */
	mosaicSupplyChange: 0x424D,

	/** Mosaic levy change transaction. */
	mosaicLevyChange: 0x434D,

	/** Modify multisig account transaction. */
	modifyMultisigAccount: 0x4155,

	/** Aggregate complete transaction. */
	aggregateComplete: 0x4141,

	/** Aggregate bonded transaction. */
	aggregateBonded: 0x4241,

	/** Hash lock transaction. */
	hashLock: 0x414C,

	/** Secret lock transaction. */
	secretLock: 0x424C,

	/** Secret proof transaction. */
	secretProof: 0x434C
};

module.exports = EntityType;
