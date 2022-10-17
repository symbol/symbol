import symbolSdk from '../../src/index.js';

const { symbol } = symbolSdk;

export default () => ([
	{
		type: 'namespace_registration_transaction_v1',
		registrationType: 'root',
		duration: 123n,
		name: 'roger'
	},

	{
		type: 'namespace_registration_transaction_v1',
		registrationType: 'child',
		parentId: symbol.generateNamespaceId('roger'),
		name: 'charlie'
	}
]);
