const { expect } = require('chai');
const ModelType = require('../../src/model/ModelType');

describe('model type enumeration', () => {
	it('exposes expected types', () => {
		// Assert:
		expect(ModelType).to.deep.equal({
			none: 0,
			object: 1,
			array: 2,
			dictionary: 3,
			binary: 4,
			uint64: 5,
			objectId: 6,
			string: 7,
			statusCode: 8,
			max: 8
		});
	});
});
