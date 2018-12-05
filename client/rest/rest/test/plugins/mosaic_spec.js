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

const mosaic = require('../../src/plugins/mosaic');
const MosaicDb = require('../../src/plugins/db/MosaicDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('mosaic plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(mosaic, MosaicDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(mosaic);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(mosaic);

	describe('register routes', () => {
		it('registers GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			mosaic.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/mosaic/:mosaicId',
			]);
		});

		it('registers POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			mosaic.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/mosaic'
			]);
		});
	});
});
