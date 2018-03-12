const empty = require('../../src/plugins/empty');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('transfer plugin', () => {
	pluginTest.assertThat.pluginDoesNotCreateDb(empty);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(empty);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(empty);

	describe('register routes', () => {
		it('does not register routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			empty.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, []);
		});
	});
});
