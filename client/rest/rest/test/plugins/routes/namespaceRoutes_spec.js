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
const namespaceRoutes = require('../../../src/plugins/routes/namespaceRoutes');
const test = require('../../routes/utils/routeTestUtils');

const { convert } = catapult.utils;
const { address } = catapult.model;

describe('namespace routes', () => {
	describe('get by id', () => {
		const namespaceId = '1234567890ABCDEF';
		test.route.document.addGetDocumentRouteTests(namespaceRoutes.register, {
			route: '/namespace/:namespaceId',
			inputs: {
				valid: { object: { namespaceId }, parsed: [[0x90ABCDEF, 0x12345678]], printable: namespaceId },
				invalid: {
					object: { namespaceId: '12345' },
					error: 'namespaceId has an invalid format'
				}
			},
			dbApiName: 'namespaceById',
			type: 'namespaceDescriptor'
		});
	});

	describe('get by owner', () => {
		const factory = {
			createNamespacePagingRouteInfo: (routeName, routeCaptureMethod) => ({
				routes: namespaceRoutes,
				routeName,
				createDb: (keyGroups, documents) => ({
					namespacesByOwners: (type, accountIds, pageId, pageSize) => {
						keyGroups.push({
							type,
							accountIds,
							pageId,
							pageSize
						});
						return Promise.resolve(documents);
					}
				}),
				routeCaptureMethod
			})
		};

		const { addresses, publicKeys } = test.sets;

		describe('get by account', () => {
			const addGetTests = traits => {
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					factory.createNamespacePagingRouteInfo('/account/:accountId/namespaces', 'get'),
					traits.valid.params,
					traits.valid.expected,
					'namespaceDescriptor'
				);

				pagingTestsFactory.addDefault();
				pagingTestsFactory.addNonPagingParamFailureTest('accountId', traits.invalid.accountId);
			};

			describe('by address', () => addGetTests({
				valid: {
					params: { accountId: addresses.valid[0] },
					expected: { type: 'address', accountIds: [address.stringToAddress(addresses.valid[0])] }
				},
				invalid: {
					accountId: addresses.invalid,
					error: 'illegal base32 character 1'
				}
			}));

			describe('by publicKey', () => addGetTests({
				valid: {
					params: { accountId: publicKeys.valid[0] },
					expected: { type: 'publicKey', accountIds: [convert.hexToUint8(publicKeys.valid[0])] }
				},
				invalid: {
					accountId: publicKeys.invalid,
					error: 'unrecognized hex char \'1G\''
				}
			}));
		});

		describe('POST', () => {
			const addPostTests = traits => {
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					factory.createNamespacePagingRouteInfo('/account/namespaces', 'post'),
					traits.valid.params,
					traits.valid.expected,
					'namespaceDescriptor'
				);

				pagingTestsFactory.addDefault();
				traits.invalid.forEach(invalid => pagingTestsFactory.addFailureTest(invalid.name, invalid.params, invalid.error));
			};

			describe('by address', () => addPostTests({
				valid: {
					params: { addresses: addresses.valid },
					expected: { type: 'address', accountIds: addresses.valid.map(addr => address.stringToAddress(addr)) }
				},
				invalid: [
					{
						name: 'element in array is invalid (address)',
						params: { addresses: [addresses.valid[0], addresses.invalid, addresses.valid[1]] },
						error: 'element in array addresses has an invalid format'
					},
					{
						name: 'element in array is invalid (accountId)',
						params: { addresses: [addresses.valid[0], '12345', addresses.valid[1]] },
						error: 'element in array addresses has an invalid format'
					}
				]
			}));

			describe('by publicKey', () => addPostTests({
				valid: {
					params: { publicKeys: publicKeys.valid },
					expected: { type: 'publicKey', accountIds: publicKeys.valid.map(publicKey => convert.hexToUint8(publicKey)) }
				},
				invalid: [
					{
						name: 'element in array is invalid (publicKey)',
						params: { publicKeys: [publicKeys.valid[0], publicKeys.invalid, publicKeys.valid[1]] },
						error: 'element in array publicKeys has an invalid format'
					},
					{
						name: 'element in array is invalid (accountId)',
						params: { publicKeys: [publicKeys.valid[0], '12345', publicKeys.valid[1]] },
						error: 'element in array publicKeys has an invalid format'
					}
				]
			}));

			it('does not support publicKeys and addresses provided at the same time', () => {
				// Arrange:
				const keyGroups = [];
				const db = test.setup.createCapturingDb('namespaceById', keyGroups, [{ value: 'this is nonsense' }]);

				// Act:
				const registerRoutes = namespaceRoutes.register;
				const errorMessage = 'publicKeys and addresses cannot both be provided';
				return test.route.executeThrows(
					registerRoutes,
					'/account/namespaces',
					'post',
					{ addresses: addresses.valid, publicKeys: publicKeys.valid },
					db,
					{ transactionStates: [] },
					errorMessage,
					409
				);
			});
		});
	});

	describe('get namespace names by ids', () => {
		const createParentId = parentId => ({ high_: 0, low_: parentId });

		const createNamespace = (parentId, namespaceId) => ({
			// 1. in db, parentId is only stored for child namespaces
			// 2. db returns null instead of undefined when a document property is not present
			parentId: undefined === parentId ? null : createParentId(parentId),
			namespaceId: [0, namespaceId]
		});

		const Valid_Hex_String_Namespace_Ids = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const Valid_Uint64_Namespace_Ids = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];

		const runTest = options => {
			// Arrange:
			const dbParamTuples = [];
			const db = (() => {
				let level = 0;
				return {
					catapultDb: {
						findNamesByIds: (ids, transactionType, fieldsDescriptor) => {
							dbParamTuples.push({ ids, transactionType, fieldsDescriptor });
							return Promise.resolve(options.dbEntitiesGroupedByLevel[level++]);
						}
					}
				};
			})();

			// Act:
			return test.route.executeSingle(
				namespaceRoutes.register,
				'/namespace/names',
				'post',
				{ namespaceIds: Valid_Hex_String_Namespace_Ids },
				db,
				undefined,
				response => {
					// Assert: parameters passed to db function are correct
					let level = 0;
					expect(dbParamTuples.length).to.equal(options.expectedNumDbQueries);
					dbParamTuples.forEach(dbParamTuple => {
						expect(dbParamTuple.ids).to.deep.equal(options.queryIdsGroupedByLevel[level++]);
						expect(dbParamTuple.transactionType).to.deep.equal(catapult.model.EntityType.registerNamespace);
						expect(dbParamTuple.fieldsDescriptor).to.deep.equal({ id: 'namespaceId', name: 'name', parentId: 'parentId' });
					});

					// check response
					let expectedPayload = [];
					options.dbEntitiesGroupedByLevel.forEach(dbEntities => {
						expectedPayload = expectedPayload.concat(dbEntities);
					});

					expect(response).to.deep.equal({ payload: expectedPayload, type: 'namespaceNameTuple' });
				}
			);
		};

		it('returns empty array if no names are found', () => runTest({
			queryIdsGroupedByLevel: [
				Valid_Uint64_Namespace_Ids
			],
			dbEntitiesGroupedByLevel: [
				[]
			],
			expectedNumDbQueries: 1
		}));

		it('returns namespace names if found', () => runTest({
			queryIdsGroupedByLevel: [
				Valid_Uint64_Namespace_Ids
			],
			dbEntitiesGroupedByLevel: [
				[createNamespace(undefined, 9), createNamespace(undefined, 5), createNamespace(undefined, 7)]
			],
			expectedNumDbQueries: 1
		}));

		it('returns level1 (one ancestor) namespace names if found', () => runTest({
			queryIdsGroupedByLevel: [
				Valid_Uint64_Namespace_Ids,
				[createParentId(12), createParentId(0), createParentId(16)]
			],
			dbEntitiesGroupedByLevel: [
				[createNamespace(undefined, 9), createNamespace(12, 5), createNamespace(0, 3), createNamespace(16, 7)],
				[createNamespace(undefined, 12), createNamespace(undefined, 0), createNamespace(undefined, 16)]
			],
			expectedNumDbQueries: 2
		}));

		it('returns level2 (two ancestor) namespace names if found', () => runTest({
			queryIdsGroupedByLevel: [
				Valid_Uint64_Namespace_Ids,
				[createParentId(17), createParentId(12), createParentId(16)],
				[createParentId(25), createParentId(25)]
			],
			dbEntitiesGroupedByLevel: [
				[createNamespace(17, 9), createNamespace(12, 5), createNamespace(16, 7)],
				[createNamespace(undefined, 12), createNamespace(25, 16), createNamespace(25, 17)],
				[createNamespace(undefined, 25), createNamespace(undefined, 25)]
			],
			expectedNumDbQueries: 3
		}));
	});
});
