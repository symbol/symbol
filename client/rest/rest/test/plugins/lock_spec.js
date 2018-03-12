const lock = require('../../src/plugins/lock');
const LockDb = require('../../src/plugins/db/LockDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('lock plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(lock, LockDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(lock);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(lock);

	describe('register routes', () => {
		it('registers lock GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			lock.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:accountId/lock/hash',
				'/account/:accountId/lock/secret'
			]);
		});
	});
});
