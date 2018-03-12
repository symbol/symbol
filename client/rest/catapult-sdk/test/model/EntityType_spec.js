const { expect } = require('chai');
const EntityType = require('../../src/model/EntityType');

describe('entity type enumeration', () => {
	it('exposes expected types', () => {
		// Assert:
		expect(EntityType).to.deep.equal({
			transfer: 0x4154,
			registerNamespace: 0x414E,
			mosaicDefinition: 0x414D,
			mosaicSupplyChange: 0x424D,
			mosaicLevyChange: 0x434D,
			modifyMultisigAccount: 0x4155,
			aggregateComplete: 0x4141,
			aggregateBonded: 0x4241,
			hashLock: 0x414C,
			secretLock: 0x424C,
			secretProof: 0x434C
		});
	});

	it('exposed values are unique', () => {
		// Act:
		const reverseMapping = Object.keys(EntityType).reduce((state, name) => {
			state[EntityType[name]] = name;
			return state;
		}, {});

		// Assert:
		expect(Object.keys(EntityType).length).to.equal(Object.keys(reverseMapping).length);
	});
});
