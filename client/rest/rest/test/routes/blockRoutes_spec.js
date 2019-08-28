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

const { MockServer, test } = require('./utils/routeTestUtils');
const blockRoutes = require('../../src/routes/blockRoutes');
const routeUtils = require('../../src/routes/routeUtils');
const { expect } = require('chai');
const sinon = require('sinon');

describe('block routes', () => {
	const addChainStatisticToDb = db => { db.chainStatistic = () => Promise.resolve({ height: 10 }); };
	const routeConfig = { pageSize: { min: 30, max: 80, step: 12 } };

	describe('block', () => {
		const builder = test.route.document.prepareGetDocumentRouteTests(blockRoutes.register, {
			route: '/block/:height',
			dbApiName: 'blockAtHeight',
			type: 'blockHeaderWithMetadata',
			extendDb: addChainStatisticToDb,
			config: routeConfig
		});
		builder.addDefault({
			valid: { object: { height: '3' }, parsed: [3], printable: '3' },
			invalid: { object: { height: '10A' }, error: 'height has an invalid format' }
		});
		builder.addNotFoundInputTest({ object: { height: '11' }, parsed: [11], printable: '11' }, 'chain height is too small');
	});

	describe('blocks from height', () => {
		const builder = test.route.document.prepareGetDocumentsRouteTests(blockRoutes.register, {
			route: '/blocks/:height/limit/:limit',
			dbApiName: 'blocksFrom',
			type: 'blockHeaderWithMetadata',
			config: routeConfig
		});

		builder.addValidInputTest({ object: { height: '1', limit: '36' }, parsed: [1, 36] }, '(default limit)');
		builder.addValidInputTest({ object: { height: '3601', limit: '72' }, parsed: [3601, 72] }, '(custom valid limit)');
		builder.addEmptyArrayTest({ object: { height: '1', limit: '36' }, parsed: [1, 36] });

		// notice that this expands to four tests { 'height', 'limit'} x { '10A', '-4321' }
		['height', 'limit'].forEach(property => ['10A', '-4321'].forEach(value => {
			const object = Object.assign({ height: '1234', limit: '4321' }, { [property]: value });
			const errorMessage = `${property} has an invalid format`;
			builder.addInvalidKeyTest({ object, error: errorMessage }, `(${property} with value ${value})`);
		}));

		builder.addRedirectTest({ object: { height: '0', limit: '0' }, redirectUri: '/blocks/1/limit/36' }, '{ height: 0, limit: 0 }');

		// invalid limit is mapped to 36 (first valid limit)
		builder.addRedirectTest(
			{ object: { height: '3601', limit: '80' }, redirectUri: '/blocks/3601/limit/36' },
			'{ height: 3601, limit: 80 }'
		);

		// invalid height is mapped to multiple of limit
		builder.addRedirectTest(
			{ object: { height: '125', limit: '60' }, redirectUri: '/blocks/121/limit/60' },
			'{ height: 125, limit: 60 }'
		);
		builder.addRedirectTest(
			{ object: { height: '360', limit: '60' }, redirectUri: '/blocks/301/limit/60' },
			'{ height: 360, limit: 60 }'
		);
		builder.addRedirectTest(
			{ object: { height: '362', limit: '60' }, redirectUri: '/blocks/361/limit/60' },
			'{ height: 362, limit: 60 }'
		);
	});

	describe('block transactions', () => {
		const builder = test.route.document.prepareGetDocumentsRouteTests(blockRoutes.register, {
			route: '/block/:height/transactions',
			dbApiName: 'transactionsAtHeight',
			type: 'transactionWithMetadata',
			extendDb: addChainStatisticToDb,
			config: routeConfig
		});

		builder.addValidInputTest({ object: { height: '3' }, parsed: [3, undefined, 0], printable: '3' });
		builder.addEmptyArrayTest({ object: { height: '3' }, parsed: [3, undefined, 0], printable: '3' });
		builder.addNotFoundInputTest(
			{ object: { height: '11' }, parsed: [11, undefined, 0], printable: '11' },
			'chain height is too small'
		);
		builder.addInvalidKeyTest({ object: { height: '10A' }, error: 'height has an invalid format' });

		describe('paging', () => {
			const pagingTestsFactory = test.setup.createPagingTestsFactory(
				{
					routes: blockRoutes,
					routeName: '/block/:height/transactions',
					createDb: (queriedIdentifiers, transactions) => ({
						transactionsAtHeight: (height, pageId, pageSize) => {
							queriedIdentifiers.push({ height, pageId, pageSize });
							return Promise.resolve(transactions);
						},
						chainStatistic: () => Promise.resolve({ height: 10 })
					}),
					config: routeConfig
				},
				{ height: '3' },
				{ height: 3 },
				'transactionWithMetadata'
			);

			pagingTestsFactory.addDefault();
			pagingTestsFactory.addNonPagingParamFailureTest('height', '-1');
		});
	});

	describe('block with merkle tree', () => {
		it('calls blockRouteMerkleProcessor with correct params', () => {
			// Arrange:
			const mockServer = new MockServer();
			const blockRouteMerkleProcessorSpy = sinon.spy(routeUtils, 'blockRouteMerkleProcessor');

			// Act:
			blockRoutes.register(mockServer.server, {}, { config: routeConfig });

			// Assert:
			expect(blockRouteMerkleProcessorSpy.calledOnce).to.equal(true);
			expect(blockRouteMerkleProcessorSpy.firstCall.args[1]).to.equal('numTransactions');
			expect(blockRouteMerkleProcessorSpy.firstCall.args[2]).to.equal('transactionMerkleTree');
			blockRouteMerkleProcessorSpy.restore();
		});
	});
});
