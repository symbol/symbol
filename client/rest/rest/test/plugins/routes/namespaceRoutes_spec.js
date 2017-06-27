import { expect } from 'chai';
import catapult from 'catapult-sdk';
import namespaceRoutes from '../../../src/plugins/routes/namespaceRoutes';
import test from '../../routes/utils/routeTestUtils';

const convert = catapult.utils.convert;

describe('namespace routes', () => {
	const factory = {
		createNamespaceRouteInfo: (routeName, dbApiName) => ({
			routes: namespaceRoutes,
			routeName,
			createDb: (queriedIdentifiers, namespace) => ({
				[dbApiName]: id => {
					queriedIdentifiers.push(id);
					return Promise.resolve(namespace);
				}
			})
		}),
		createNamespacePagingRouteInfo: (routeName, createDb) => ({
			routes: namespaceRoutes,
			routeName,
			createDb
		})
	};

	describe('get by id', () => {
		const namespaceRouteInfo = factory.createNamespaceRouteInfo('/namespace/id/:id', 'namespaceById');
		const Valid_Namespace_Id = '1234567890ABCDEF';

		it('returns namespace if found', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(namespaceRouteInfo, {
				params: { id: Valid_Namespace_Id },
				paramsIdentifier: [0x90ABCDEF, 0x12345678],
				dbEntity: { id: 8 },
				type: 'namespaceDescriptor'
			}));

		it('returns 404 if namespace is not found', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(namespaceRouteInfo, {
				params: { id: Valid_Namespace_Id },
				paramsIdentifier: [0x90ABCDEF, 0x12345678],
				printableParamsIdentifier: Valid_Namespace_Id,
				dbEntity: undefined
			}));

		it('returns 409 if namespace id is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(namespaceRouteInfo, {
				params: { id: '12345' }, // odd number of chars
				error: 'id has an invalid format: hex string has unexpected size \'5\''
			}));
	});

	describe('get by owner', () => {
		const Valid_Public_Key = '75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C';
		const pagingTestsFactory = test.setup.createPagingTestsFactory(
			factory.createNamespacePagingRouteInfo(
				'/account/key/:publicKey/namespaces',
				(queriedIdentifiers, entities) => ({
					namespacesByOwner: (publicKey, pageId, pageSize) => {
						queriedIdentifiers.push({ publicKey, pageId, pageSize });
						return Promise.resolve(entities);
					}
				})),
			{ publicKey: Valid_Public_Key },
			{ publicKey: convert.hexToUint8(Valid_Public_Key) },
			'namespaceDescriptor');

		test.assert.addPagingTests(pagingTestsFactory);

		pagingTestsFactory.addFailureTest(
			'key is invalid',
			{ publicKey: '12345' },
			'publicKey has an invalid format: hex string has unexpected size \'5\'');
	});

	describe('get namespace names by ids', () => {
		// notice that for deep equal to return true for functions, they must have exactly the same source,
		// so this cannot be defined within createParentId
		function isZero() {
			return 0 === this.high_ && 0 === this.low_;
		}

		function createParentId(parentId) {
			return { high_: 0, low_: parentId, isZero };
		}

		function createNamespace(parentId, namespaceId) {
			return {
				parentId: createParentId(parentId),
				namespaceId: [0, namespaceId]
			};
		}

		const Valid_Hex_String_Namespace_Ids = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const Valid_Uint64_Namespace_Ids = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];

		function runTest(options) {
			// Arrange:
			const dbParamTuples = [];
			const db = (function () {
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
				'/names/namespace/ids',
				'post',
				{ ids: Valid_Hex_String_Namespace_Ids },
				db,
				response => {
					// Assert: parameters passed to db function are correct
					let level = 0;
					expect(dbParamTuples.length).to.equal(options.expectedNumDbQueries);
					for (const dbParamTuple of dbParamTuples) {
						expect(dbParamTuple.ids).to.deep.equal(options.queryIdsGroupedByLevel[level++]);
						expect(dbParamTuple.transactionType).to.deep.equal(catapult.model.EntityType.registerNamespace);
						expect(dbParamTuple.fieldsDescriptor).to.deep.equal({ id: 'namespaceId', name: 'name', parentId: 'parentId' });
					}

					// check response
					let expectedPayload = [];
					for (const dbEntities of options.dbEntitiesGroupedByLevel)
						expectedPayload = expectedPayload.concat(dbEntities);

					expect(response).to.deep.equal({ payload: expectedPayload, type: 'namespaceNameTuple' });
				});
		}

		it('returns empty array if no names are found', () =>
			runTest({
				queryIdsGroupedByLevel: [
					Valid_Uint64_Namespace_Ids
				],
				dbEntitiesGroupedByLevel: [
					[]
				],
				expectedNumDbQueries: 1
			}));

		it('returns namespace names if found', () =>
			runTest({
				queryIdsGroupedByLevel: [
					Valid_Uint64_Namespace_Ids
				],
				dbEntitiesGroupedByLevel: [
					[createNamespace(0, 9), createNamespace(0, 5), createNamespace(0, 7)]
				],
				expectedNumDbQueries: 1
			}));

		it('returns level1 (one ancestor) namespace names if found', () =>
			runTest({
				queryIdsGroupedByLevel: [
					Valid_Uint64_Namespace_Ids,
					[createParentId(12), createParentId(16)]
				],
				dbEntitiesGroupedByLevel: [
					[createNamespace(0, 9), createNamespace(12, 5), createNamespace(16, 7)],
					[createNamespace(0, 12), createNamespace(0, 16)]
				],
				expectedNumDbQueries: 2
			}));

		it('returns level2 (two ancestor) namespace names if found', () =>
			runTest({
				queryIdsGroupedByLevel: [
					Valid_Uint64_Namespace_Ids,
					[createParentId(17), createParentId(12), createParentId(16)],
					[createParentId(25), createParentId(25)]
				],
				dbEntitiesGroupedByLevel: [
					[createNamespace(17, 9), createNamespace(12, 5), createNamespace(16, 7)],
					[createNamespace(0, 12), createNamespace(25, 16), createNamespace(25, 17)],
					[createNamespace(0, 25), createNamespace(0, 25)]
				],
				expectedNumDbQueries: 3
			}));
	});
});
