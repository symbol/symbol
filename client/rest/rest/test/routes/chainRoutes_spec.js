import { expect } from 'chai';
import chainRoutes from '../../src/routes/chainRoutes';
import test from './utils/routeTestUtils';

describe('chain routes', () => {
	function executeRoute(routeName, db, assertResponse) {
		return test.route.executeSingle(chainRoutes.register, routeName, 'get', {}, db, assertResponse);
	}

	describe('get', () => {
		function createMockChainInfoDb(height, scoreLow, scoreHigh) {
			return {
				chainInfo: () => Promise.resolve({ height, scoreLow, scoreHigh })
			};
		}

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
