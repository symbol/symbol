/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const { test } = require('./utils/routeTestUtils');
const allRoutes = require('../../src/routes/allRoutes');

describe('all routes', () => {
	const registerAll = server => {
		const config = {
			pageSize: { min: 10, max: 100 },
			transactionStates: [],
			apiNode: { timeout: 1000 }
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
			'/account/:accountId/harvest',
			'/account/:accountId/beneficiary',

			'/block/:height',
			'/block/:height/transaction/:hash/merkle',
			'/blocks/:height/limit/:limit',
			'/state/:state/hash/:hash/merkle',

			'/chain/height',
			'/chain/score',

			'/network',
			'/network/properties',
			'/network/fees/transaction',
			'/network/fees/rental',
			'/node/health',
			'/node/info',
			'/node/peers',
			'/node/server',
			'/node/storage',
			'/node/time',

			'/transactions/:transactionId',
			'/transactions/:hash/status',
			'/transactions'
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
			'/transactions',
			'/transactions/statuses'
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
			'/transactions'
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
