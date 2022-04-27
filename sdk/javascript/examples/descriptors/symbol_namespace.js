const { generateNamespaceId } = require('../../src/index').symbol;

const descriptorFactory = () => ([
	{
		type: 'namespace_registration_transaction',
		registrationType: 'root',
		duration: 123n,
		name: 'roger'
	},

	{
		type: 'namespace_registration_transaction',
		registrationType: 'child',
		parentId: generateNamespaceId('roger'),
		name: 'charlie'
	}
]);

module.exports = { descriptorFactory };
