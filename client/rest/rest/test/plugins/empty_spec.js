import { expect } from 'chai';
import empty from '../../src/plugins/empty';
import test from '../routes/utils/routeTestUtils';

describe('transfer plugin', () => {
	describe('create db', () => {
		it('returns undefined', () => {
			// Act:
			const db = empty.createDb();

			// Assert:
			expect(db).to.equal(undefined);
		});
	});

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
