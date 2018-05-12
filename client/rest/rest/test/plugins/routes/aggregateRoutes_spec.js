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

const aggregateRoutes = require('../../../src/plugins/routes/aggregateRoutes');
const test = require('../../routes/utils/routeTestUtils');

describe('aggregate routes', () => {
	describe('PUT transaction partial', () => {
		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transaction/partial',
			packetType: '500',
			inputs: {
				valid: {
					params: { payload: '123456' },
					parsed: Buffer.of(
						0x0B, 0x00, 0x00, 0x00, // size (header)
						0xF4, 0x01, 0x00, 0x00, // type (header)
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

	describe('PUT transaction cosignature', () => {
		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transaction/cosignature',
			packetType: '501',
			inputs: {
				valid: {
					params: { signer: '123456', signature: '998811', parentHash: 'ABEF' },
					parsed: Buffer.of(
						0x10, 0x00, 0x00, 0x00, // size (header)
						0xF5, 0x01, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x56, 0x99, 0x88, 0x11, 0xAB, 0xEF // payload (signer, signature, parentHash)
					)
				},
				invalid: {
					params: { signer: '123456', signature: '998S11', parentHash: 'ABEF' },
					error: { key: 'signature' }
				}
			}
		});
	});
});
