const { expect } = require('chai');
const chainRoutes = require('../../src/routes/chainRoutes');
const test = require('./utils/routeTestUtils');

describe('chain routes', () => {
	const executeRoute = (routeName, db, assertResponse) =>
		test.route.executeSingle(chainRoutes.register, routeName, 'get', {}, db, undefined, assertResponse);

	describe('get', () => {
		const createMockChainInfoDb = (height, scoreLow, scoreHigh) => ({
			chainInfo: () => Promise.resolve({ height, scoreLow, scoreHigh })
		});

		it('can retrieve height', () => {
			// Arrange:
			const db = createMockChainInfoDb(2, 64, 9);

			// Act:
			return executeRoute('/chain/height', db, response => {
				// Assert:
				expect(response).to.deep.equal({ payload: { height: 2 }, type: 'chainInfo' });
			});
		});

		it('can retrieve score', () => {
			// Arrange:
			const db = createMockChainInfoDb(2, 64, 9);

			// Act:
			return executeRoute('/chain/score', db, response => {
				// Assert:
				expect(response).to.deep.equal({ payload: { scoreLow: 64, scoreHigh: 9 }, type: 'chainInfo' });
			});
		});
	});
});
