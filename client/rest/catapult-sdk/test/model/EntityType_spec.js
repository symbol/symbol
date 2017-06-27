import { expect } from 'chai';
import EntityType from '../../src/model/EntityType';

describe('entity type enumeration', () => {
	it('exposes expected types', () => {
		// Assert:
		expect(EntityType).to.deep.equal({
			transfer: 0x4101,
			registerNamespace: 0x4201,
			mosaicDefinition: 0x4202,
			mosaicLevyChange: 0x4203,
			mosaicSupplyChange: 0x4204,
			modifyMultisigAccount: 0x4401,
			aggregate: 0x4801
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
