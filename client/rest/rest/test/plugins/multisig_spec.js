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

const multisig = require('../../src/plugins/multisig');
const MultisigDb = require('../../src/plugins/db/MultisigDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('multisig plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(multisig, MultisigDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(multisig);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(multisig);

	describe('register routes', () => {
		it('registers multisig GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			multisig.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:accountId/multisig',
				'/account/:accountId/multisig/graph'
			]);
		});
	});
});
