import allRoutes from '../../src/routes/allRoutes';
import test from './utils/routeTestUtils';

describe('all routes', () => {
	it('registers all get routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('get', routes);

		// Act:
		allRoutes.register(server, {});

		// Assert:
		test.assert.assertRoutes(routes, [
			'/account/address/:address',
			'/account/key/:publicKey',
			'/account/key/:publicKey/transactions',
			'/account/key/:publicKey/transactions/incoming',
			'/account/key/:publicKey/transactions/outgoing',
			'/account/key/:publicKey/transactions/unconfirmed',
			'/block/height/:height',
			'/block/height/:height/transactions',
			'/blocks/from/:height/group/:grouping',
			'/chain/height',
			'/chain/score',
			'/transaction/id/:id',
			'/diagnostic/storage',
			'/diagnostic/blocks/:height/count/:count'
		]);
	});

	it('registers all put routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('put', routes);

		// Act:
		allRoutes.register(server, {});

		// Assert:
		test.assert.assertRoutes(routes, [
			'/transaction/send'
		]);
	});

	it('registers all ws routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('ws', routes);

		// Act:
		allRoutes.register(server, {});

		// Assert:
		test.assert.assertRoutes(routes, [
			'/ws/block'
		]);
	});
});
