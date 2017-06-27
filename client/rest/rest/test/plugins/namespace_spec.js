import { expect } from 'chai';
import namespace from '../../src/plugins/namespace';
import NamespaceDb from '../../src/plugins/db/NamespaceDb';
import test from '../routes/utils/routeTestUtils';

describe('namespace plugin', () => {
	describe('create db', () => {
		it('returns namespace db', () => {
			// Act:
			const db = namespace.createDb();

			// Assert:
			expect(db).to.be.instanceOf(NamespaceDb);
		});
	});

	describe('register routes', () => {
		it('registers namespace / mosaic GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			namespace.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/mosaic/id/:id',
				'/namespace/:namespaceId/mosaics',
				'/namespace/id/:id',
				'/account/key/:publicKey/namespaces'
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
				'/mosaics/ids',
				'/names/mosaic/ids',
				'/names/namespace/ids'
			]);
		});
	});
});
