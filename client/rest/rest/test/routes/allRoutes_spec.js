const allRoutes = require('../../src/routes/allRoutes');
const test = require('./utils/routeTestUtils');

describe('all routes', () => {
	const registerAll = server => {
		const config = {
			pageSize: { min: 10, max: 100, step: 25 },
			transactionStates: []
		};
		allRoutes.register(server, {}, { config });
	};

	it('registers all get routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('get', routes);

		// Act:
		registerAll(server);

		// Assert:
		test.assert.assertRoutes(routes, [
			'/account/:accountId',
			'/account/:publicKey/transactions',
			'/account/:publicKey/transactions/incoming',
			'/account/:publicKey/transactions/outgoing',
			'/account/:publicKey/transactions/unconfirmed',
			// no custom account transactions routes are registered

			'/block/:height',
			'/block/:height/transactions',
			'/blocks/:height/limit/:limit',

			'/chain/height',
			'/chain/score',

			'/network',

			'/transaction/:transactionId',
			'/transaction/:hash/status',

			'/diagnostic/blocks/:height/limit/:limit',
			'/diagnostic/storage'
		]);
	});

	it('registers all post routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('post', routes);

		// Act:
		registerAll(server);

		// Assert:
		test.assert.assertRoutes(routes, [
			'/account',
			'/transaction',
			'/transaction/statuses'
		]);
	});

	it('registers all put routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('put', routes);

		// Act:
		registerAll(server);

		// Assert:
		test.assert.assertRoutes(routes, [
			'/transaction'
		]);
	});

	it('registers all ws routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('ws', routes);

		// Act:
		registerAll(server);

		// Assert:
		test.assert.assertRoutes(routes, [
			'/ws'
		]);
	});
});
