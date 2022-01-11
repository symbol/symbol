/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

const aggregateRoutes = require('../../../src/plugins/aggregate/aggregateRoutes');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');

const { PacketType } = catapult.packet;

describe('aggregate routes', () => {
	describe('PUT transaction partial', () => {
		const packetTypeBuffer = Buffer.alloc(4);
		packetTypeBuffer.writeUInt32LE(PacketType.pushPartialTransactions, 0);

		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transactions/partial',
			packetType: PacketType.pushPartialTransactions,
			inputs: {
				valid: {
					params: { payload: '123456' },
					parsed: Buffer.of(
						0x0B, 0x00, 0x00, 0x00, // size (header)
						...packetTypeBuffer, // type (header)
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
		const packetTypeBuffer = Buffer.alloc(4);
		packetTypeBuffer.writeUInt32LE(PacketType.pushDetachedCosignatures, 0);

		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transactions/cosignature',
			packetType: PacketType.pushDetachedCosignatures,
			inputs: {
				valid: {
					params: {
						version: '9007199254740993',
						signerPublicKey: '123456',
						signature: '998811',
						parentHash: 'ABEF'
					},
					parsed: Buffer.of(
						// header
						0x18, 0x00, 0x00, 0x00, // size
						...packetTypeBuffer, // type

						// payload
						0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // version
						0x12, 0x34, 0x56, // signerPublicKey
						0x99, 0x88, 0x11, // signature
						0xAB, 0xEF // parentHash
					)
				},
				invalid: {
					params: {
						version: '9007199254740993',
						signerPublicKey: '123456',
						signature: '998S11',
						parentHash: 'ABEF'
					},
					error: { key: 'signature' }
				}
			}
		});
	});
});
