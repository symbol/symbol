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

import light from '../../../src/plugins/light/light.js';
import test from '../../routes/utils/routeTestUtils.js';
import pluginTest from '../utils/pluginTestUtils.js';

describe('Light REST plugin', () => {
	pluginTest.assertThat.pluginDoesNotCreateDb(light, { shouldForwardDb: true });
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(light);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(light);

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
				'/node/info',
				'/node/server',
				'/node/unlockedaccount'
			]);
		});
	});
});
