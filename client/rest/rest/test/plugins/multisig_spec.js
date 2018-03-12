const multisig = require('../../src/plugins/multisig');
const MultisigDb = require('../../src/plugins/db/MultisigDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('multisig plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(multisig, MultisigDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(multisig);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(multisig);

	describe('register routes', () => {
		it('registers multisig GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			multisig.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:accountId/multisig',
				'/account/:accountId/multisig/graph'
			]);
		});
	});
});
