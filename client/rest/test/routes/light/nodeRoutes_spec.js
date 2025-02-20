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

import nodeRoutes from '../../../src/routes/light/nodeRoutes.js';
import MockServer from '../utils/MockServer.js';
import test from '../utils/routeTestUtils.js';
import { expect } from 'chai';

const restVersion = '2.5.0';

describe('Light REST node routes', () => {
	describe('get', () => {
		const serviceCreator = packet => ({
			connections: {
				singleUse: () => new Promise(resolve => {
					resolve({
						pushPull: () => new Promise(innerResolve => { innerResolve(packet); })
					});
				})
			},
			config: {
				apiNode: { timeout: 1000 },
				deployment: {
					deploymentTool: 'shoestring',
					deploymentToolVersion: '0.1.0',
					lastUpdatedDate: '1900-01-01'
				}
			}
		});

		describe('node information', () => {
			it('can retrieve node information', () => {
				// Arrange:
				const publicKeyBuffer = Buffer.from([
					0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
					0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
				]);
				const networkGenerationHashSeedBuffer = Buffer.from([
					0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
					0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
				]);

				const nodePublicKeyBuffer = Buffer.from([
					0xE4, 0x28, 0xC1, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
					0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
				]);

				const packet = {
					type: 601,
					size: 57,
					payload: Buffer.concat([
						Buffer.from([0x31, 0x00, 0x00, 0x00]), // size
						Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
						publicKeyBuffer,
						networkGenerationHashSeedBuffer,
						Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
						Buffer.from([0xDC, 0x1E]), // port
						Buffer.from([0x90]), // network identifier
						Buffer.from([0x00]), // host size
						Buffer.from([0x00]) // friendly name size
					])
				};
				const services = serviceCreator(packet);
				services.config.apiNode.nodePublicKey = nodePublicKeyBuffer;

				// Act:
				return test.route.prepareExecuteRoute(nodeRoutes.register, '/node/info', 'get', {}, {}, services, routeContext =>
					routeContext.routeInvoker().then(() => {
						// Assert:
						expect(routeContext.numNextCalls).to.equal(1);
						expect(routeContext.responses.length).to.equal(1);
						expect(routeContext.redirects.length).to.equal(0);
						expect(routeContext.responses[0]).to.deep.equal({
							formatter: 'ws',
							payload: {
								friendlyName: Buffer.alloc(0),
								host: Buffer.alloc(0),
								networkIdentifier: 144,
								port: 7900,
								publicKey: publicKeyBuffer,
								networkGenerationHashSeed: networkGenerationHashSeedBuffer,
								nodePublicKey: nodePublicKeyBuffer,
								roles: 2,
								version: 23
							},
							type: 'nodeInfo'
						});
					}));
			});
		});

		describe('node peers', () => {
			it('can retrieve node peers', () => {
				// Arrange:
				const publicKeyBuffer = Buffer.from([
					0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
					0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
				]);
				const networkGenerationHashSeedBuffer = Buffer.from([
					0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
					0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
				]);

				const packet = {
					type: 603,
					size: 114,
					payload: Buffer.concat([
						// First peer
						Buffer.from([0x31, 0x00, 0x00, 0x00]), // size
						Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
						publicKeyBuffer,
						networkGenerationHashSeedBuffer,
						Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
						Buffer.from([0xDC, 0x1E]), // port
						Buffer.from([0x90]), // network identifier
						Buffer.from([0x00]), // host size
						Buffer.from([0x00]), // friendly name size

						// Second peer
						Buffer.from([0x31, 0x00, 0x00, 0x00]), // size
						Buffer.from([0x18, 0x00, 0x00, 0x00]), // version
						publicKeyBuffer,
						networkGenerationHashSeedBuffer,
						Buffer.from([0x03, 0x00, 0x00, 0x00]), // roles
						Buffer.from([0xDC, 0x1E]), // port
						Buffer.from([0x90]), // network identifier
						Buffer.from([0x00]), // host size
						Buffer.from([0x00]) // friendly name size
					])
				};

				const services = serviceCreator(packet);

				// Act:
				return test.route.prepareExecuteRoute(nodeRoutes.register, '/node/peers', 'get', {}, {}, services, routeContext =>
					routeContext.routeInvoker().then(() => {
						// Assert:
						expect(routeContext.numNextCalls).to.equal(1);
						expect(routeContext.responses.length).to.equal(1);
						expect(routeContext.redirects.length).to.equal(0);
						expect(routeContext.responses[0]).to.deep.equal({
							formatter: 'ws',
							payload: [{
								friendlyName: Buffer.alloc(0),
								host: Buffer.alloc(0),
								networkIdentifier: 144,
								port: 7900,
								publicKey: publicKeyBuffer,
								networkGenerationHashSeed: networkGenerationHashSeedBuffer,
								roles: 2,
								version: 23
							},
							{
								friendlyName: Buffer.alloc(0),
								host: Buffer.alloc(0),
								networkIdentifier: 144,
								port: 7900,
								publicKey: publicKeyBuffer,
								networkGenerationHashSeed: networkGenerationHashSeedBuffer,
								roles: 3,
								version: 24
							}],
							type: 'nodeInfo'
						});
					}));
			});
		});

		describe('node server', () => {
			it('can retrieve server info', () => {
				// Arrange:
				const endpointUnderTest = '/node/server';
				const mockServer = new MockServer();
				nodeRoutes.register(mockServer.server, {}, serviceCreator({}));

				// Act:
				const route = mockServer.getRoute(endpointUnderTest).get();
				mockServer.callRoute(route, {});

				// Assert:
				expect(mockServer.send.firstCall.args[0]).to.deep.equal({
					payload: {
						serverInfo: {
							restVersion,
							deployment: {
								deploymentTool: 'shoestring',
								deploymentToolVersion: '0.1.0',
								lastUpdatedDate: '1900-01-01'
							}
						}
					},
					type: 'serverInfo',
					formatter: 'ws'
				});
			});
		});

		describe('unlocked account', () => {
			it('can retrieve unlocked account', () => {
				// Arrange:
				const packet = {
					type: 772,
					size: 40,
					payload: Buffer.from([0x9b, 0x4E, 0xF2, 0x78, 0x9b, 0x4E, 0xF2, 0x78, 0x9b,
						0x4E, 0xF2, 0x78, 0x9b, 0x4E, 0xF2, 0x78, 0x9b, 0x4E, 0xF2, 0x78, 0x9b,
						0x4E, 0xF2, 0x78, 0x9b, 0x4E, 0xF2, 0x78, 0x9b, 0x4E, 0xF2, 0x78])
				};
				const services = serviceCreator(packet);

				// Act:
				return test.route.prepareExecuteRoute(nodeRoutes.register, '/node/unlockedaccount', 'get', {}, {}, services, routeContext =>
					routeContext.routeInvoker().then(() => {
						// Assert:
						expect(routeContext.numNextCalls).to.equal(1);
						expect(routeContext.responses.length).to.equal(1);
						expect(routeContext.redirects.length).to.equal(0);
						expect(routeContext.responses[0]).to.deep.equal({
							unlockedAccount: [
								'9B4EF2789B4EF2789B4EF2789B4EF2789B4EF2789B4EF2789B4EF2789B4EF278'
							]
						});
					}));
			});
		});
	});
});
