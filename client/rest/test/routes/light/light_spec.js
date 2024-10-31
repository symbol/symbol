/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

import light from '../../../src/routes/light/light.js';
import test from '../utils/routeTestUtils.js';
import { expect } from 'chai';

describe('Light REST Route', () => {
	describe('register routes', () => {
		it('registers GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			light.registerRoutes(server, undefined, {
				config: {
					network: {
						name: 'testnet'
					},
					apiNode: {
						host: 'localhost',
						port: 7900,
						timeout: 1000
					}
				}
			});

			// Assert:
			test.assert.assertRoutes(routes, [
				// node route
				'/node/info',
				'/node/peers',
				'/node/server',
				'/node/unlockedaccount',

				// chain route
				'/chain/info'
			]);
		});
	});

	describe('create db', () => {
		it('undefined', () => {
			// Arrange:
			const catapultDb = {};

			// Act:
			const db = light.createDb(catapultDb);

			// Assert:
			expect(db).to.equal(undefined);
		});
	});

	describe('register transaction states', () => {
		it('does not register states', () => {
			// Arrange:
			const states = [];

			// Act:
			light.registerTransactionStates(states);

			// Assert:
			expect(states.length).to.equal(0);
		});
	});

	describe('register message channels', () => {
		it('does not register channels', () => {
			// Arrange:
			let numAddCalls = 0;
			const builder = { add: () => { ++numAddCalls; } };

			// Act:
			light.registerMessageChannels(builder);

			// Assert:
			expect(numAddCalls).to.equal(0);
		});
	});
});
