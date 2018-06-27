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

const { expect } = require('chai');
const catapult = require('catapult-sdk');

const { convert } = catapult.utils;
const blockRoutes = require('../../src/routes/blockRoutes');
const test = require('./utils/routeTestUtils');

describe('block routes', () => {
	const addChainInfoToDb = db => { db.chainInfo = () => Promise.resolve({ height: 10 }); };
	const routeConfig = { pageSize: { min: 30, max: 80, step: 12 } };

	describe('block', () => {
		const builder = test.route.document.prepareGetDocumentRouteTests(blockRoutes.register, {
			route: '/block/:height',
			dbApiName: 'blockAtHeight',
			type: 'blockHeaderWithMetadata',
			extendDb: addChainInfoToDb,
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
			extendDb: addChainInfoToDb,
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
						chainInfo: () => Promise.resolve({ height: 10 })
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
		const rd = {
			route: '/block/:height/transaction/:hash/merkle',
			dbApiName: 'blockWithMerkleTreeAtHeight',
			type: 'merkleProofInfo',
			extendDb: addChainInfoToDb,
			config: routeConfig
		};

		const formatHashAsBinary = hash => test.factory.createBinary(Buffer.from(convert.hexToUint8(hash), 'hex'));
		const formatBinaryAsHash = binary => convert.uint8ToHex(binary.buffer);

		const merkleTree = [
			formatHashAsBinary('9922093F19F7160BDCBCA8AA48499DA8DF532D4102745670B85AA4BDF63B8D59'),
			formatHashAsBinary('E8FCFD95CA220D442BE748F5494001A682DC8015A152EBC433222136E99A96B8'),
			formatHashAsBinary('C1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'),
			formatHashAsBinary('F8E8FCDAD1B94D2C76D769B113FF5CAC5D5170772F2D80E466EB04FCA23D6887'),
			formatHashAsBinary('2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391'),
			formatHashAsBinary('AEAF30ED55BBE4805C53E5232D88242F0CF719F99A8E6D339BCBF5D5DE85E1FB'),
			formatHashAsBinary('AFE6C917BABA60ADC1512040CC35033B563DAFD1718FA486AB1F3D9B84866B27')
		];
		const db = test.setup.createCapturingDbWithExtensions(rd, [], {
			meta: {
				numTransactions: 4,
				merkleTree
			}
		});

		it('should return a merkle path if the transaction and block were correct', () => {
			// Arrange:
			const urlParams = {
				height: '10',
				hash: formatBinaryAsHash(merkleTree[2])
			};

			// Act:
			return test.route.executeSingle(blockRoutes.register, rd.route, 'get', urlParams, db, rd.config, response => {
				// Assert:
				expect(response).to.deep.equal({
					payload: {
						merklePath: [
							{ position: 2, hash: merkleTree[3].buffer },
							{ position: 1, hash: merkleTree[4].buffer }
						]
					},
					type: 'merkleProofInfo'
				});
			});
		});

		it('should throw error for invalid block height', () => {
			// Arrange:
			const urlParams = {
				height: '4aa',
				hash: 'A1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'
			};

			// Assert:
			return test.route.executeThrows(
				blockRoutes.register,
				rd.route,
				'get',
				urlParams,
				db,
				rd.config,
				'height has an invalid format',
				409
			);
		});

		it('should throw error for invalid hash format', () => {
			// Arrange:
			const urlParams = {
				height: '1',
				hash: 'E3F4FEAE575D81A7F728F4E3704613AF'
			};

			// Assert:
			return test.route.executeThrows(
				blockRoutes.register,
				rd.route,
				'get',
				urlParams,
				db,
				rd.config,
				'hash has an invalid format',
				409
			);
		});

		it('should throw error if block has no transactions, thus no merkle tree (hash does not belong to block)', () => {
			// Arrange:
			const localDb = test.setup.createCapturingDbWithExtensions(rd, [], {
				meta: {
					numTransactions: 0
				}
			});

			// Assert:
			const urlParams = {
				height: '10',
				hash: 'A1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'
			};

			return test.route.executeSingle(blockRoutes.register, rd.route, 'get', urlParams, localDb, rd.config, response => {
				expect(response.body.code).to.equal('InvalidArgument');
				expect(response.body.message).to.contain('not included in block height');
			});
		});

		it('should throw error if transaction is not found in the block\'s merkle tree', () => {
			// Arrange:
			const urlParams = {
				height: '10',
				hash: 'B1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'
			};

			// Act:
			return test.route.executeSingle(blockRoutes.register, rd.route, 'get', urlParams, db, rd.config, response => {
				// Assert:
				expect(response.body.code).to.equal('ResourceNotFound');
				expect(response.body.message).to.contain('no resource exists with id');
				expect(response.body.message).to.contain(urlParams.hash);
			});
		});

		describe('block with merkle tree', () => {
			const builder = test.route.document.prepareGetDocumentRouteTests(blockRoutes.register, {
				route: '/block/:height/transaction/:hash/merkle',
				dbApiName: 'blockWithMerkleTreeAtHeight',
				type: 'merkleProofInfo',
				extendDb: addChainInfoToDb,
				config: routeConfig
			});
			builder.addNotFoundInputTest({
				object: {
					height: '11',
					hash: 'B1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'
				},
				parsed: [11],
				printable: '11'
			}, 'block height higher than the chain\'s');
		});
	});
});
