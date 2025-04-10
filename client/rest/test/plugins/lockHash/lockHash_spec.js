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

import LockHashDb from '../../../src/plugins/lockHash/LockHashDb.js';
import lockHash from '../../../src/plugins/lockHash/lockHash.js';
import test from '../../routes/utils/routeTestUtils.js';
import pluginTest from '../utils/pluginTestUtils.js';

describe('lock hash plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(lockHash, LockHashDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(lockHash);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(lockHash);

	describe('register routes', () => {
		it('registers lock hash GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			lockHash.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:address/lock/hash',
				'/lock/hash',
				'/lock/hash/:hash',
				'/lock/hash/:hash/merkle'
			]);
		});

		it('registers POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			lockHash.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/lock/hash'
			]);
		});
	});
});
