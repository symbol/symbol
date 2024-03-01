import { generateNamespaceId } from '../../src/symbol/index.js';

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
		parentId: generateNamespaceId('roger'),
		name: 'charlie'
	}
]);
