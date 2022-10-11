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

const { MockServer, test } = require('./utils/routeTestUtils');
const networkRoutes = require('../../src/routes/networkRoutes');
const { expect } = require('chai');
const sinon = require('sinon');
const tmp = require('tmp');
const fs = require('fs');

describe('network routes', () => {
	describe('get', () => {
		describe('network', () => {
			it('can retrieve network info', () => {
				// Act:
				const services = { config: { network: { name: 'foo', description: 'bar' } } };
				return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, services, routeContext => {
					// - invoke route synchronously
					routeContext.routeInvoker();

					// Assert:
					expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
					expect(routeContext.responses.length, 'single response is expected').to.equal(1);
					expect(routeContext.redirects.length, 'no redirects are expected').to.equal(0);

					// - no type information because formatting is completely bypassed
					const response = routeContext.responses[0];
					expect(response).to.deep.equal({
						name: services.config.network.name,
						description: services.config.network.description
					});
				});
			});

			it('skips network info not explicitly included', () => {
				// Act:
				const services = { config: { network: { name: 'foo', description: 'bar', head: 'fuu' } } };
				return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, services, routeContext => {
					// - invoke route synchronously
					routeContext.routeInvoker();

					const response = routeContext.responses[0];
					expect(response).to.deep.equal({
						name: services.config.network.name,
						description: services.config.network.description
					});
				});
			});
		});

		describe('network properties', () => {
			const assertCanRetrieveNetworkProperties = lines => {
				// Arrange:
				const tempNetworkFile = tmp.fileSync();
				fs.writeFileSync(tempNetworkFile.name, lines.join('\n'));

				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } } });

				// Act:
				const route = mockServer.getRoute('/network/properties').get();
				return mockServer.callRoute(route).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						network: { identifier: 'testnet' },
						chain: { enableVerifiableState: true },
						plugins: { aggregate: { maxTransactionsPerAggregate: '1\'000' } }
					});
				});
			};

			it('returns all whitelisted sections', () => assertCanRetrieveNetworkProperties([
				'[network]',
				'identifier = testnet',
				'[chain]',
				'enableVerifiableState = true',
				'[plugin:catapult.plugins.aggregate]',
				'maxTransactionsPerAggregate = 1\'000'
			]));

			it('filters out other sections', () => assertCanRetrieveNetworkProperties([
				'[network]',
				'identifier = testnet',
				'[chain]',
				'enableVerifiableState = true',
				'[private]',
				'secretCode = 42',
				'[plugin:catapult.plugins.aggregate]',
				'maxTransactionsPerAggregate = 1\'000'
			]));

			it('fails when file does not exist', () => {
				// Arrange:
				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: 'fake.dat' } } });

				// Act:
				const route = mockServer.getRoute('/network/properties').get();
				return mockServer.callRoute(route).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
					expect(mockServer.send.firstCall.args[0].message).to.equal('there was an error reading the network properties file');
				});
			});
		});

		describe('network fees transaction', () => {
			const tempNodeFile = tmp.fileSync();
			fs.writeFileSync(tempNodeFile.name, [
				'[node]',
				'minFeeMultiplier = 1\'234\'567'
			].join('\n'));

			const tempNetworkFile = tmp.fileSync();
			fs.writeFileSync(tempNetworkFile.name, [
				'[chain]',
				'defaultDynamicFeeMultiplier = 1\'000'
			].join('\n'));

			const runNetworkFeesTest = (testName, feeMultipliers, average, median, max, min) => {
				const services = {
					config: {
						numBlocksTransactionFeeStats: feeMultipliers.length,
						apiNode: {
							nodePropertyFilePath: tempNodeFile.name,
							networkPropertyFilePath: tempNetworkFile.name
						}
					}
				};

				const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves(feeMultipliers);
				const db = {
					latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
				};

				it(`${testName}: [${feeMultipliers}] average:${average}, median:${median}, max:${max}, min:${min}`, () => {
					// Arrange:
					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, db, services);
					const route = mockServer.getRoute('/network/fees/transaction').get();

					// Act
					return mockServer.callRoute(route, {}).then(() => {
						// Assert
						expect(dbLatestBlocksFeeMultiplierFake.calledOnce).to.equal(true);
						expect(dbLatestBlocksFeeMultiplierFake.firstCall.args[0]).to.equal(feeMultipliers.length);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							averageFeeMultiplier: average,
							medianFeeMultiplier: median,
							highestFeeMultiplier: max,
							lowestFeeMultiplier: min,
							minFeeMultiplier: 1234567
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			};

			describe('network fees are computed correctly for the following values', () => {
				runNetworkFeesTest('One block', [0], 1000, 1000, 0, 0);
				runNetworkFeesTest('All 0s', [0, 0, 0, 0, 0], 1000, 1000, 0, 0);
				runNetworkFeesTest('With 0s', [10, 0, 0, 0, 10], 604, 1000, 10, 0);
				runNetworkFeesTest('Big values', [999999999, 999999999, 999999999, 999999999], 999999999, 999999999, 999999999, 999999999);
				runNetworkFeesTest('Correct average', [1, 1, 1, 1], 1, 1, 1, 1);
				runNetworkFeesTest('Correct median', [90, 92, 93, 88, 95, 88, 97, 87, 98], 92, 92, 98, 87);
				runNetworkFeesTest('Correct median even number', [35, 27, 31, 32, 30, 40, 29, 43], 33, 31, 43, 27);
				runNetworkFeesTest('Correct decimals floor', [10, 11, 12, 13], 11, 11, 13, 10);
				runNetworkFeesTest('Correct decimals floor', [23, 29, 31, 27], 27, 28, 31, 23);
			});
		});

		describe('network effective rental fees', () => {
			it('can retrieve network properties needed for rental fees', () => {
				const tempNetworkFile = tmp.fileSync();
				fs.writeFileSync(tempNetworkFile.name, [
					'[chain]',
					'maxDifficultyBlocks = 5',
					'defaultDynamicFeeMultiplier = 1\'000',
					'[plugin:catapult.plugins.namespace]',
					'rootNamespaceRentalFeePerBlock = 1\'000',
					'childNamespaceRentalFee = 100',
					'[plugin:catapult.plugins.mosaic]',
					'mosaicRentalFee = 500'
				].join('\n'));

				const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves([0, 1, 2, 3, 4]);
				const db = {
					latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
				};

				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, db, { config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } } });

				const route = mockServer.getRoute('/network/fees/rental').get();
				return mockServer.callRoute(route).then(() => {
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						effectiveChildNamespaceRentalFee: '300',
						effectiveMosaicRentalFee: '1500',
						effectiveRootNamespaceRentalFeePerBlock: '3000'
					});
				});
			});

			it('fails when file does not exist', () => {
				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: 'fake.dat' } } });

				const route = mockServer.getRoute('/network/fees/rental').get();
				return mockServer.callRoute(route).then(() => {
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
					expect(mockServer.send.firstCall.args[0].message).to.equal('there was an error reading the network properties file');
				});
			});

			const runNetworkEffectiveRentalFeesTest = (
				testName,
				maxDifficultyBlocks,
				defaultDynamicFeeMultiplier,
				rootNamespaceRentalFeePerBlock,
				childNamespaceRentalFee,
				mosaicRentalFee,
				feeMultipliers,
				effectiveRootNamespaceRentalFeePerBlock,
				effectiveChildNamespaceRentalFee,
				effectiveMosaicRentalFee
			) => {
				it(`${testName}: [${[feeMultipliers]}]`, () => {
					// Arrange:
					const tempNetworkFile = tmp.fileSync();
					fs.writeFileSync(tempNetworkFile.name, [
						'[chain]',
						`maxDifficultyBlocks = ${maxDifficultyBlocks}`,
						`defaultDynamicFeeMultiplier = ${defaultDynamicFeeMultiplier}`,
						'[plugin:catapult.plugins.namespace]',
						`rootNamespaceRentalFeePerBlock = ${rootNamespaceRentalFeePerBlock}`,
						`childNamespaceRentalFee = ${childNamespaceRentalFee}`,
						'[plugin:catapult.plugins.mosaic]',
						`mosaicRentalFee = ${mosaicRentalFee}`
					].join('\n'));

					const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves(feeMultipliers);
					const db = {
						latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
					};

					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, db, {
						config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } }
					});

					// Act:
					const route = mockServer.getRoute('/network/fees/rental').get();
					return mockServer.callRoute(route).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							effectiveChildNamespaceRentalFee,
							effectiveMosaicRentalFee,
							effectiveRootNamespaceRentalFeePerBlock
						});
					});
				});
			};

			describe('network effective rental fees are computed correctly for the following values', () => {
				runNetworkEffectiveRentalFeesTest(
					'Simple all 1', 1,
					0, 1, 1, 1, [1],
					'1', '1', '1'
				);
				runNetworkEffectiveRentalFeesTest(
					'No need for default dynamic fee multiplier', 3,
					0, 1, 1, 1, [1, 2, 3],
					'2', '2', '2'
				);
				runNetworkEffectiveRentalFeesTest(
					'Default dynamic fee multiplier applied', 3,
					5, 1, 1, 1, [5, 0, 5],
					'5', '5', '5'
				);
				runNetworkEffectiveRentalFeesTest(
					'Standard case', 5,
					0, 1, 2, 3, [10, 10, 25, 50, 50],
					'25', '50', '75'
				);
				runNetworkEffectiveRentalFeesTest(
					'Decimals', 6,
					0, 1, 1, 1, [10, 10, 20, 29, 50, 50],
					'24', '24', '24'
				);
				runNetworkEffectiveRentalFeesTest(
					'Random case', 12,
					0, 1, 55, 28, [25, 32, 77, 9, 1, 50, 11, 4, 89, 56],
					'28', '1540', '784'
				);
				runNetworkEffectiveRentalFeesTest(
					'Big numbers', 3,
					0, '4294967295', '67632967295', '9007199254740993', [25, 32, 77],
					'137438953440', '2164254953440', '288230376151711776'
				);
			});
		});
	});
});
