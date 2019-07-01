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

const diagnosticRoutes = require('../../src/routes/diagnosticRoutes');
const { expect } = require('chai');
const { MockServer, test } = require('./utils/routeTestUtils');
const { version: restVersion } = require('../../package.json');
const { version: sdkVersion } = require('../../../catapult-sdk/package.json');

describe('diagnostic routes', () => {
	const executeRoute = (routeName, db, assertResponse) =>
		test.route.executeSingle(diagnosticRoutes.register, routeName, 'get', {}, db, undefined, assertResponse);

	describe('blocks', () => {
		const builder = test.route.document.prepareGetDocumentsRouteTests(diagnosticRoutes.register, {
			route: '/diagnostic/blocks/:height/limit/:limit',
			dbApiName: 'blocksFrom',
			type: 'blockHeaderWithMetadata'
		});

		builder.addValidInputTest({ object: { height: '1234', limit: '4321' }, parsed: [1234, 4321] });
		builder.addEmptyArrayTest({ object: { height: '1234', limit: '4321' }, parsed: [1234, 4321] });

		// notice that this expands to four tests { 'height', 'limit'} x { '10A', '-4321' }
		['height', 'limit'].forEach(property => ['10A', '-4321'].forEach(value => {
			const object = Object.assign({ height: '1234', limit: '4321' }, { [property]: value });
			const errorMessage = `${property} has an invalid format`;
			builder.addInvalidKeyTest({ object, error: errorMessage }, `(${property} with value ${value})`);
		}));
	});

	describe('server info', () => {
		it('can retrieve info', () => {
			// Arrange:
			const endpointUnderTest = '/diagnostic/server';
			const mockServer = new MockServer();
			diagnosticRoutes.register(mockServer.server, {});

			// Act:
			const route = mockServer.routes[endpointUnderTest];
			mockServer.callRoute(route, {});

			// Assert:
			expect(mockServer.send.firstCall.args[0]).to.deep.equal({
				payload: {
					serverInfo: {
						restVersion,
						sdkVersion
					}
				},
				type: 'serverInfo'
			});
		});
	});

	describe('storage', () => {
		const createMockStorageInfoDb = (numBlocks, numTransactions, numAccounts) => ({
			storageInfo: () => Promise.resolve({ numBlocks, numTransactions, numAccounts })
		});

		it('can retrieve info', () => {
			// Arrange:
			const db = createMockStorageInfoDb(2, 64, 9);

			// Act:
			return executeRoute('/diagnostic/storage', db, response => {
				// Assert:
				expect(response).to.deep.equal({
					payload: { numBlocks: 2, numTransactions: 64, numAccounts: 9 },
					type: 'storageInfo'
				});
			});
		});
	});
});
