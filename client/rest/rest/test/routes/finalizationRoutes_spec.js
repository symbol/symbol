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

const { MockServer } = require('./utils/routeTestUtils');
const finalizationRoutes = require('../../src/routes/finalizationRoutes');
const routeResultTypes = require('../../src/routes/routeResultTypes');
const { expect } = require('chai');

describe('finalization routes', () => {
	describe('get', () => {
		const size = Buffer.from([0x38, 0x00, 0x00, 0x00]); // 4b
		const version = Buffer.from([0x64, 0x00, 0x00, 0x00]); // 4b
		const finalizationEpoch = Buffer.from([0x02, 0x00, 0x00, 0x00]); // 4b
		const finalizationPoint = Buffer.from([0x01, 0x00, 0x00, 0x00]); // 4b
		const height = Buffer.from([0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]); // 8b
		const hash = Buffer.from([ // 32b
			0xC3, 0xC4, 0xDE, 0xA0, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
			0xB4, 0x64, 0xBB, 0x00, 0xFF, 0x11, 0xFF, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0xF8, 0xF3
		]);

		const serviceCreator = (resultPacket, assertSentPacket) => ({
			connections: {
				singleUse: () => new Promise(resolve => {
					resolve({
						pushPull: packet => {
							assertSentPacket(packet);
							return new Promise(innerResolve => innerResolve(resultPacket));
						}
					});
				})
			},
			config: {
				apiNode: { timeout: 1000 }
			}
		});

		const createFinalizationProofResultPacket = () => {
			const finalizationProof = [
				size, version, finalizationEpoch, finalizationPoint, height, hash
			];
			const finalizationProofBuffer = Buffer.concat(finalizationProof);
			finalizationProofBuffer.writeInt32LE(finalizationProofBuffer.length, 0);

			return { packetSize: finalizationProofBuffer.length, packetPayload: finalizationProofBuffer };
		};

		describe('finalization proof', () => {
			describe('by epoch', () => {
				const endpointUnderTest = '/finalization/proof/epoch/:epoch';

				it('parses params and creates correct request packet', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize, packetPayload } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x133,
						size: packetSize,
						payload: packetPayload
					};

					const assertSentPacket = packet => {
						// 0c 00 00 00 size => 12b
						// 33 01 00 00 type => 307
						// 0b 00 00 00 epoch => 11

						expect(packet).to.deep.equal(Buffer.from([
							0x0c, 0x00, 0x00, 0x00, 0x33, 0x01, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
						]));
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, assertSentPacket));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { epoch: '11' } }).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns correct finalization proof', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize, packetPayload } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x133,
						size: packetSize,
						payload: packetPayload
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, () => {}));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { epoch: '11' } }).then(() => {
						// Assert:
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: {
								version: 100,
								finalizationEpoch: 2,
								finalizationPoint: 1,
								height: [10, 0],
								hash,
								messageGroups: []
							},
							formatter: 'ws',
							type: routeResultTypes.finalizationProof
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns not found if no payload is found', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x133,
						size: packetSize,
						payload: Buffer.from([])
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, () => {}));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { epoch: '11' } }).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
					});
				});
			});

			describe('by height', () => {
				const endpointUnderTest = '/finalization/proof/height/:height';

				it('parses params and creates correct request packet', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize, packetPayload } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x134,
						size: packetSize,
						payload: packetPayload
					};

					const assertSentPacket = packet => {
						// 10 00 00 00 size => 16b
						// 34 01 00 00 type => 308
						// 00 04 00 00 00 00 00 00 height => 1024

						expect(packet).to.deep.equal(Buffer.from([
							0x10, 0x00, 0x00, 0x00, 0x34, 0x01, 0x00, 0x00,
							0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						]));
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, assertSentPacket));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { height: '1024' } }).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns correct finalization proof', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize, packetPayload } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x134,
						size: packetSize,
						payload: packetPayload
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, () => {}));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { height: '1024' } }).then(() => {
						// Assert:
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: {
								version: 100,
								finalizationEpoch: 2,
								finalizationPoint: 1,
								height: [10, 0],
								hash,
								messageGroups: []
							},
							formatter: 'ws',
							type: routeResultTypes.finalizationProof
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns not found if no payload is found', () => {
					// Arrange:
					const mockServer = new MockServer();
					const { packetSize } = createFinalizationProofResultPacket();
					const resultPacket = {
						type: 0x134,
						size: packetSize,
						payload: Buffer.from([])
					};

					finalizationRoutes.register(mockServer.server, {}, serviceCreator(resultPacket, () => {}));

					// Act:
					const route = mockServer.getRoute(endpointUnderTest).get();
					return mockServer.callRoute(route, { params: { height: '1024' } }).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
					});
				});
			});
		});
	});
});
