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

import rosetta from '../../../src/plugins/rosetta/rosetta.js';
import test from '../../routes/utils/routeTestUtils.js';
import pluginTest from '../utils/pluginTestUtils.js';

describe('rosetta plugin', () => {
	pluginTest.assertThat.pluginDoesNotCreateDb(rosetta, { shouldForwardDb: true });
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(rosetta);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(rosetta);

	describe('register routes', () => {
		it('registers POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			rosetta.registerRoutes(server, undefined, {
				config: {
					network: { name: 'testnet' },
					rest: { protocol: 'http', port: '3456' }
				}
			});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/construction/derive',
				'/construction/preprocess',
				'/construction/metadata'
			]);
		});
	});
});