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

const chainRoutes = require('../../src/routes/chainRoutes');
const test = require('./utils/routeTestUtils');
const { expect } = require('chai');

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
