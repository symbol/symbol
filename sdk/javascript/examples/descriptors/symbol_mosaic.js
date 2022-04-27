const { Address, generateMosaicId } = require('../../src/index').symbol;

const descriptorFactory = () => {
	const sampleAddress = new Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y');

	return [
		{
			type: 'mosaic_definition_transaction',
			duration: 1n,
			nonce: 123,
			flags: 'transferable restrictable',
			divisibility: 2
		},

		{
			type: 'mosaic_supply_change_transaction',
			mosaicId: generateMosaicId(sampleAddress, 123),
			delta: 1000n * 100n, // assuming divisibility = 2
			action: 'increase'
		}
	];
};

module.exports = { descriptorFactory };
