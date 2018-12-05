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

const mosaicRoutes = require('../../../src/plugins/routes/mosaicRoutes');
const test = require('../../routes/utils/routeTestUtils');

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
});
