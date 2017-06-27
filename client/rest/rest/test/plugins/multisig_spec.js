import { expect } from 'chai';
import multisig from '../../src/plugins/multisig';
import MultisigDb from '../../src/plugins/db/MultisigDb';
import test from '../routes/utils/routeTestUtils';

describe('multisig plugin', () => {
	describe('create db', () => {
		it('returns multisig db', () => {
			// Act:
			const db = multisig.createDb();

			// Assert:
			expect(db).to.be.instanceOf(MultisigDb);
		});
	});

	describe('register routes', () => {
		it('registers multisig GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			multisig.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/key/:publicKey/multisig'
			]);
		});
	});
});
