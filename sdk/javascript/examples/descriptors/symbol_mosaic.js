import { Address, generateMosaicId } from '../../src/symbol/index.js';

export default () => {
	const sampleAddress = new Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y');

	return [
		{
			type: 'mosaic_definition_transaction_v1',
			duration: 1n,
			nonce: 123,
			flags: 'transferable restrictable',
			divisibility: 2
		},

		{
			type: 'mosaic_supply_change_transaction_v1',
			mosaicId: generateMosaicId(sampleAddress, 123),
			delta: 1000n * 100n, // assuming divisibility = 2
			action: 'increase'
		}
	];
};
