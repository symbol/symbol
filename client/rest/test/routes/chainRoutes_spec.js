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

const { MockServer, test } = require('./utils/routeTestUtils');
const chainRoutes = require('../../src/routes/chainRoutes');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { Binary, Long } = MongoDb;

describe('chain routes', () => {
	describe('get', () => {
		it('can retrieve info', () => {
			// Arrange:
			const chainStatisticData = {
				height: Long.fromNumber(33),
				scoreHigh: Long.fromNumber(44),
				scoreLow: Long.fromNumber(55)
			};
			const chainStatisticCurrentFake = sinon.fake(() => Promise.resolve(chainStatisticData));
			const finalizedBlockData = {
				height: Long.fromNumber(222),
				hash: new Binary(test.random.hash()),
				finalizationEpoch: 777,
				finalizationPoint: 111
			};
			const latestFinalizedBlockFake = sinon.fake(() => Promise.resolve(latestFinalizedBlockFake));

			const mockServer = new MockServer();
			const db = { chainStatisticCurrent: chainStatisticCurrentFake, latestFinalizedBlock: latestFinalizedBlockFake };
			chainRoutes.register(mockServer.server, db, {});
			const route = mockServer.getRoute('/chain/info').get();

			// Act:
			return mockServer.callRoute(route, { params: {} }).then(() => {
				expect(chainStatisticCurrentFake.calledOnce).to.equal(true);
				expect(latestFinalizedBlockFake.calledOnce).to.equal(true);

				const expectedData = chainStatisticData;
				expectedData.latestFinalizedBlock = finalizedBlockData;
				expect(mockServer.send.firstCall.args[0]).to.deep.equal({
					payload: expectedData,
					type: 'chainInfo'
				});
			});
		});
	});
});
