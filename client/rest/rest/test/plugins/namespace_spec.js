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

const namespace = require('../../src/plugins/namespace');
const NamespaceDb = require('../../src/plugins/db/NamespaceDb');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('namespace plugin', () => {
	pluginTest.assertThat.pluginCreatesDb(namespace, NamespaceDb);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalTransactionStates(namespace);
	pluginTest.assertThat.pluginDoesNotRegisterAdditionalMessageChannels(namespace);

	describe('register routes', () => {
		it('registers namespace / mosaic GET routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			namespace.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/:accountId/namespaces',
				'/mosaic/:mosaicId',
				'/namespace/:namespaceId',
				'/namespace/:namespaceId/mosaics'
			]);
		});

		it('registers namespace / mosaic POST routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('post', routes);

			// Act:
			namespace.registerRoutes(server, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/account/namespaces',
				'/mosaic',
				'/mosaic/names',
				'/namespace/names'
			]);
		});
	});
});
