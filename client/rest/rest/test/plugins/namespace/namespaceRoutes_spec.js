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

const AccountType = require('../../../src/plugins/AccountType');
const { convertToLong } = require('../../../src/db/dbUtils');
const namespaceRoutes = require('../../../src/plugins/namespace/namespaceRoutes');
const namespaceUtils = require('../../../src/plugins/namespace/namespaceUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { Binary } = MongoDb;
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

	describe('by owner', () => {
		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
		const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
		const nonExistingPublicKey = '75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C';
		const nonExistingTestAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';

		const ownedNamespaceSample = {
			meta: {
				id: ''
			},
			namespace: {
				ownerPublicKey: '',
				ownerAddress: ''
			}
		};

		const dbNamespacesByOwnersFake = sinon.fake((type, accountIds) => {
			if (Buffer.from(accountIds[0]).equals(Buffer.from(address.stringToAddress(nonExistingTestAddress)))
				|| Buffer.from(accountIds[0]).equals(Buffer.from(convert.hexToUint8(nonExistingPublicKey))))
				return Promise.resolve([]);

			return Promise.resolve([ownedNamespaceSample]);
		});

		const mockServer = new MockServer();
		const db = { namespacesByOwners: dbNamespacesByOwnersFake };
		namespaceRoutes.register(mockServer.server, db);

		beforeEach(() => {
			mockServer.resetStats();
			dbNamespacesByOwnersFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/account/:accountId/namespaces').get();

			it('returns empty if address not found', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.address);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(nonExistingTestAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns empty if publicKey not found', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingPublicKey } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.publicKey);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(nonExistingPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('parses and fordwards pagination params correctly', () => {
				// Arrange:
				const req = { params: { accountId: testAddress, id: '00123456789AABBBCCDDEEFF', pageSize: '25' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[2]).to.equal('00123456789AABBBCCDDEEFF');
					expect(dbNamespacesByOwnersFake.firstCall.args[3]).to.equal(25);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned namespaces by address', () => {
				// Arrange:
				const req = { params: { accountId: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.address);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(testAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [ownedNamespaceSample] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned namespaces by publicKey', () => {
				// Arrange:
				const req = { params: { accountId: testPublicKey } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.publicKey);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(testPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [ownedNamespaceSample] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('throws error if accountId is invalid', () => {
				// Arrange:
				const req = { params: { accountId: 'AB12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('accountId has an invalid format');
			});
		});

		describe('POST', () => {
			const route = mockServer.getRoute('/account/namespaces').post();

			it('returns empty if address not found', () => {
				// Arrange:
				const req = { params: { addresses: [nonExistingTestAddress] } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.address);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(nonExistingTestAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns empty if publicKey not found', () => {
				// Arrange:
				const req = { params: { publicKeys: [nonExistingPublicKey] } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.publicKey);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(nonExistingPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('parses and fordwards pagination params correctly', () => {
				// Arrange:
				const req = { params: { addresses: [testAddress], id: '00123456789AABBBCCDDEEFF', pageSize: '25' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[2]).to.equal('00123456789AABBBCCDDEEFF');
					expect(dbNamespacesByOwnersFake.firstCall.args[3]).to.equal(25);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned namespaces by address', () => {
				// Arrange:
				const req = { params: { addresses: [testAddress] } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.address);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(testAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [ownedNamespaceSample] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned namespaces by publicKey', () => {
				// Arrange:
				const req = { params: { publicKeys: [testPublicKey] } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbNamespacesByOwnersFake.calledOnce).to.equal(true);
					expect(dbNamespacesByOwnersFake.firstCall.args[0]).to.equal(AccountType.publicKey);
					expect(dbNamespacesByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(testPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: { namespaces: [ownedNamespaceSample] },
						type: 'namespaces'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('does not support publicKeys and addresses provided at the same time', () => {
				// Arrange:
				const req = { params: { addresses: [''], publicKeys: [''] } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('publicKeys and addresses cannot both be provided');
			});

			it('throws error if addresses contains an invalid address', () => {
				// Arrange:
				const req = { params: { addresses: [testAddress, 'AAAAAAAAAA'] } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('element in array addresses has an invalid format');
			});

			it('throws error if publicKeys contains an invalid publicKey', () => {
				// Arrange:
				const req = { params: { publicKeys: [testPublicKey, 'AAAAAAAAAA'] } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('element in array publicKeys has an invalid format');
			});
		});
	});

	describe('get namespace names by ids', () => {
		const createParentId = parentId => ({ high_: 0, low_: parentId });

		const createNamespace = (parentId, namespaceId) => ({
			// 1. in db, parentId is only stored for child namespaces
			// 2. db returns null instead of undefined when a document property is not present
			parentId: undefined === parentId ? null : createParentId(parentId),
			id: [0, namespaceId]
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
						expect(dbParamTuple.fieldsDescriptor).to.deep.equal({ id: 'id', name: 'name', parentId: 'parentId' });
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

	describe('get mosaic names', () => {
		describe('calls aliasNamesRoutesProcessor with correct params', () => {
			it('is called once for each endpoint using it with correct parameters', () => {
				// Arrange:
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};

				// Act:
				namespaceRoutes.register(server, {}, {});

				// Assert:
				expect(aliasNamesRoutesProcessorSpy.calledTwice).to.equal(true);
				expect(aliasNamesRoutesProcessorSpy.firstCall.args[1]).to.equal(catapult.model.namespace.aliasType.mosaic);
				expect(aliasNamesRoutesProcessorSpy.firstCall.args[4]).to.equal('mosaicId');
				expect(aliasNamesRoutesProcessorSpy.firstCall.args[5]).to.equal('mosaicNames');

				aliasNamesRoutesProcessorSpy.restore();
			});

			describe('getParams parses mosaic ids correctly', () => {
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};

				namespaceRoutes.register(server, {}, {});
				const getParams = aliasNamesRoutesProcessorSpy.firstCall.args[2];
				aliasNamesRoutesProcessorSpy.restore();

				it('parses mosaic ids correctly', () => {
					// Arrange:
					const req = { params: { mosaicIds: ['78A4895CB6653DE4', '56AB67FF45468988'] } };
					const parsedValues = [[0xB6653DE4, 0x78A4895C], [0x45468988, 0x56AB67FF]].map(convertToLong);

					// Act + Assert:
					expect(getParams(req)).to.deep.equal(parsedValues);
				});

				it('parses empty mosaic ids list correctly', () => {
					// Arrange:
					const req = { params: { mosaicIds: [] } };

					// Act + Assert:
					expect(getParams(req)).to.deep.equal([]);
				});

				it('returns 409 if provided mosaic id is invalid', () => {
					// Arrange:
					const req = { params: { mosaicIds: ['78A4895CB6653DE4', '123XXX', '56AB67FF45468988'] } };

					// Act + Assert:
					expect(() => { getParams(req); })
						.to.throw('element in array mosaicIds has an invalid format');
				});
			});

			describe('namespaceFilter filters namespaces correctly', () => {
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};

				namespaceRoutes.register(server, {}, {});
				const namespaceFilter = aliasNamesRoutesProcessorSpy.firstCall.args[3];
				aliasNamesRoutesProcessorSpy.restore();

				it('filters namespaces aliasing mosaicId correctly', () => {
					// Arrange:
					const id = 12345;
					const namespace1 = {
						namespace: { alias: { mosaicId: convertToLong(id) } }
					};

					// Act + Assert:
					expect(namespaceFilter(namespace1, convertToLong(id))).to.equal(true);
					expect(namespaceFilter(namespace1, convertToLong(id + 1))).to.equal(false);
				});
			});
		});
	});

	describe('get account names', () => {
		describe('calls aliasNamesRoutesProcessor with correct params', () => {
			const testAddress = {
				one: 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV',
				two: 'SCFZFP7N5C3P6EHP5D2UJ7GQD7Q7ZIENV4NZ6ELN'
			};
			const testPublicKey = {
				one: '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7',
				two: 'E6B9584AA679CADAD6569F04CD624054C0946EC49057E7AE394CEB510B606467'
			};

			it('is called once for each endpoint using it with correct parameters', () => {
				// Arrange:
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};

				// Act:
				namespaceRoutes.register(server, {});

				// Assert:
				expect(aliasNamesRoutesProcessorSpy.calledTwice).to.equal(true);
				expect(aliasNamesRoutesProcessorSpy.secondCall.args[1]).to.equal(catapult.model.namespace.aliasType.address);
				expect(aliasNamesRoutesProcessorSpy.secondCall.args[4]).to.equal('address');
				expect(aliasNamesRoutesProcessorSpy.secondCall.args[5]).to.equal('accountNames');

				aliasNamesRoutesProcessorSpy.restore();
			});

			describe('getParams parses addresses and public keys correctly', () => {
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};
				const services = {
					config: { network: { name: 'mijinTest' } }
				};

				namespaceRoutes.register(server, {}, services);
				const getParams = aliasNamesRoutesProcessorSpy.secondCall.args[2];
				aliasNamesRoutesProcessorSpy.restore();

				it('parses public keys correctly', () => {
					// Arrange:
					const req = { params: { publicKeys: [testPublicKey.one, testPublicKey.two] } };

					// Act + Assert:
					expect(getParams(req)).to.deep.equal([
						address.stringToAddress(testAddress.one),
						address.stringToAddress(testAddress.two)
					]);
				});

				it('parses addresses correctly', () => {
					// Arrange:
					const req = { params: { addresses: [testAddress.one, testAddress.two] } };

					// Act + Assert:
					expect(getParams(req)).to.deep.equal([
						address.stringToAddress(testAddress.one),
						address.stringToAddress(testAddress.two)
					]);
				});

				it('parses empty public keys list correctly', () => {
					// Arrange:
					const req = { params: { publicKeys: [] } };

					// Act + Assert:
					expect(getParams(req)).to.deep.equal([]);
				});

				it('parses empty addresses list correctly', () => {
					// Arrange:
					const req = { params: { addresses: [] } };

					// Act + Assert:
					expect(getParams(req)).to.deep.equal([]);
				});

				it('does not support publicKeys and addresses provided at the same time', () => {
					// Arrange:
					const req = { params: { publicKeys: [], addresses: [] } };

					// Act + Assert:
					expect(() => { getParams(req); })
						.to.throw('publicKeys and addresses cannot both be provided');
				});

				it('returns 409 if public keys are invalid', () => {
					// Arrange:
					const req = { params: { publicKeys: [testPublicKey.one, '12345ABCDE'] } };

					// Act + Assert:
					expect(() => { getParams(req); })
						.to.throw('element in array publicKeys has an invalid format');
				});

				it('returns 409 if addresses are invalid', () => {
					// Arrange:
					const req = { params: { addresses: [testAddress.one, '12345ABCDE'] } };

					// Act + Assert:
					expect(() => { getParams(req); })
						.to.throw('element in array addresses has an invalid format');
				});
			});

			describe('namespaceFilter filters namespaces correctly', () => {
				const aliasNamesRoutesProcessorSpy = sinon.spy(namespaceUtils, 'aliasNamesRoutesProcessor');
				const routes = {};
				const server = {
					get: (path, handler) => { routes[path] = handler; },
					post: (path, handler) => { routes[path] = handler; }
				};

				namespaceRoutes.register(server, {});
				const namespaceFilter = aliasNamesRoutesProcessorSpy.secondCall.args[3];
				aliasNamesRoutesProcessorSpy.restore();

				it('filters namespaces aliasing addresses correctly', () => {
					// Arrange:
					const id = address.stringToAddress(testAddress.one);
					const namespace1 = {
						namespace: { alias: { address: new Binary(Buffer.from(id)) } }
					};
					const namespace2 = {
						namespace: { alias: { address: new Binary(Buffer.from(id)) } }
					};

					// Act + Assert:
					expect(namespaceFilter(namespace1, address.stringToAddress(testAddress.one))).to.equal(true);
					expect(namespaceFilter(namespace2, address.stringToAddress(testAddress.two))).to.equal(false);
				});
			});
		});
	});
});
