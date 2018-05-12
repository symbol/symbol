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
const mosaicRoutes = require('../../../src/plugins/routes/mosaicRoutes');
const test = require('../../routes/utils/routeTestUtils');

const { uint64 } = catapult.utils;

describe('mosaic routes', () => {
	describe('by id', () => {
		const mosaicIds = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const uint64MosaicIds = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];
		const errorMessage = 'has an invalid format';
		test.route.document.addGetPostDocumentRouteTests(mosaicRoutes.register, {
			routes: { singular: '/mosaic/:mosaicId', plural: '/mosaic' },
			inputs: {
				valid: { object: { mosaicId: mosaicIds[0] }, parsed: [uint64MosaicIds[0]], printable: mosaicIds[0] },
				validMultiple: { object: { mosaicIds }, parsed: uint64MosaicIds },
				invalid: { object: { mosaicId: '12345' }, error: `mosaicId ${errorMessage}` },
				invalidMultiple: {
					object: { mosaicIds: [mosaicIds[0], '12345', mosaicIds[1]] },
					error: `element in array mosaicIds ${errorMessage}`
				}
			},
			dbApiName: 'mosaicsByIds',
			type: 'mosaicDescriptor'
		});
	});

	describe('get by namespace', () => {
		const Valid_Namespace_Id = '1234567890ABCDEF';
		const pagingTestsFactory = test.setup.createPagingTestsFactory(
			{
				routes: mosaicRoutes,
				routeName: '/namespace/:namespaceId/mosaics',
				createDb: (queriedIdentifiers, entities) => ({
					mosaicsByNamespaceId: (namespaceId, pageId, pageSize) => {
						queriedIdentifiers.push({ namespaceId, pageId, pageSize });
						return Promise.resolve(entities);
					}
				})
			},
			{ namespaceId: Valid_Namespace_Id },
			{ namespaceId: uint64.fromHex(Valid_Namespace_Id) },
			'mosaicDescriptor'
		);

		pagingTestsFactory.addDefault();
		pagingTestsFactory.addNonPagingParamFailureTest('namespaceId', '12345');
	});

	describe('get mosaic names by ids', () => {
		const createMosaic = (parentId, mosaicId) => ({
			parentId: [0, parentId],
			mosaicId: [0, mosaicId]
		});

		const Valid_Hex_String_Mosaic_Ids = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const Valid_Uint64_Mosaic_Ids = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];

		const runTest = options => {
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
				'/mosaic/names',
				'post',
				{ mosaicIds: Valid_Hex_String_Mosaic_Ids },
				db,
				undefined,
				response => {
					// Assert: parameters passed to db function are correct
					expect(dbParamTuples.length).to.equal(1);
					expect(dbParamTuples[0].ids).to.deep.equal(options.queryIds);
					expect(dbParamTuples[0].transactionType).to.deep.equal(catapult.model.EntityType.mosaicDefinition);
					expect(dbParamTuples[0].fieldsDescriptor).to.deep.equal({ id: 'mosaicId', name: 'name', parentId: 'parentId' });

					// check response
					expect(response).to.deep.equal({ payload: options.dbEntities, type: 'mosaicNameTuple' });
				}
			);
		};

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
