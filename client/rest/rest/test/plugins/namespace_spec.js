const namespace = require('../../src/plugins/namespace');
const NamespaceDb = require('../../src/plugins/db/NamespaceDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('namespace plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(namespace, NamespaceDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(namespace);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(namespace);

	describe('register routes', () => {
		it('registers namespace / mosaic GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			namespace.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:accountId/namespaces',
				'/mosaic/:mosaicId',
				'/namespace/:namespaceId',
				'/namespace/:namespaceId/mosaics'
			]);
		});

		it('registers namespace / mosaic POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			namespace.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/namespaces',
				'/mosaic',
				'/mosaic/names',
				'/namespace/names'
			]);
		});
	});
});
