const catapult = require('catapult-sdk');
const test = require('../../../routes/utils/routeTestUtils');

const Valid_Address = test.sets.addresses.valid[0];
const Valid_Public_Key = test.sets.publicKeys.valid[0];

const routeAccountIdGetTestUtils = {
	routeDescriptorFactory: traits =>
		dataTraits => ({
			route: traits.route,
			inputs: {
				valid: {
					object: { accountId: dataTraits.valid },
					parsed: dataTraits.expected,
					printable: dataTraits.valid
				},
				invalid: { object: { accountId: '12345' }, error: 'accountId has an invalid format' }
			},
			dbApiName: traits.dbApiName,
			type: traits.dbType
		}),

	addGetDocumentTests: addTests => {
		const Valid_Address_Traits = {
			valid: Valid_Address,
			expected: ['address', [catapult.model.address.stringToAddress(Valid_Address)]]
		};

		const Valid_Public_Key_Traits = {
			valid: Valid_Public_Key,
			expected: ['publicKey', [catapult.utils.convert.hexToUint8(Valid_Public_Key)]]
		};

		describe('by address', () => addTests(Valid_Address_Traits));
		describe('by publicKey', () => addTests(Valid_Public_Key_Traits));
	},

	addDefaultTests: traits => {
		const createRouteDescriptor = routeAccountIdGetTestUtils.routeDescriptorFactory(traits);
		const addGetTests = dataTraits => {
			const routeDescriptor = createRouteDescriptor(dataTraits);

			// Assert:
			test.route.document.addGetDocumentRouteTests(traits.registerRoutes, routeDescriptor);
		};

		routeAccountIdGetTestUtils.addGetDocumentTests(addGetTests);
	}
};

module.exports = routeAccountIdGetTestUtils;
