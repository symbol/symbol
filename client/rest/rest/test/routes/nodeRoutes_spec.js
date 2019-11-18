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

const { test } = require('./utils/routeTestUtils');
const nodeRoutes = require('../../src/routes/nodeRoutes');
const { expect } = require('chai');

describe('node routes', () => {
	describe('get', () => {
		const serviceCreator = packet => ({
			connections: {
				singleUse: () => new Promise(resolve => {
					resolve({
						pushPull: () => new Promise(innerResolve => innerResolve(packet))
					});
				})
			},
			config: {
				apiNode: { timeout: 1000 }
			}
		});

		it('can retrieve node information', () => {
			// Arrange:
			const publicKeyBuffer = Buffer.from([
				0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
				0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
			]);

			const packet = {
				type: 601,
				size: 57,
				payload: Buffer.concat([
					Buffer.from([0x31, 0x00, 0x00, 0x00]), // size
					Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
					publicKeyBuffer,
					Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
					Buffer.from([0xDC, 0x1E]), // port
					Buffer.from([0x90]), // network identifier
					Buffer.from([0x00]), // host size
					Buffer.from([0x00]) // friendly name size
				])
			};
			const services = serviceCreator(packet);

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
							roles: 2,
							version: 23
						},
						type: 'nodeInfo'
					});
				}));
		});

		it('can retrieve node time', () => {
			// Arrange:
			const packet = {
				type: 700,
				size: 24,
				payload: Buffer.from([0x90, 0xF8, 0x6D, 0x06, 0x01, 0x00, 0x00, 0x00, 0x90, 0xF8, 0x6D, 0x06, 0x10, 0x00, 0x00, 0x00])
			};
			const services = serviceCreator(packet);

			// Act:
			return test.route.prepareExecuteRoute(nodeRoutes.register, '/node/time', 'get', {}, {}, services, routeContext =>
				routeContext.routeInvoker().then(() => {
					// Assert:
					expect(routeContext.numNextCalls).to.equal(1);
					expect(routeContext.responses.length).to.equal(1);
					expect(routeContext.redirects.length).to.equal(0);
					expect(routeContext.responses[0]).to.deep.equal({
						formatter: 'ws',
						payload: {
							communicationTimestamps: {
								receiveTimestamp: [107870352, 16],
								sendTimestamp: [107870352, 1]
							}
						},
						type: 'nodeTime'
					});
				}));
		});
	});
});
