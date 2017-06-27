import { expect } from 'chai';
import catapult from 'catapult-sdk';
import mosaicRoutes from '../../../src/plugins/routes/mosaicRoutes';
import test from '../../routes/utils/routeTestUtils';

const uint64 = catapult.utils.uint64;

describe('mosaic routes', () => {
	const factory = {
		createMosaicRouteInfo: (routeName, dbApiName, routeCaptureMethod) => ({
			routes: mosaicRoutes,
			routeName,
			routeCaptureMethod: routeCaptureMethod || 'get',
			createDb: (queriedIdentifiers, mosaic) => ({
				[dbApiName]: id => {
					queriedIdentifiers.push(id);
					return Promise.resolve(mosaic);
				}
			})
		}),
		createMosaicPagingRouteInfo: (routeName, createDb) => ({
			routes: mosaicRoutes,
			routeName,
			createDb
		})
	};

	describe('get by id', () => {
		const mosaicRouteInfo = factory.createMosaicRouteInfo('/mosaic/id/:id', 'mosaicById');
		const Valid_Mosaic_Id = '1234567890ABCDEF';

		it('returns mosaic if found', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(mosaicRouteInfo, {
				params: { id: Valid_Mosaic_Id },
				paramsIdentifier: [0x90ABCDEF, 0x12345678],
				dbEntity: { id: 8 },
				type: 'mosaicDescriptor'
			}));

		it('returns 404 if mosaic is not found', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(mosaicRouteInfo, {
				params: { id: Valid_Mosaic_Id },
				paramsIdentifier: [0x90ABCDEF, 0x12345678],
				printableParamsIdentifier: Valid_Mosaic_Id,
				dbEntity: undefined
			}));

		it('returns 409 if mosaic id is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(mosaicRouteInfo, {
				params: { id: '12345' }, // odd number of chars
				error: 'id has an invalid format: hex string has unexpected size \'5\''
			}));
	});

	describe('get by ids', () => {
		const mosaicRouteInfo = factory.createMosaicRouteInfo('/mosaics/ids', 'mosaicsByIds', 'post');
		const Valid_Mosaic_Ids = ['1234567890ABCDEF', 'ABCDEF0123456789'];

		it('returns mosaics if found', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(mosaicRouteInfo, {
				params: { ids: Valid_Mosaic_Ids },
				paramsIdentifier: [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]],
				dbEntity: [1, 2, 3, 4],
				type: 'mosaicDescriptor'
			}));

		it('returns 409 if one of mosaic ids is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(mosaicRouteInfo, {
				params: { ids: [Valid_Mosaic_Ids[0], '12345', Valid_Mosaic_Ids[1]] },
				error: 'element in array ids has an invalid format: hex string has unexpected size \'5\''
			}));
	});

	describe('get by namespace', () => {
		const Valid_Namespace_Id = '1234567890ABCDEF';
		const pagingTestsFactory = test.setup.createPagingTestsFactory(
			factory.createMosaicPagingRouteInfo(
				'/namespace/:namespaceId/mosaics',
				(queriedIdentifiers, entities) => ({
					mosaicsByNamespaceId: (namespaceId, pageId, pageSize) => {
						queriedIdentifiers.push({ namespaceId, pageId, pageSize });
						return Promise.resolve(entities);
					}
				})),
			{ namespaceId: Valid_Namespace_Id },
			{ namespaceId: uint64.fromHex(Valid_Namespace_Id) },
			'mosaicDescriptor');

		test.assert.addPagingTests(pagingTestsFactory);

		pagingTestsFactory.addFailureTest(
			'namespace id is invalid',
			{ namespaceId: '12345' },
			'namespaceId has an invalid format: hex string has unexpected size \'5\'');
	});

	describe('get mosaic names by ids', () => {
		function createMosaic(parentId, mosaicId) {
			return {
				parentId: [0, parentId],
				mosaicId: [0, mosaicId]
			};
		}

		const Valid_Hex_String_Mosaic_Ids = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const Valid_Uint64_Mosaic_Ids = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];

		function runTest(options) {
			// Arrange:
			const dbParamTuples = [];
			const db = {
				catapultDb: {
					findNamesByIds: (ids, transactionType, fieldsDescriptor) => {
						dbParamTuples.push({ ids, transactionType, fieldsDescriptor });
						return Promise.resolve(options.dbEntities);
					}
				}
			};

			// Act:
			return test.route.executeSingle(
				mosaicRoutes.register,
				'/names/mosaic/ids',
				'post',
				{ ids: Valid_Hex_String_Mosaic_Ids },
				db,
				response => {
					// Assert: parameters passed to db function are correct
					expect(dbParamTuples.length).to.equal(1);
					expect(dbParamTuples[0].ids).to.deep.equal(options.queryIds);
					expect(dbParamTuples[0].transactionType).to.deep.equal(catapult.model.EntityType.mosaicDefinition);
					expect(dbParamTuples[0].fieldsDescriptor).to.deep.equal({ id: 'mosaicId', name: 'name', parentId: 'parentId' });

					// check response
					expect(response).to.deep.equal({ payload: options.dbEntities, type: 'mosaicNameTuple' });
				});
		}

		it('returns empty array if no names are found', () =>
			runTest({
				queryIds: Valid_Uint64_Mosaic_Ids,
				dbEntities: []
			}));

		it('returns mosaic names if found', () =>
			runTest({
				queryIds: Valid_Uint64_Mosaic_Ids,
				dbEntities: [createMosaic(12, 14), createMosaic(12, 5), createMosaic(4, 5)]
			}));
	});
});
