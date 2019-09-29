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

const mosaicRestrictions = require('../../../src/plugins/mosaicRestrictions/mosaicRestrictions');
const MosaicRestrictionsDb = require('../../../src/plugins/mosaicRestrictions/MosaicRestrictionsDb');
const { test } = require('../../routes/utils/routeTestUtils');
const pluginTest = require('../utils/pluginTestUtils');

describe('account restrictions plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(mosaicRestrictions, MosaicRestrictionsDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(mosaicRestrictions);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(mosaicRestrictions);

	describe('register routes', () => {
		it('registers mosaic restrictions GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			mosaicRestrictions.registerRoutes(server, {}, { network: { name: 'mijinTest' } });

			// Assert:
			test.assert.assertRoutes(routes, [
				'/restrictions/mosaic/:mosaicId',
				'/restrictions/mosaic/:mosaicId/address/:accountId'
			]);
		});

		it('registers mosaic restrictions POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			mosaicRestrictions.registerRoutes(server, {}, { network: { name: 'mijinTest' } });

			// Assert:
			test.assert.assertRoutes(routes, [
				'/restrictions/mosaic',
				'/restrictions/mosaic/:mosaicId'
			]);
		});
	});
});
