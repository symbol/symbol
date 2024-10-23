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

import chainRoutes from '../../../src/plugins/light/chainRoutes.js';
import test from '../../routes/utils/routeTestUtils.js';
import { expect } from 'chai';
import sinon from 'sinon';

describe('Light REST chain routes', () => {
	describe('get', () => {
		describe('chain information', () => {
			it('can retrieve node information', async () => {
				// Arrange:
				const packetBufferChainStatistics = Buffer.from([0x08, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00]);
				const packetBufferFinalizationStatistics = Buffer.from([0x08, 0x00, 0x00, 0x00, 0x32, 0x01, 0x00, 0x00]);

				const chainInfoPacket = {
					type: 5,
					size: 40,
					payload: Buffer.concat([
						Buffer.from([0x89, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // height
						Buffer.from([0x70, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // finalizedHeight
						Buffer.from([0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]), // scoreHigh
						Buffer.from([0xd3, 0x45, 0x3a, 0x6f, 0x05, 0x24, 0x26, 0xaf]) // scoreLows
					])
				};

				const hash = Buffer.from([
					0x8e, 0x10, 0x37, 0xba, 0xeb, 0xb3, 0x4a, 0x7a, 0x2e, 0x6c, 0xb3, 0x90, 0xa5, 0xdf, 0xb5, 0xc3,
					0xf6, 0xe4, 0xba, 0xd4, 0xe6, 0x0c, 0x55, 0xa5, 0x4c, 0x8e, 0x8e, 0xb4, 0x8f, 0x01, 0xa0, 0xaf
				]);
				const finalizedBlockInfoPacket = {
					type: 306,
					size: 56,
					payload: Buffer.concat([
						Buffer.from([0x46, 0x0a, 0x00, 0x00]), // finalizationEpoch
						Buffer.from([0x36, 0x00, 0x00, 0x00]), // finalizationPoint
						Buffer.from([0xc4, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // height
						hash
					])
				};

				const fetchDataStub = sinon.stub();

				fetchDataStub.withArgs(packetBufferChainStatistics).resolves(chainInfoPacket);
				fetchDataStub.withArgs(packetBufferFinalizationStatistics).resolves(finalizedBlockInfoPacket);

				const services = {
					connections: {
						singleUse: () => Promise.resolve({
							pushPull: fetchDataStub
						})
					},
					config: {
						apiNode: { timeout: 1000 }
					}
				};

				// Act:
				return test.route.prepareExecuteRoute(chainRoutes.register, '/chain/info', 'get', {}, {}, services, routeContext =>
					routeContext.routeInvoker().then(() => {
						// Assert:
						expect(routeContext.numNextCalls).to.equal(1);
						expect(routeContext.responses.length).to.equal(1);
						expect(routeContext.redirects.length).to.equal(0);
						expect(routeContext.responses[0]).to.deep.equal({
							formatter: 'ws',
							payload: {
								height: 3785353n,
								scoreLow: 12620814611511920083n,
								scoreHigh: 24n,
								latestFinalizedBlock: {
									finalizationEpoch: 2630,
									finalizationPoint: 54,
									height: 3785412n,
									hash
								}
							},
							type: 'chainInfo'
						});
					}));
			});
		});
	});
});
