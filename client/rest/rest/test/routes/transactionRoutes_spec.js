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

const catapult = require('catapult-sdk');
const transactionRoutes = require('../../src/routes/transactionRoutes');
const test = require('./utils/routeTestUtils');

const { convert } = catapult.utils;

describe('transaction routes', () => {
	describe('PUT transaction', () => {
		test.route.packet.addPutPacketRouteTests(transactionRoutes.register, {
			routeName: '/transaction',
			packetType: '9',
			inputs: {
				valid: {
					params: { payload: '123456' },
					parsed: Buffer.of(
						0x0B, 0x00, 0x00, 0x00, // size (header)
						0x09, 0x00, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x56 // payload
					)
				},
				invalid: {
					params: { payload: '1234S6' },
					error: { key: 'payload' }
				}
			}
		});
	});

	describe('get', () => {
		const addGetPostTests = (dbApiName, key, ids, parsedIds) => {
			const errorMessage = 'has an invalid format';
			test.route.document.addGetPostDocumentRouteTests(transactionRoutes.register, {
				routes: { singular: '/transaction/:transactionId', plural: '/transaction' },
				inputs: {
					valid: { object: { transactionId: ids[0] }, parsed: [parsedIds[0]], printable: ids[0] },
					validMultiple: { object: { transactionIds: ids }, parsed: parsedIds },
					invalid: { object: { transactionId: '12345' }, error: `transactionId ${errorMessage}` },
					invalidMultiple: {
						object: { transactionIds: ['12345', ids[0], ids[1]] },
						error: `element in array transactionIds ${errorMessage}`
					}
				},
				dbApiName,
				type: 'transactionWithMetadata'
			});
		};

		const addHomogeneousCheck = (validIds, invalidId) => {
			it('does not support lookup of heterogenous ids', () => {
				// Arrange:
				const keyGroups = [];
				const db = test.setup.createCapturingDb('transactionsByIds', keyGroups, [{ value: 'this is nonsense' }]);

				// Act:
				const registerRoutes = transactionRoutes.register;
				const ids = [validIds[0], validIds[1], invalidId, validIds[2]];
				const errorMessage = 'element in array transactionIds has an invalid format';
				return test.route.executeThrows(
					registerRoutes,
					'/transaction',
					'post',
					{ transactionIds: ids },
					db,
					undefined,
					errorMessage,
					409
				);
			});
		};

		const Valid_Object_Ids = ['00112233445566778899AABB', 'CCDDEEFF0011223344556677', '8899AABBCCDDEEFF00112233'];
		const Valid_Transaction_Hashes = [
			'00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
			'112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00',
			'2233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF0011'
		];

		describe('objectId', () => {
			addGetPostTests('transactionsByIds', 'transactionId', Valid_Object_Ids, Valid_Object_Ids);
			addHomogeneousCheck(Valid_Object_Ids, Valid_Transaction_Hashes[0]);
		});

		describe('transactionHash', () => {
			addGetPostTests(
				'transactionsByHashes',
				'transactionId',
				Valid_Transaction_Hashes,
				Valid_Transaction_Hashes.map(convert.hexToUint8)
			);
			addHomogeneousCheck(Valid_Transaction_Hashes, Valid_Object_Ids[0]);
		});
	});
});
